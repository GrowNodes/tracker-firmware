#include <Homie.h>
#include "utils/SDQueue.hpp"
#include "../lib/homie-esp8266/src/Homie/Timer.hpp"

namespace Tracker {
  class GPSNode {
    public:
      GPSNode(HomieNode homieNode);
      void setup();
      void loop();
    private:
      SDQueue _sdQueue;
      HomieInternals::Timer _gpsTimer;
      bool _messageType;
      char _gpsRecord[70];
      char _tmpGpsRecord[70];
      void _readGpsRecord(const char* prefix, char* gpsRecord);
      HomieNode _homieNode;
  };
}
