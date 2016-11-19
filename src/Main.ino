//tested against https://github.com/esp8266/Arduino.git version 2.3.0-rc1

#include <Homie.h>
#include "GPSNode.hpp"
#include "Watchdog.hpp"

using namespace Tracker;

GPSNode gpsNode;
Watchdog watchdog;

void setup() {
  Serial.begin(9600);
  Serial.println("Stutz Tracker initialization");

  //initialize watchdog module
  watchdog.setup();

  //initialize Homie
  Serial.println("");
  Serial.println("--Initializing Homie");
  Homie_setFirmware("stutz-tracker", "1.0.0");
  Homie_setBrand("StutzTracker");
  Homie.setLoopFunction(loopHandler);
  Homie.setSetupFunction(setupHandler);
  Homie.disableLedFeedback();
  watchdog.ping();
  Homie.setup();
  watchdog.ping();

  //initialize GPS module
  Serial.println("");
  Serial.println("--Initializing GPS");
  watchdog.ping();
  gpsNode.setup();
  watchdog.ping();

  Serial.println("");
  Serial.println("--Starting operational loop");
}

void loop() {
  watchdog.loop();
  Homie.loop();
  watchdog.loop();
  gpsNode.loop();
}

void setupHandler() {
  // configNode.setup();
}

void loopHandler() {
  // configNode.loop();
}
