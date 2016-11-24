//tested against https://github.com/esp8266/Arduino.git version 2.3.0-rc1

#include <Homie.h>
#include "GPSNode.hpp"
#include "Watchdog.hpp"
#include "utils/SDUtils.hpp"

using namespace Tracker;

Watchdog watchdog;
GPSNode gpsNode;

void setup() {
  Serial.begin(9600);
  Serial.println("Stutz Tracker initialization");

  //initialize watchdog module
  Serial.println("--Initializing Watchdog");
  watchdog.setup();

  //initialize Homie
  Serial.println("");
  Serial.println("--Initializing Homie");
  Homie_setFirmware("stutz-tracker", "1.0.0");
  Homie_setBrand("StutzTracker");
//  Homie.setBootMode(HomieInternals::BootMode::STANDALONE);
  Homie.setLoopFunction(loopHandler);
  Homie.setSetupFunction(setupHandler);
  Homie.onEvent(onHomieEvent);
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
}

void onHomieEvent(const HomieEvent& event) {
  switch (event.type) {
    case HomieEventType::ABOUT_TO_RESET:
      SDUtils sdUtils;
      sdUtils.setup();
      sdUtils.deleteAllFilesOnCard();
      break;
  }
}

void setupHandler() {
}

//loops only on NORMAL MODE
void loopHandler() {
  gpsNode.loop();
}
