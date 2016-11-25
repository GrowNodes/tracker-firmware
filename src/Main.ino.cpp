# 1 "c:\\users\\flavio\\appdata\\local\\temp\\tmpavdkat"
#include <Arduino.h>
# 1 "D:/development/tracker-firmware/src/Main.ino"




#include <Homie.h>

#include "GPSNode.hpp"

#include "utils/SDUtils.hpp"



using namespace Tracker;



Watchdog watchdog;

GPSNode gpsNode = GPSNode(watchdog);
void setup();
void loop();
void onHomieEvent(const HomieEvent& event);
#line 23 "D:/development/tracker-firmware/src/Main.ino"
void setup() {

  watchdog.ping();



  Serial.begin(9600);

  Serial.println("\nStutz Tracker initialization");



  watchdog.ping();





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







      break;

  }

}