#include <Homie.h>
#include <ESP8266WiFi.h>
#include "GPSNode.hpp"

using namespace Tracker;

const char* GPS_GGA = "$GPGGA";
const char* GPS_RMC = "$GPRMC";

//String name, int maxQueueSize, int recordBytes, int bufferRecords
GPSNode::GPSNode(HomieNode homieNode) :
  _sdQueue(SDQueue("gps-data", 10, 70, 5)),
  _gpsTimer(HomieInternals::Timer()),
  _homieNode(homieNode) {
}

void GPSNode::setup() {
  this->_sdQueue.setup();
  this->_gpsTimer.setInterval(3000, true);
}

void GPSNode::loop() {
  char* recordBuffer = &this->_gpsRecord[0];

  //upload gps data to cloud
  if(WiFi.status()==WL_CONNECTED) {
    //send 2 messages max at each loop step
    for(int i=0; i < min(this->_sdQueue.getCount(), 2); i++) {
      this->_sdQueue.poll(recordBuffer);
      Homie.setNodeProperty(this->_homieNode, "data", recordBuffer);
      Serial.println("Sent gps data to mqtt");
    }
  }

  //record gps messages
  if(this->_gpsTimer.check()) {

    //read message from module
    if(this->_messageType) {
      Serial.println("Recording GGA GPS message");
      this->_readGpsRecord(GPS_GGA, recordBuffer);
    } else {
      Serial.println("Recording RMC GPS message");
      this->_readGpsRecord(GPS_RMC, recordBuffer);
    }
    this->_messageType = !this->_messageType;

    //check data integrity and record
    if(this->_validateNmeaChecksum(recordBuffer)) {
      this->_sdQueue.push(recordBuffer);
      Homie.setNodeProperty(this->_homieNode, "data", recordBuffer);
      Serial.println("Sent gps data to mqtt");
    } else {
      Serial.println("GPS record invalid");
    }
    this->_gpsTimer.tick();
  }
}

void GPSNode::_readGpsRecord(const char* prefix, char* gpsRecord) {
  Serial.find(prefix);
  char tmp[70] = "000000000000000000";
  Serial.readBytesUntil('\n', tmp, sizeof(tmp));
  sprintf(gpsRecord, "%s%s", prefix, tmp);
  // Serial.println(gpsRecord);
}

// this takes a nmea gps string, and validates it againts the checksum
// at the end if the string. (requires first byte to be $)
bool GPSNode::_validateNmeaChecksum(char* gpsRecord) {
  int len = strlen(gpsRecord);
  byte realCrc = 0;
  for (int i=1; i<len-4; i++) {
    realCrc ^= (byte)*(gpsRecord+i);
  }
  byte correctCrc = (byte)(16 * this->_fromHex(*(gpsRecord+(len-3))) + this->_fromHex(*(gpsRecord+(len-2))));

  return realCrc == correctCrc;
}

int GPSNode::_fromHex(char a) {
  if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else
    return a - '0';
}
