#include <Homie.h>
#include "../lib/homie-esp8266/src/Homie/Timer.hpp"

namespace Tracker {
  class Sleeper:public HomieNode {
    public:
      Sleeper();
      ~Sleeper();
      void setup();
      void loop();
    private:
      void _reportMetrics();
      bool _initialized = false;
      HomieInternals::Timer _timer;
      HomieInternals::Timer _metricsTimer;
  };
}
