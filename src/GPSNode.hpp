#include <Homie.h>
#include "utils/SDQueue.hpp"
#include "ConfigNode.hpp"
#include "../lib/homie-esp8266/src/Homie/Timer.hpp"

namespace Tracker {
  class GPSNode {
    public:
      GPSNode(ConfigNode configNode);
      void setup();
      void loop();
    private:
      SDQueue _sdQueue;
      HomieInternals::Timer _gpsTimer;
      bool _messageType = false;
      char _gpsRecord[70];
      char _gpsUploadBuffer[1300];
      void _sendNextGpsData();
      void _readGpsRecord(const char* prefix, char* gpsRecord);
      bool _validateNmeaChecksum(char* gpsRecord);
      int _fromHex(char a);
      HomieNode _homieNode;
      ConfigNode _configNode;
      int _totalUploadRecordsSuccess = 0;
      int _totalUploadCountSuccess = 0;
      int _totalUploadTimeSuccess = 0;
      int _totalUploadCountError = 0;
      int _totalUploadTimeError = 0;
  };
}
