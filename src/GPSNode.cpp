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
  _sdQueue(SDQueue("gps-data", 100, 70, 3)),
  _gpsTimer(HomieInternals::Timer()),
  _metricsTimer(HomieInternals::Timer()),
  _homieNode(HomieNode("gps", "gps")),
  _configNode(configNode),
  _gpsRecord() {
}

void GPSNode::setup() {
  this->_sdQueue.setup();
  this->_gpsTimer.setInterval(500, true);
  this->_metricsTimer.setInterval(10000, true);
  HomieNode h = this->_homieNode;
}

void GPSNode::loop() {
  char* recordBuffer = &this->_gpsRecord[0];

  //upload gps data to cloud
  if(Homie.isReadyToOperate()) {
    if(this->_sdQueue.getCount() > 70) {
      this->_sendNextGpsData();
    }

    if(this->_metricsTimer.check()) {
      this->_metricsTimer.tick();
      Serial.println("Reporting metrics");
      this->_reportMetrics();
    }
  }

  //record gps messages
  if(this->_gpsTimer.check()) {
    this->_gpsTimer.tick();

    //read message from module
    bool valid = false;
    if(this->_messageType) {
      Serial.println("Recording GGA GPS message");
      valid = this->_readGpsRecord(GPS_GGA, recordBuffer);
    } else {
      Serial.println("Recording RMC GPS message");
      valid = this->_readGpsRecord(GPS_RMC, recordBuffer);
    }
    this->_messageType = !this->_messageType;

    //check data integrity and record
    if(valid) {
      this->_sdQueue.push(recordBuffer);
      this->_totalRecordsReadSuccess++;
      Serial.println("GPS record stored");

      //send position online if connected
      if(Homie.isReadyToOperate()) {
        if(this->_messageType) {
          Homie.setNodeProperty(this->_homieNode, "rmc", recordBuffer);
        } else {
          Homie.setNodeProperty(this->_homieNode, "gga", recordBuffer);
        }
      }

    } else {
      this->_totalRecordsReadError++;
      Serial.printf("GPS read error: %s\n", recordBuffer);
    }
  }
}

void GPSNode::_sendNextGpsData() {

  Serial.println("Sending gps data to server");
  this->_sdQueue.flush();//avoid paralel flush() during server connection (too much mem)

  //connect to server. multiple posts may be sent over this same connection (HTTP 1.1)
  int startTime = millis();

  // for(int i=0; i<10; i++) {
    WiFiClient client;
    Serial.printf("Host=%s:%d\n", this->_configNode.getUploadServerHost().c_str(), this->_configNode.getUploadServerPort());
    if (!client.connect(this->_configNode.getUploadServerHost().c_str(), this->_configNode.getUploadServerPort())) {
      Serial.println("Upload: Server connection failed");
      return;
    }

    //fill post with ~1200 bytes
    int len = 0;
    int sdRecords = 0;
    while(this->_sdQueue.peek(this->_gpsRecord, sdRecords++) && len < 1150) {
      strcpy((char*)(this->_gpsUploadBuffer + len), this->_gpsRecord);
      len += strlen(this->_gpsRecord);
      strcpy((char*)(this->_gpsUploadBuffer + len++), "\n");
    }

    if(len>0) {
      Serial.println("Uploading gps data. records=" + String(sdRecords-1));
      Serial.println("POST /v1/" + String(this->_configNode.getUploadServerUri()));
      client.printf("POST /v1/%s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n",
                      this->_configNode.getUploadServerUri().c_str(),
                      this->_configNode.getUploadServerHost().c_str(),
                      len
      );
      client.print(this->_gpsUploadBuffer);
      Serial.print(".");

      //wait for response available
      int timeout = millis();
      while (client.available() < 15) {
        if (millis() - timeout > 5000) {
          Serial.println("Upload: server response timeout");
          client.stop();
          this->_totalUploadCountError++;
          this->_totalUploadTimeError+=(millis()-startTime);
          return;
        }
        delay(50);
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
      Serial.println("Upload: post finished");

    } else {
      Serial.println("No gps data pending upload");
      // break;
    }
    client.stop();
  // }

}

void GPSNode::_reportMetrics() {
  this->_totalRecordsPendingUpload = this->_sdQueue.getCount();

  Homie.setNodeProperty(this->_homieNode, "totalUploadCountSuccess", String(this->_totalUploadCountSuccess));
  Homie.setNodeProperty(this->_homieNode, "totalUploadTimeSuccess", String(this->_totalUploadTimeSuccess));
  Homie.setNodeProperty(this->_homieNode, "totalUploadRecordsSuccess", String(this->_totalUploadRecordsSuccess));
  Homie.setNodeProperty(this->_homieNode, "totalUploadCountError", String(this->_totalUploadCountError));
  Homie.setNodeProperty(this->_homieNode, "totalUploadTimeError", String(this->_totalUploadTimeError));
  Homie.setNodeProperty(this->_homieNode, "totalRecordsReadSuccess", String(this->_totalRecordsReadSuccess));
  Homie.setNodeProperty(this->_homieNode, "totalRecordsReadError", String(this->_totalRecordsReadError));
  Homie.setNodeProperty(this->_homieNode, "totalRecordsPendingUpload", String(this->_totalRecordsPendingUpload));
}

bool GPSNode::_readGpsRecord(const char* prefix, char* gpsRecord) {
  int t = 0;
  bool valid = false;
  do {
    Serial.find(prefix);
    String str = Serial.readStringUntil('\n');
    sprintf(gpsRecord, "%s%s", prefix, str.c_str());
    Serial.println(gpsRecord);
    valid = _validateNmeaChecksum(gpsRecord);
    yield();
  } while(!valid && t++<4);

  return valid;
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
