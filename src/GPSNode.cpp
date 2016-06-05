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
    if(this->_messageType) {
      this->_readGpsRecord(GPS_GGA, recordBuffer);
    } else {
      this->_readGpsRecord(GPS_RMC, recordBuffer);
    }
    this->_messageType = !this->_messageType;
    this->_sdQueue.push(recordBuffer);
    this->_gpsTimer.tick();
  }
}

void GPSNode::_readGpsRecord(const char* prefix, char* gpsRecord) {
  Serial.find(prefix);
  Serial.readBytesUntil('\n', this->_tmpGpsRecord, 70);
  sprintf(gpsRecord, "%s%s", prefix, this->_tmpGpsRecord);
}
