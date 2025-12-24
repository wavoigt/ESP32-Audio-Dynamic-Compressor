/**
 * @file audiokit-effects-audiokit.ino
 * @author Phil Schatzmann
 * @brief Some Guitar Effects that can be controlled via a Web Gui. 
 * @version 0.1
 * @date 2022-10-14
 * @copyright Copyright (c) 2022
 */ 

// Audio Compressor mit Web Interface
// Board: ESP32 Wrover Kit (also for HiFi-ESP32 Board)
// Partition Scheme: Minimal SPIFFS with OTA

// For Stereo Compressor using modified versions of AudioEffects.h and AudioEffect.h
// Copy the modified files into the Arduino library folder: Arduino\libraries\audio-tools\src\AudioTools\CoreAudio\AudioEffects\
// If you are using the original Audio Tools library, you have to comment out the lines with 'Compressor_Stereo' and 'Compressor_Active'.
// in this case, the compressor is working in mono mode.

// # Test Output to SPDIF:
// If you encounter some quality issues you can increase the DEFAULT_BUFFER_SIZE (e.g. to 2048) 
// and I2S_BUFFER_SIZE/I2S_BUFFER_COUNT

// LED_GRN: Reduction -1dB to -12dB
// LED_RED: Reduction -6dB to -max

#define USE_AUDIO_LOGGING false // false = less memory
#define LED_GRN 12 // pull down, must be low at boot
#define LED_RED 21 // pull down, must be low at boot
// #define TEST_GENERATOR
#define TOS_LINK

#include "HttpServer.h"   // https://github.com/pschatzmann/TinyHttp
#include "AudioTools.h"   // https://github.com/pschatzmann/arduino-audio-tools.git
#include "AudioTools/AudioLibs/SPDIFOutput.h" // Arduino\libraries\audio-tools\src\AudioTools\CoreAudio\AudioEffects
#include <Preferences.h>
Preferences preferences;

// Server
WiFiServer wifi;
HttpServer server(wifi);
HttpParameters parameters;
const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASS";
TaskHandle_t TaskCore0; // Handle für den Task

// Audio Format
const uint32_t sample_rate = 44100;
const uint16_t channels = 2;
const uint8_t bits_per_sample = 16;
AudioInfo info(sample_rate, channels, bits_per_sample);

// Effects control input initial
uint8_t ratio = 100;             // Ratio
uint8_t threshold = 30;       // Threshold in %
uint16_t attackTime = 10;     // Attack-Zeit in ms
uint16_t releaseTime = 500;   // Release-Zeit in ms

// Effects
Compressor compressor ((float)sample_rate, (float)attackTime, (float)releaseTime, 0, (float)threshold, (float)ratio);

#ifdef TEST_GENERATOR
  // Test with Sine Generator
  SineWaveGenerator<int16_t> sineWave(32000);     // subclass of SoundGenerator with max amplitude of 32000
  GeneratedSoundStream<int16_t> sound(sineWave);  // Stream generated from sine wave
  AudioEffectStream effects(sound);  // input
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

static const char PROGMEM htmlForm[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Compressor</title>
<style>
body {background-color: #cccccc; font-family: Arial; text-align: left; margin: 0px auto; padding-top: 20px; padding-left: 30px;}
.slider {width: 95%;}
</style>
</head>
<body>
<h2>Compressor</h2>
<form method='post'>
<div>
Ratio 1 - 200 &nbsp;&nbsp;<b>%ratio%</b><br>
<input type='range' class='slider' name='ratio' onchange='this.form.submit()' min='1' max='201' step='10' value='%ratio%'>
</div><div>
<br>Threshold 5 - 100 &nbsp;&nbsp;<b>%thresh%</b><br>
<input type='range' class='slider' name='thresh' onchange='this.form.submit()' min='5' max='100' step='1' value='%thresh%'>
</div><div>
<br>Attack 5 - 100ms &nbsp;&nbsp;<b>%attack%</b><br>
<input type='range' class='slider' name='attack' onchange='this.form.submit()' min='5' max='100' step='5' value='%attack%'>
</div><div>
<br>Release 10 - 1000ms &nbsp;&nbsp;<b>%release%</b><br>
<input type='range' class='slider' name='release' onchange='this.form.submit()' min='10' max='1010' step='20' value='%release%'>
</div>
</form>
</body>
</html>
)rawliteral";


// Update values in effects
void updateValues(){
  compressor.setCompressionRatio((float)ratio);
  compressor.setThreshold((float)threshold);
  compressor.setAttack((float)attackTime);
  compressor.setRelease((float)releaseTime);
 }

void printValues() {
    char msg[120];
    snprintf(msg, 120, "==> updated values %d %d %d %d",ratio, threshold, attackTime, releaseTime);
    Serial.println(msg);        
}

void getHtml(HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
    // provide html and replace variables with actual values
    tinyhttp::Str html(2000);
    html.set(htmlForm);
    html.replace("%ratio%",ratio); html.replace("%ratio%",ratio);
    html.replace("%thresh%",threshold); html.replace("%thresh%",threshold);
    html.replace("%attack%",attackTime); html.replace("%attack%",attackTime);
    html.replace("%release%",releaseTime); html.replace("%release%",releaseTime);
    server->reply("text/html", html.c_str(), 200);
};

void postData(HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
    // parse the parameters
    parameters.parse(server->client()); // timeout verkleinert in HttpParameters.h
    // update parameters
    ratio = parameters.getInt("ratio");
    threshold = parameters.getInt("thresh");
    attackTime = parameters.getInt("attack");
    releaseTime = parameters.getInt("release");
    // return updated html       
    getHtml(server, requestPath, hl); 
    updateValues();
    preferences.begin("Compressor", false);
    preferences.putUChar("ratio", ratio);
    preferences.putUChar("threshold", threshold);
    preferences.putUShort("attackTime", attackTime);
    preferences.putUShort("releaseTime", releaseTime);
    preferences.end();
    // printValues();
};


// Function to run on Core 0
void httpTaskCode( void * parameter ){
  Serial.print("HTTP-Task running on Core: ");
  Serial.println(xPortGetCoreID());
  for(;;){
    server.copy(); 
    delay(5); // time for processing WiFi
  } 
};



// Arduino Setup
void setup(void) {
  
  pinMode(LED_GRN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_GRN, LOW);
  digitalWrite(LED_RED, LOW);
  Compressor_Stereo  = true; // comment out if using original AudioEffects.h
  
  // Get Preferences
  preferences.begin("Compressor", false);
  ratio = preferences.getUChar("ratio", ratio);
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
  HttpLogger.begin(Serial, Error);
  server.on("/",T_GET, getHtml);
  server.on("/",T_POST, postData);
  server.begin(80, ssid, password);
  server.setTimeout(200); // default = 1000

  // Den Task im Setup erstellen und an Core 0 "pinnen"
  xTaskCreatePinnedToCore(
      httpTaskCode,    /* Name der Funktion (Task) */
      "HttpCore0",     /* Task-Name */
      8192,            /* Stack-Größe (RAM) – 8K ist ein sicherer Wert */
      NULL,            /* Parameter */
      2,               /* Priorität (etwas höher als Standard-1) */
      &TaskCore0,      /* Task Handle */
      0);              /* <-- Core ID 0 */
 

#ifdef TEST_GENERATOR
  // Generator
  sineWave.begin(info, 500);
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
void loop() {
  copier.copy();
  // comment out if using original AudioEffects.h
  if (Compressor_Active1) digitalWrite(LED_GRN, HIGH); else digitalWrite(LED_GRN, LOW); 
  if (Compressor_Active2) digitalWrite(LED_RED, HIGH); else digitalWrite(LED_RED, LOW); 
}
