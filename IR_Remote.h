// Source: Examples - IRremote - SimpleReceiver
// from Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.

// We use the IR Remote to increase / decrease the Limiter Threshold.
// To find out your IR Protocol and Codes:
// Flash IR-SimpleReceiver, watch Serial monitor and press desired buttons on the IR Remote.
// Define the Protocol (#define DECODE_xxx) which is declared in IR-SimpleReceiver.ino
// Replace the Codes in IR_getButton() by the desired codes.

#define DECODE_NEC  // IR Includes Yamaha, Apple and Onkyo ~ 250 bytes, change to your remote protocol
#define IR_RECEIVE_PIN          39  // D15 ; IR_IN = 39
#define LED_BUILTIN             21  // RGB LED = same as LED_RED
// #define IR_SEND_PIN             4  // D4 ; no sending
#define NO_LED_FEEDBACK_CODE // Schaltet das automatische Blinken ab
#include <IRremote.hpp> // include the IR library Arduino_IRremote with IR Receiver TSOP4838 at GPIO39

bool IRledIsOn = false;
unsigned long ledOnTime = 0; 

bool IR_begin()  {
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  Serial.print(F("Ready to receive IR signals of protocols: "));
  printActiveIRProtocols(&Serial); // Requires additional 318 bytes program memory
  Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));
  return true;
}

bool IR_getButton(int &tdelta)
{
  tdelta = 0;
  bool found = false;
  if (IrReceiver.decode()) {
    // Print a summary of received data
    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
        Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
        // We have an unknown protocol here, print extended info
        IrReceiver.printIRResultRawFormatted(&Serial, true);
        IrReceiver.resume(); // Do it here, to preserve raw data for printing with printIRResultRawFormatted()
    } else {
        IrReceiver.resume(); // Early enable receiving of the next IR frame
        // IrReceiver.printIRResultShort(&Serial);   // Requires additional 1436 bytes program memory
        // IrReceiver.printIRSendUsage(&Serial);     // Calls printIRResultShort() and other functions, if protocol is UNKNOWN
    }
    // Yamaha FB (NEC): Taste 1 0x11 .. 9 0x19, Stop 0x1, Start 0x2, FWD 0x3, REV 0x4
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
    } else if ((IrReceiver.decodedIRData.command == 0x8E) || (IrReceiver.decodedIRData.command == 0x9E)) { // Threshold >
        tdelta = 5; found = true;
    } else if ((IrReceiver.decodedIRData.command == 0x8F) || (IrReceiver.decodedIRData.command == 0x9F)) { // Threshold <
        tdelta = -5; found = true;
    }
    if (found) {
      IRledIsOn = true;
      digitalWrite(LED_BUILTIN, HIGH);
      ledOnTime = millis();
    }
  } 
  if (IRledIsOn && (millis() - ledOnTime > 100)) {
    IRledIsOn = false;
    digitalWrite(LED_BUILTIN, LOW);
  }
  return found;
}
