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
      int _state;
      int _signalPin;
      HomieInternals::Timer _timer;
  };
}
