#include <Homie.h>
#include "utils/SDQueue.hpp"
#include "Watchdog.hpp"
#include "../lib/homie-esp8266/src/Homie/Timer.hpp"

namespace Tracker {
  const int GPS_RECORD_LENGTH = 100;
  const int UPLOAD_BUFFER_LENGTH = 1300;
  const int GPS_STORAGE_MAX_RECORDS = 300000;
  const int GPS_STORAGE_BUFFER_SIZE = 10;
  const int UPLOAD_MIN_SAMPLES = 20;
  class GPSNode:public HomieNode {
    public:
      GPSNode(Watchdog watchdog);
      ~GPSNode();
      void setup();
      void loop();
    private:
      Watchdog _watchdog;
      SDQueue _sdQueue;
      HomieInternals::Timer _gpsTimer;
      HomieInternals::Timer _metricsTimer;
      bool _initialized = false;
      HomieSetting<const char*> _uploadServerHost;
      HomieSetting<long> _uploadServerPort;
      String _uploadServerUri;
      bool _messageType = false;
      char* _gpsRecord;
      char* _gpsUploadBuffer;
      void _sendNextGpsData();
      void _reportMetrics();
      bool _readGpsRecord(const char* prefix, char* gpsRecord);
      bool _validateNmeaChecksum(char* gpsRecord);
      int _fromHex(char a);
      unsigned long _totalRecordsReadSuccess = 0;
      unsigned long _totalRecordsReadError = 0;
      unsigned long _totalUploadRecordCRCError = 0;
      unsigned long _totalUploadRecordsSuccess = 0;
      unsigned long _totalUploadCountSuccess = 0;
      unsigned long _totalUploadTimeSuccess = 0;
      unsigned long _totalUploadCountError = 0;
      unsigned long _totalUploadTimeError = 0;
      unsigned long _totalRecordsPendingUpload = 0;
      unsigned long _totalUploadBytesSuccess = 0;
      unsigned long _totalUploadBytesError = 0;
      // bool _onSetClearPendingData(const HomieRange& range, const String& value);
  };
}
