#include <Arduino.h>

namespace Tracker {
  class GPSUtils {
    public:
      static bool validateNmeaChecksum(char* gpsRecord);
    private:
      static int _fromHex(char a);
  };
}
