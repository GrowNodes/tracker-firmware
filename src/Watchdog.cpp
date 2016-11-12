#include "Watchdog.hpp"

using namespace Tracker;

//String name, int maxQueueSize, int recordBytes, int bufferRecords
Watchdog::Watchdog() :
  _timer(HomieInternals::Timer()),
  _state(0),
  _signalPin(16) {
}

Watchdog::~Watchdog() {
}

void Watchdog::setup() {
  Serial.println("Watchdog init");
  this->_timer.setInterval(500, true);
  pinMode(this->_signalPin, OUTPUT);
  this->loop();
  delay(100);
  this->loop();
}

void Watchdog::loop() {
  if(this->_timer.check()) {
    this->_timer.tick();
    this->ping();
  }
}

void Watchdog::ping() {
  if(this->_state==0) {
    this->_state = 1;
    digitalWrite(this->_signalPin, LOW);
  } else {
    this->_state = 0;
    digitalWrite(this->_signalPin, HIGH);
  }
  Serial.print(".");
}
