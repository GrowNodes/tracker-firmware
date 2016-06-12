//tested against https://github.com/esp8266/Arduino.git version 2.3.0-rc1

#include <Homie.h>
#include "GPSNode.hpp"

using namespace Tracker;

ConfigNode configNode;
GPSNode gpsNode = GPSNode(configNode);

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
