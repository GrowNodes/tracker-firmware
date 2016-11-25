#include <Homie.h>
#include "../lib/homie-esp8266/src/Homie/Timer.hpp"

namespace Tracker {
  class Watchdog:public HomieNode {
    public:
      Watchdog();
      ~Watchdog();
      void setup();
      void loop();
      void ping();
    private:
      void _reportMetrics();
      bool _initialized = false;
      int _state;
      int _signalPin;
      HomieInternals::Timer _timer;
      HomieInternals::Timer _metricsTimer;
      unsigned int _maxTimeBetweenPings = 0;
      unsigned int _pingCount = 0;
      unsigned long _lastPingTime = 0;
      int _bootCount = 0;
  };
}
