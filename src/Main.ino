//tested against https://github.com/esp8266/Arduino.git version 2.3.0-rc1

#include <Homie.h>
#include "GPSNode.hpp"
#include "utils/SDData.hpp"

using namespace Tracker;

ConfigNode configNode;
GPSNode gpsNode = GPSNode(configNode);

SDData bootCounter = SDData("reboot");

void setup() {
  Serial.begin(9600);

  int bootCount = bootCounter.getIntValue() + 1;
  bootCounter.setup();
  bootCounter.setIntValue(bootCount);
  gpsNode.setBootCount(bootCount);
  Serial.printf("BOOT COUNTER: %d\n", bootCount);

  Homie_setFirmware("stutz-tracker", "1.0.0");
  Homie_setBrand("StutzTracker");
  Homie.setLoopFunction(loopHandler);
  Homie.setSetupFunction(setupHandler);

  Homie.enableBuiltInLedIndicator(false);
  Homie.enableLogging(true);

  Homie.setup();
  gpsNode.setup();

  Serial.println("Stutz Tracker initiated");
}

void loop() {
  Homie.loop();
  gpsNode.loop();
}

void setupHandler() {
  configNode.setup();
}

void loopHandler() {
  configNode.loop();
}
