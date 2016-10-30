//tested against https://github.com/esp8266/Arduino.git version 2.3.0-rc1

#include <Homie.h>
#include "GPSNode.hpp"
#include "utils/SDData.hpp"

using namespace Tracker;

GPSNode gpsNode;

void setup() {
  Serial.begin(9600);

  // SDData bootCounter = SDData("boot");
  // int bootCount = bootCounter.getIntValue() + 1;
  // bootCounter.setup();
  // bootCounter.setIntValue(bootCount);
  // configNode.setBootCount(bootCount);
  // Serial.printf("BOOT COUNTER: %d\n", bootCount);

  Homie_setFirmware("stutz-tracker", "1.0.0");
  Homie_setBrand("StutzTracker");
  Homie.setLoopFunction(loopHandler);
  Homie.setSetupFunction(setupHandler);

  Homie.disableLedFeedback();

  Homie.setup();
  gpsNode.setup();

  Serial.println("Stutz Tracker initiated");
}

void loop() {
  Homie.loop();
  gpsNode.loop();
}

void setupHandler() {
  // configNode.setup();
}

void loopHandler() {
  // configNode.loop();
}
