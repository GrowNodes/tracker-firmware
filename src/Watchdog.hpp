#include <Homie.h>
#include "utils/SDData.hpp"
#include "../lib/homie-esp8266/src/Homie/Timer.hpp"

namespace Tracker {
  class Watchdog {
    public:
      Watchdog();
      ~Watchdog();
      void setup();
      void loop();
      void ping();
    private:
      void _reportMetrics();
      int _state;
      int _signalPin;
      HomieInternals::Timer _timer;
      HomieInternals::Timer _metricsTimer;
      HomieNode _homieNode;
      unsigned int _maxTimeBetweenPings = 0;
      unsigned int _pingCount = 0;
      unsigned long _lastPingTime = 0;
      unsigned long _bootCount = 0;
  };
}
