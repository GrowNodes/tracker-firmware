#include "Watchdog.hpp"
#include "utils/SDData.hpp"

using namespace Tracker;

//String name, int maxQueueSize, int recordBytes, int bufferRecords
Watchdog::Watchdog() : HomieNode("watchdog", "watchdog"),
  _timer(HomieInternals::Timer()),
  _metricsTimer(HomieInternals::Timer()),
  _state(0),
  _signalPin(16) {
    this->_timer.setInterval(500, true);
    this->_metricsTimer.setInterval(30000, true);
    pinMode(this->_signalPin, OUTPUT);
    this->ping();
}

Watchdog::~Watchdog() {
}

void Watchdog::setup() {
  Serial.println("\n--Initializing Watchdog");
  //boot counter
//  Serial.println("Reading boot count from storage");
  SDData bootCounter = SDData("/bootn.txt");
  bootCounter.setup();
  this->_bootCount = bootCounter.getIntValue() + 1;
  bootCounter.setIntValue(this->_bootCount);
  Serial.printf("Boot counter: %d\n", this->_bootCount);

  this->ping();

  this->_initialized = true;
  Serial.println("Watchdog setup OK");
}

void Watchdog::loop() {
  if(!this->_initialized) return;
  if(this->_timer.check()) {
    this->_timer.tick();
    this->ping();
  }
  if(Homie.isConnected()) {
    if(this->_metricsTimer.check()) {
      this->_metricsTimer.tick();
      Serial.println("Reporting Watchdog metrics");
      this->_reportMetrics();
      this->ping();
    }
  }
}

void Watchdog::ping() {
  // if(this->_state==0) {
  //   this->_state = 1;
  //   digitalWrite(this->_signalPin, LOW);
  // } else {
  //   this->_state = 0;
  //   digitalWrite(this->_signalPin, HIGH);
  // }
  digitalWrite(this->_signalPin, LOW);
  delay(10);
  digitalWrite(this->_signalPin, HIGH);

  //metrics
  this->_pingCount++;
  if((this->_lastPingTime>0) && (millis()-this->_lastPingTime) > this->_maxTimeBetweenPings) {
    this->_maxTimeBetweenPings = (millis()-this->_lastPingTime);
  }
  this->_lastPingTime = millis();

  Serial.print("W");
}


void Watchdog::_reportMetrics() {
  if(!this->_initialized) return;
  setProperty("maxTimeBetweenPings").send(String(this->_maxTimeBetweenPings));
  setProperty("pingCount").send(String(this->_pingCount));
  setProperty("bootCount").send(String(this->_bootCount));
}
