#include <Homie.h>
#include "GPSUploader.hpp"

namespace Tracker {
  class GPSReader:public HomieNode {
    public:
      GPSReader(Watchdog watchdog, GPSUploader gpsUploader);
      ~GPSReader();
      void setup();
      void loop();
    private:
      Watchdog _watchdog;
      GPSUploader _gpsUploader;
      HomieInternals::Timer _gpsTimer;
      HomieInternals::Timer _metricsTimer;
      bool _initialized = false;
      bool _messageType = false;
      char* _gpsRecord;
      void _reportMetrics();
      bool _readGpsRecord(const char* prefix, char* gpsRecord);
      unsigned long _totalRecordsReadSuccess = 0;
      unsigned long _totalRecordsReadError = 0;
  };
}
