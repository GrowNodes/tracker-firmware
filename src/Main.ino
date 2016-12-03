//tested against https://github.com/esp8266/Arduino.git version 2.3.0-rc1

#include <Homie.h>
#include "GPSReader.hpp"

using namespace Tracker;

Watchdog watchdog = Watchdog();
GPSUploader gpsUploader = GPSUploader(watchdog);
GPSReader gpsReader = GPSReader(watchdog, gpsUploader);

void setup() {
  watchdog.ping();

  Serial.begin(9600);
  Serial.println("\nStutz Tracker initialization");

  watchdog.ping();

  //initialize Homie
  Serial.println("");
  Serial.println("--Initializing Homie");
  Homie_setFirmware("stutz-tracker", "1.0.0");
  Homie_setBrand("StutzTracker");
  Homie.onEvent(onHomieEvent);
  Homie.disableLedFeedback();
  watchdog.ping();
  Homie.setup();
  watchdog.ping();
  Serial.println("\nInitialization complete");
}

void loop() {
  Homie.loop();
}

void onHomieEvent(const HomieEvent& event) {
  switch (event.type) {
    case HomieEventType::ABOUT_TO_RESET:
      //SDUtils sdUtils;
      //sdUtils.setup();
      //sdUtils.deleteAllFilesOnCard();
      break;
  }
}
