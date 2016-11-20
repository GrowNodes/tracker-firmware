#include <Homie.h>
#include "utils/SDQueue.hpp"
#include "../lib/homie-esp8266/src/Homie/Timer.hpp"

namespace Tracker {
  const int GPS_RECORD_LENGTH = 100;
  const int UPLOAD_BUFFER_LENGTH = 1300;
  const int GPS_STORAGE_MAX_RECORDS = 300000;
  const int GPS_STORAGE_BUFFER_SIZE = 10;
  const int UPLOAD_MIN_SAMPLES = 20;
  class GPSNode {
    public:
      GPSNode();
      ~GPSNode();
      void setup();
      void loop();
    private:
      HomieNode _homieNode;
      SDQueue _sdQueue;
      HomieInternals::Timer _gpsTimer;
      HomieInternals::Timer _metricsTimer;
//      String _uploadServerHost = String("api.devices.stutzthings.com");
      HomieSetting<const char*> _uploadServerHost;
//      int _uploadServerPort = 80;
      HomieSetting<long> _uploadServerPort;
      String _uploadServerUri;
      bool _messageType = false;
      char* _gpsRecord;
      char* _gpsUploadBuffer;
      void _sendNextGpsData();
      void _reportMetrics();
      bool _readGpsRecord(const char* prefix, char* gpsRecord);
      bool _validateNmeaChecksum(char* gpsRecord);
      // bool _onSetClearPendingData(const HomieRange& range, const String& value);
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
  };
}
