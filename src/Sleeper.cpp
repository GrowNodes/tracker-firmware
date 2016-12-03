#include "Sleeper.hpp"

using namespace Tracker;

//String name, int maxQueueSize, int recordBytes, int bufferRecords
Sleeper::Sleeper() : HomieNode("sleeper", "sleeper"),
  _timer(HomieInternals::Timer()),
  _metricsTimer(HomieInternals::Timer()) {
    this->_timer.setInterval(500, true);
    this->_metricsTimer.setInterval(30000, true);
}

Sleeper::~Sleeper() {
}

void Sleeper::setup() {
  Serial.println("\n--Initializing Sleeper");
  this->_initialized = true;
  Serial.println("Sleeper setup OK");
}

void Sleeper::loop() {
  if(!this->_initialized) return;
  if(this->_timer.check()) {
    this->_timer.tick();
  }
  if(Homie.isConnected()) {
    if(this->_metricsTimer.check()) {
      this->_metricsTimer.tick();
      Serial.println("Reporting Sleeper metrics");
      this->_reportMetrics();
    }
  }
}

void Sleeper::_reportMetrics() {
  if(!this->_initialized) return;
  // setProperty("maxTimeBetweenPings").send(String(this->_maxTimeBetweenPings));
}
