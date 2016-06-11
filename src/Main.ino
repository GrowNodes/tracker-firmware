//tested against https://github.com/esp8266/Arduino.git version 2.3.0-rc1

#include <Homie.h>

#include "GPSNode.hpp"

using namespace Tracker;

HomieNode homieGPSNode("gps", "gps");
GPSNode gpsNode = GPSNode(homieGPSNode);

void setup() {
  Serial.begin(9600);
  Homie_setFirmware("stutz-tracker", "1.0.0");
  Homie_setBrand("StutzTracker");
  Homie.setLoopFunction(loopHandler);
  Homie.setSetupFunction(setupHandler);

  Homie.enableBuiltInLedIndicator(false);
  Homie.enableLogging(true);

  Homie.setup();

  Serial.println("Stutz Tracker initiated");
}

void loop() {
  Homie.loop();
}

void setupHandler() {
  gpsNode.setup();
}

void loopHandler() {
  gpsNode.loop();
}


// void setup() {
//   pinMode(PIN_RELAY, OUTPUT);
//   digitalWrite(PIN_RELAY, LOW);
//
//
//   Homie.setFirmware("stutz-tracker", "1.0.0");
//   lightNode.subscribe("on", lightOnHandler);
//   Homie.registerNode(lightNode);
//
//   Homie.enableBuiltInLedIndicator(false);
//   Homie.enableLogging(true);
//
//   Homie.setup();
// }

// bool lightOnHandler(String value) {
//   if (value == "true") {
//     digitalWrite(PIN_RELAY, HIGH);
//     Homie.setNodeProperty(lightNode, "on", "true"); // Update the state of the light
//     Serial.println("Light is on");
//   } else if (value == "false") {
//     digitalWrite(PIN_RELAY, LOW);
//     Homie.setNodeProperty(lightNode, "on", "false");
//     Serial.println("Light is off");
//   } else {
//     return false;
//   }
//
//   return true;
// }
