#include <Homie.h>

const int PIN_RELAY = 5;

HomieNode gpsNode("gps", "gps");
GPSNode gpsNode;

void setup() {
  Homie.setFirmware("stutz-tracker", "1.0.0");
  Homie.setLoopFunction(loopHandler);
  Homie.setSetupFunction(setupHandler);
  Homie.registerNode(gpsNode);

  Homie.enableBuiltInLedIndicator(false);
  Homie.enableLogging(true);

  Homie.setup();

  this->_interface->logger->log(F("Stutz Tracker initiated."));
}

void setupHandler() {
  gpsNode.setup();
}

void loopHandler() {
  gpsNode.loop();
}

void loop() {
  Homie.loop();
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
