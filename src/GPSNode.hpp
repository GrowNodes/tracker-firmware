#include <Homie.h>
#include "utils/SDQueue.hpp"
#include "../lib/homie-esp8266/src/Homie/Timer.hpp"

namespace Tracker {
  const int GPS_RECORD_LENGTH = 100;
  const int UPLOAD_BUFFER_LENGTH = 1300;
  const int GPS_STORAGE_MAX_RECORDS = 100000;
  const int GPS_STORAGE_BUFFER_SIZE = 10;
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
      String _uploadServerHost = String("api.stutzthings.com");
      int _uploadServerPort = 80;
      String _uploadServerUri = "tst";
      bool _messageType = false;
      char* _gpsRecord;
      char* _gpsUploadBuffer;
      void _sendNextGpsData();
      void _reportMetrics();
      bool _readGpsRecord(const char* prefix, char* gpsRecord);
      bool _validateNmeaChecksum(char* gpsRecord);
      int _fromHex(char a);
      int _totalRecordsReadSuccess = 0;
      int _totalRecordsReadError = 0;
      int _totalUploadRecordCRCError = 0;
      int _totalUploadRecordsSuccess = 0;
      int _totalUploadCountSuccess = 0;
      int _totalUploadTimeSuccess = 0;
      int _totalUploadCountError = 0;
      int _totalUploadTimeError = 0;
      int _totalRecordsPendingUpload = 0;
  };
}
