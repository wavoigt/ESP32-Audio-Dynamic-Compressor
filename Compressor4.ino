/**
 * @file audiokit-effects-audiokit.ino
 * @author Phil Schatzmann
 * @brief Some Guitar Effects that can be controlled via a Web Gui. 
 * @version 0.1
 * @date 2022-10-14
 * @copyright Copyright (c) 2022
 */ 

// Audio Compressor mit Web Interface Siehe CompHtmlServer.h
// Board: ESP32 Wrover Kit (also for HiFi-ESP32 Board)
// Partition Scheme: Minimal SPIFFS with OTA

// For Stereo Compressor using modified versions of AudioEffects.h and AudioEffect.h
// Copy the modified files into the Arduino library folder: Arduino\libraries\audio-tools\src\AudioTools\CoreAudio\AudioEffects\
// If you are using the original Audio Tools library, you have to comment out the lines with 'Compressor_Stereo' and 'Compressor_Active'.
// in this case, the compressor is working in mono mode.

// # Test Output to SPDIF:
// If you encounter some quality issues you can increase the DEFAULT_BUFFER_SIZE (e.g. to 2048) 
// and I2S_BUFFER_SIZE/I2S_BUFFER_COUNT

#define USE_AUDIO_LOGGING true // false = less memory
#define RGB_LED 12 // pull down, must be low at boot
// #define TEST_GENERATOR
#define TOS_LINK

#include <ArduinoJson.h>  // https://arduinojson.org/
#include "HttpServer.h"   // https://github.com/pschatzmann/TinyHttp
#include "AudioTools.h"   // https://github.com/pschatzmann/arduino-audio-tools.git
#include "AudioTools/AudioLibs/SPDIFOutput.h"
// Arduino\libraries\audio-tools\src\AudioTools\CoreAudio\AudioEffects
#include "CompHtmlServer.h"
#include <ArduinoOTA.h>
#include <Preferences.h>
Preferences preferences;

// Audio Format
const uint32_t sample_rate = 44100;
const uint16_t channels = 2;
const uint8_t bits_per_sample = 16;
AudioInfo info(sample_rate, channels, bits_per_sample);

// Effects control input initial
float ratio = 50;             // Ratio
uint8_t threshold = 50;       // Threshold in %
uint16_t attackTime = 10;     // Attack-Zeit in ms
uint16_t releaseTime = 100;   // Release-Zeit in ms
uint16_t holdTime = 10;       // Hold-Zeit in ms

// Effects
Compressor compressor ((float)sample_rate, (float)attackTime, (float)releaseTime, (float)holdTime, (float)threshold, ratio);

#ifdef TEST_GENERATOR
  // Test with Sine Generator
  SineWaveGenerator<int16_t> sineWave(32000);     // subclass of SoundGenerator with max amplitude of 32000
  GeneratedSoundStream<int16_t> sound(sineWave);  // Stream generated from sine wave
  FadeStream fade(sound); 
  AudioEffectStream effects(fade);  // input
#else
  // Streams
  I2SStream in;     // Toslink in
  AudioEffectStream effects(in);   // input
#endif

#ifdef TOS_LINK
  SPDIFOutput out;  // Toslink out
#else
  I2SStream out;  // DAC
#endif

StreamCopy copier(out, effects); // copies effects into i2s

// Update values in effects
void updateValues(){
  compressor.setCompressionRatio(ratio);
  compressor.setThreshold((float)threshold);
  compressor.setAttack((float)attackTime);
  compressor.setRelease((float)releaseTime);
 }


// provide JSON as webservice
void getJson(HttpServer * server, const char*requestPath, HttpRequestHandlerLine * hl) {
  auto parameters2Json = [](Stream & out) {
    JsonDocument doc;
    doc["RatioControl"]["value"] = ratio;
    doc["RatioControl"]["min"] = 1;
    doc["RatioControl"]["max"] = 101;
    doc["RatioControl"]["step"] = 5;
    doc["Threshold"]["value"] = threshold;
    doc["Threshold"]["min"] = 5;
    doc["Threshold"]["max"] = 100;
    doc["Threshold"]["step"] = 1;
    doc["AttackTime"]["value"] = attackTime;
    doc["AttackTime"]["min"] = 5;
    doc["AttackTime"]["max"] = 100;
    doc["AttackTime"]["step"] = 5;
    doc["ReleaseTime"]["value"] = releaseTime;
    doc["ReleaseTime"]["min"] = 10;
    doc["ReleaseTime"]["max"] = 1010;
    doc["ReleaseTime"]["step"] = 20;
    serializeJson(doc, out);
  };
  // provide data as json using callback
  server->reply("text/json", parameters2Json, 200);
}


// Process Posted Json
void postJson(HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) {
  // post json to server
  JsonDocument doc;
  deserializeJson(doc, server->client());
  ratio = doc["RatioControl"];
  threshold = doc["Threshold"];
  attackTime = doc["AttackTime"];
  releaseTime = doc["ReleaseTime"];
  // update values in controls
  updateValues();
  preferences.begin("Compressor", false);
  preferences.putFloat("ratio", ratio);
  preferences.putUChar("threshold", threshold);
  preferences.putUShort("attackTime", attackTime);
  preferences.putUShort("releaseTime", releaseTime);
  preferences.end();
  server->reply("text/json", "{}", 200);
  //char msg[120];
  //snprintf(msg, 120, "> updated values %f %d %d %d", ratio, threshold, attackTime, releaseTime);
  //Serial.println(msg);
  //Serial.print(gain_reduce); Serial.print("  "); Serial.println(gain_); 
}


void ota_Setup()
{
  ArduinoOTA.setHostname("Compressor"); // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setPassword("madda"); // No authentication by default
  
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
      else type = "filesystem"; // U_SPIFFS
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  ArduinoOTA.begin();
}

// Arduino Setup
void setup(void) {
  
  pinMode(RGB_LED, OUTPUT);
  digitalWrite(RGB_LED, LOW);
  Compressor_Stereo  = true; // comment out if using original AudioEffects.h
  
  // Get Preferences
  preferences.begin("Compressor", false);
  ratio = preferences.getFloat("ratio", ratio);
  threshold = preferences.getUChar("threshold", threshold);
  attackTime = preferences.getUShort("attackTime", attackTime);
  releaseTime = preferences.getUShort("releaseTime", releaseTime);
  preferences.end();

  Serial.begin(115200);
  // change to Warning to improve the quality
  #if USE_AUDIO_LOGGING == true  
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);
  #endif

  // Setup Server
  server.on("/", T_GET, getHtml);
  server.on("/service", T_GET, getJson);
  server.on("/service", T_POST, postJson);
  server.begin(80, ssid, password);
  server.setNoConnectDelay(0);

  ota_Setup();

#ifdef TEST_GENERATOR
  // Generator
  sineWave.begin(info, 500);
  fade.begin(info);
  Serial.println("Generator started...");

#else
  // start I2S in
  auto config_in = in.defaultConfig(RX_MODE);
  config_in.copyFrom(info); 
  config_in.i2s_format = I2S_STD_FORMAT;
  config_in.is_master = true;
  config_in.port_no = 1;
  config_in.pin_data = 19;  // (MISO) // 22
  config_in.pin_bck = 18;   // (CLK)  // 14
  config_in.pin_ws = 14;    // (RST)  // 15
  config_in.buffer_size = 512; // minimize lag 256
  config_in.buffer_count = 2;
  in.begin(config_in);
  Serial.println("I2S started");
#endif

#ifdef TOS_LINK
  // start SPDIF out for TosLink
  auto config_out = out.defaultConfig();
  config_out.pin_data = 23; // (MOSI)
  config_out.port_no = 0;
  config_out.buffer_size = 512; // 384
  config_out.buffer_count = 8;
  out.begin(config_out);
  Serial.println("SPDIF started");
#else
  // start I2S out for DA converter
  auto config_out = out.defaultConfig(TX_MODE);
  config_out.copyFrom(info); 
  config_out.i2s_format = I2S_STD_FORMAT;
  config_out.is_master = true;
  config_out.port_no = 0;
  config_out.pin_data = 22;
  config_out.pin_bck = 26;
  config_out.pin_ws = 25;
  out.begin(config_out);
  Serial.println("D/A started");
#endif

  // setup effects
  effects.addEffect(compressor);
  effects.begin(info);
  updateValues();
  Serial.println("Compressor started");
}

// Arduino loop - copy data

long cnt = 0;
bool on = true;

void loop() {
  copier.copy();
  server.copy();
  // comment our if using original AudioEffects.h
  if (Compressor_Active) digitalWrite(RGB_LED, HIGH); else digitalWrite(RGB_LED, LOW); 
  ArduinoOTA.handle();
}
