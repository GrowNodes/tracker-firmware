#include <Homie.h>
#include <ESP8266WiFi.h>
#include <ESP8266HttpClient.h>
#include "GPSNode.hpp"

//BENCHMARKS RESULTS
//ADSL 5mbps
//MQTT: 2 messages/s (size 70 bytes each)
//POST 545 bytes/s

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
  HomieNode h = this->_homieNode;
  this->_homieNode.subscribe("factoryReset", [h](String value) {
    if(value == "true") {
      Serial.println("Factory reset requested");
      Homie.setNodeProperty(h, "factoryReset", "false");
      Homie.eraseConfig();
      ESP.restart();
    }
    return true;
  });
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

    //---- BEGIN HTTP POST TEST
    WiFiClient client;
    if (!client.connect("api.stutzthings.com", 80)) {
      Serial.println("connection failed");
      return;
    }
    //http.begin("http://api.stutzthings.com/v1/test");
    Serial.println("Initiating POST");
    client.print(String("POST /v1/test HTTP/1.1\r\n") +
                  String("Host: api.stutzthings.com\r\n") +
                  String("Content-Length: 36000\r\n") +
                  String("Content-Type: text/plain\r\n\r\n"));
    //1k - 1800ms (10 x 100) - 550B/s
    //2k - 1800ms (10 x 200) - 1kB/s
    //3k - 1800ms (10 x 300) - 1.6kB/s
    //4k - 1850ms (10 x 400) -> 2kB/s
    //6k - 1900ms (10 x 600) -> 3kB/s
    //12k - 1980ms (10 x 1200) -> 6kB/s <- optimal chunk size (1200 bytes)
    //24k - 3970ms (10 x 2400) -> 6kB/s <- stagnado
    //36k - 6600 (30 x 1200) -> 5.4kB/s
    int s = millis();
    for(int i=0; i<30; i++) {
      client.print("012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
      Serial.print(".");
      yield();
    }
    Serial.print(millis()-s);
    int timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout!");
        client.stop();
        return;
      }
    }
    while(client.available()) {
      // client.readString();
      Serial.print(client.readString());
    }
    Serial.println("POST FINISHED");
    client.stop();
    //---- END TEST

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

      //Homie.setNodeProperty(this->_homieNode, "data", recordBuffer);
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
