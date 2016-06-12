#include <ESP8266WiFi.h>
#include <ESP8266HttpClient.h>
#include "GPSNode.hpp"

//BENCHMARK RESULTS
//ADSL 5mbps
//MQTT: 2 messages/s (size 70 bytes each)
//POST:
//1k - 1800ms (10 x 100) - 550B/s
//2k - 1800ms (10 x 200) - 1kB/s
//3k - 1800ms (10 x 300) - 1.6kB/s
//4k - 1850ms (10 x 400) -> 2kB/s
//6k - 1900ms (10 x 600) -> 3kB/s
//12k - 1980ms (10 x 1200) -> 6kB/s <- optimal chunk size (1200 bytes)
//24k - 3970ms (10 x 2400) -> 6kB/s <- stagnado
//36k - 6600 (30 x 1200) -> 5.4kB/s

using namespace Tracker;

const char* GPS_GGA = "$GPGGA";
const char* GPS_RMC = "$GPRMC";
const char CHAR_SPACE = ' ';

//String name, int maxQueueSize, int recordBytes, int bufferRecords
GPSNode::GPSNode(ConfigNode configNode) :
  _sdQueue(SDQueue("gps-data", 10, 70, 5)),
  _gpsTimer(HomieInternals::Timer()),
  _homieNode(HomieNode("gps", "gps")),
  _configNode(configNode) {
}

void GPSNode::setup() {
  this->_sdQueue.setup();
  this->_gpsTimer.setInterval(3000, true);
  HomieNode h = this->_homieNode;
}

void GPSNode::loop() {
  char* recordBuffer = &this->_gpsRecord[0];

  //upload gps data to cloud
  if(WiFi.status()==WL_CONNECTED) {
    //send 2 messages max at each loop step
    this->_sendNextGpsData();
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
      Serial.println("GPS record stored");
    } else {
      Serial.println("GPS record invalid");
    }
    this->_gpsTimer.tick();
  }
}

void GPSNode::_sendNextGpsData() {
  Serial.println("Sent gps data to server");
  int startTime = millis();

  //connect to server
  WiFiClient client;
  if (!client.connect(this->_configNode.getUploadServerHost().c_str(), this->_configNode.getUploadServerPort())) {
    Serial.println("Upload: Server connection failed");
    return;
  }

  Serial.println("Upload: Initiating POST");
  //perform a chuked upload. chunk size~=1200 bytes; total payload~=12000 bytes
  client.print(String("POST /v1/" + String(Homie.getBaseTopic()) + " HTTP/1.1\r\n") +
                String("Host: "+ String(this->_configNode.getUploadServerHost()) +"\r\n") +
                String("Transfer-Encoding: chunked\r\n") +
                String("Content-Type: text/plain\r\n\r\n"));
  int sdRecords = 0;
  for(int i=0; i<10; i++) {
    //fill chunk with ~1200 bytes
    int len = 0;
    while(this->_sdQueue.peek(this->_gpsRecord, sdRecords++) && len < 1150) {
      strcpy(this->_gpsUploadBuffer + len, this->_gpsRecord);
      len += strlen(this->_gpsRecord);
      strcpy(this->_gpsUploadBuffer + len++, "\n");
      strcpy(this->_gpsUploadBuffer + len++, 0);
    }
    //send chunk
    client.print(String(strlen(this->_gpsUploadBuffer)) + "\r\n");
    client.print(this->_gpsUploadBuffer);
    Serial.print(".");
    yield();
  }
  client.print("0\r\n\r\n");
  Serial.print("Upload: chunks sent. records=" + String(sdRecords));

  //wait for response available
  int timeout = millis();
  while (client.available() < 15) {
    if (millis() - timeout > 5000) {
      Serial.print("Upload: server timeout");
      client.stop();
      return;
    }
  }

  //process response
  bool success = false;
  if(client.readStringUntil(CHAR_SPACE).length()>0) {
    String code = client.readStringUntil(CHAR_SPACE);
    if(code == "200") {
      Serial.println("Upload: server 200 OK");
      success = true;
      this->_totalUploadCountSuccess++;
      this->_totalUploadTimeSuccess+=(millis()-startTime);
      this->_totalUploadRecordsSuccess+=sdRecords;
      this->_sdQueue.removeElements(sdRecords);
      Serial.println("Upload: sent records removed from disk");
    } else {
      this->_totalUploadCountError++;
      this->_totalUploadTimeError+=(millis()-startTime);
      Serial.println("Upload: server error " + String(code));
    }
  } else {
    this->_totalUploadCountError++;
    this->_totalUploadTimeError+=(millis()-startTime);
    Serial.println("Upload: invalid server response");
  }

  client.stop();
  Serial.println("Upload: post finished");

  //metrics
  if(success) {
    Homie.setNodeProperty(this->_homieNode, "totalUploadCountSuccess", String(this->_totalUploadCountSuccess));
    Homie.setNodeProperty(this->_homieNode, "totalUploadTimeSuccess", String(this->_totalUploadTimeSuccess));
    Homie.setNodeProperty(this->_homieNode, "totalUploadRecordsSuccess", String(this->_totalUploadRecordsSuccess));
  } else {
    Homie.setNodeProperty(this->_homieNode, "totalUploadCountError", String(this->_totalUploadCountError));
    Homie.setNodeProperty(this->_homieNode, "totalUploadTimeError", String(this->_totalUploadTimeError));
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
