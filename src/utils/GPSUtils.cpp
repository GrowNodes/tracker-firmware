#include "GPSUtils.hpp"

using namespace Tracker;

// this takes a nmea gps string, and validates it againts the checksum
// at the end if the string. (requires first byte to be $)
bool GPSUtils::validateNmeaChecksum(char* gpsRecord) {
  int len = strlen(gpsRecord);
  byte realCrc = 0;
  for (int i=1; i<len-4; i++) {
    realCrc ^= (byte)*(gpsRecord+i);
  }
  byte correctCrc = (byte)(16 * GPSUtils::_fromHex(*(gpsRecord+(len-3))) + GPSUtils::_fromHex(*(gpsRecord+(len-2))));

  return realCrc == correctCrc;
}

int GPSUtils::_fromHex(char a) {
  if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else
    return a - '0';
}
