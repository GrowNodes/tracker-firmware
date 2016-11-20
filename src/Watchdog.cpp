#include "Watchdog.hpp"

using namespace Tracker;

//String name, int maxQueueSize, int recordBytes, int bufferRecords
Watchdog::Watchdog() :
  _homieNode(HomieNode("watchdog", "watchdog")),
  _timer(HomieInternals::Timer()),
  _metricsTimer(HomieInternals::Timer()),
  _state(0),
  _signalPin(16) {
}

Watchdog::~Watchdog() {
}

void Watchdog::setup() {
  Serial.println("Watchdog init");
  this->_timer.setInterval(500, true);
  this->_metricsTimer.setInterval(30000, true);
  pinMode(this->_signalPin, OUTPUT);

  this->loop();

  //boot counter
  SDData bootCounter = SDData("boot");
  this->_bootCount = bootCounter.getIntValue() + 1;
  bootCounter.setup();
  bootCounter.setIntValue(this->_bootCount);
  Serial.printf("BOOT COUNTER: %d\n", this->_bootCount);

  this->loop();
}

void Watchdog::loop() {
  if(this->_timer.check()) {
    this->_timer.tick();
    this->ping();
  }
  if(this->_metricsTimer.check()) {
    this->_metricsTimer.tick();
    Serial.println("Reporting Watchdog metrics");
    this->_reportMetrics();
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

  //metrics
  this->_pingCount++;
  if((this->_lastPingTime>0) && (millis()-this->_lastPingTime) > this->_maxTimeBetweenPings) {
    this->_maxTimeBetweenPings = (millis()-this->_lastPingTime);
  }
  this->_lastPingTime = millis();

  Serial.print(".");
}

void Watchdog::_reportMetrics() {
  this->_homieNode.setProperty("maxTimeBetweenPings").send(String(this->_maxTimeBetweenPings));
  this->_homieNode.setProperty("pingCount").send(String(this->_pingCount));
  this->_homieNode.setProperty("bootCount").send(String(this->_bootCount));
}
