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

  //upload gps data do cloud
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
    Serial.printf("GPS: %s\n", recordBuffer);
    if(this->_validateNmeaChecksum(recordBuffer)) {
      Serial.println("GPS record valid. Storing.");
      this->_sdQueue.push(recordBuffer);
    } else {
      Serial.printf("GPS record invalid. %s\n", recordBuffer);
    }
    this->_gpsTimer.tick();
  }
}

void GPSNode::_readGpsRecord(const char* prefix, char* gpsRecord) {
  Serial.find(prefix);
  Serial.readBytesUntil('\n', &this->_tmpGpsRecord[0], 70);
  sprintf(gpsRecord, "%s%s", prefix, this->_tmpGpsRecord);
}

// this takes a nmea gps string, and validates it againts the checksum
// at the end if the string. (requires first byte to be $)
bool GPSNode::_validateNmeaChecksum(char* gpsRecord) {
  unsigned int realCrc = 0;
  int len = strlen(gpsRecord);
  for (int i=0; i<len; i++) {
    realCrc ^= (char)*(gpsRecord+i);
  }
  Serial.printf("realCrcI=%d\n", realCrc);
  parei aqui
  Serial.printf("realCrcH=%d\n", realCrc);

  Serial.printf("correctCrcH=%s\n", gpsRecord+(len-3));
  unsigned int correctCrc = strtoul(gpsRecord+(len-3), NULL, 16);
  Serial.printf("correctCrcI=%s\n", correctCrc);

  return realCrc == correctCrc;
}
