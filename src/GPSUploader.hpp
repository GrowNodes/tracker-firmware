#include <Homie.h>
#include "Constants.hpp"
#include "utils/SDQueue.hpp"
#include "utils/GPSUtils.hpp"
#include "Watchdog.hpp"
#include "../lib/homie-esp8266/src/Homie/Timer.hpp"

namespace Tracker {
  class GPSUploader:public HomieNode {
    public:
      GPSUploader(Watchdog watchdog);
      ~GPSUploader();
      void setup();
      void loop();
      void addGpsMessage(char* gpsMessage);
    private:
      Watchdog _watchdog;
      SDQueue _sdQueue;
      HomieInternals::Timer _gpsTimer;
      HomieInternals::Timer _metricsTimer;
      bool _initialized = false;
      HomieSetting<const char*> _uploadServerHost;
      HomieSetting<long> _uploadServerPort;
      String _uploadServerUri;
      char* _gpsRecord;
      char* _gpsUploadBuffer;
      void _sendNextGpsData();
      void _reportMetrics();
      unsigned long _totalUploadRecordCRCError = 0;
      unsigned long _totalUploadRecordsSuccess = 0;
      unsigned long _totalUploadCountSuccess = 0;
      unsigned long _totalUploadTimeSuccess = 0;
      unsigned long _totalUploadCountError = 0;
      unsigned long _totalUploadTimeError = 0;
      unsigned long _totalUploadsPendingRecords = 0;
      unsigned long _totalUploadBytesSuccess = 0;
      unsigned long _totalUploadBytesError = 0;
      unsigned long _totalUploadPendingRecords = 0;
      // bool _onSetClearPendingData(const HomieRange& range, const String& value);
  };
}
