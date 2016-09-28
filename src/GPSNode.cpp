#include <Arduino.h>
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
//24k - 3970ms (10 x 2400) -> 6kB/s <- estagnado
//36k - 6600 (30 x 1200) -> 5.4kB/s

using namespace Tracker;

const char* GPS_GGA = "$GPGGA";
const char* GPS_RMC = "$GPRMC";
const char CHAR_SPACE = ' ';

//String name, int maxQueueSize, int recordBytes, int bufferRecords
GPSNode::GPSNode() :
  _homieNode(HomieNode("gps", "gps")),
  _sdQueue(SDQueue("gps-data", GPS_STORAGE_MAX_RECORDS, GPS_RECORD_LENGTH, GPS_STORAGE_BUFFER_SIZE)),
  _gpsTimer(HomieInternals::Timer()),
  _metricsTimer(HomieInternals::Timer()) {
    this->_gpsRecord = (char*)malloc(GPS_RECORD_LENGTH);
    this->_gpsUploadBuffer = (char*)malloc(UPLOAD_BUFFER_LENGTH);
}

GPSNode::~GPSNode() {
  free(this->_gpsRecord);
  free(this->_gpsUploadBuffer);
}

void GPSNode::setup() {
  this->_uploadServerUri = Homie.getBaseTopic() + String(Homie.getId()) + String("/gps/raw");
  // this->_uploadServerUri = "test";
  this->_sdQueue.setup();
  this->_gpsTimer.setInterval(500, true);
  this->_metricsTimer.setInterval(60000, true);
}

void GPSNode::loop() {
  char* recordBuffer = this->_gpsRecord;

  //upload gps data to cloud
  if(Homie.isReadyToOperate()) {
    if(this->_sdQueue.getCount() > 90000) {
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
      // Serial.println("Recording GGA GPS message");
      valid = this->_readGpsRecord(GPS_GGA, recordBuffer);
    } else {
      // Serial.println("Recording RMC GPS message");
      valid = this->_readGpsRecord(GPS_RMC, recordBuffer);
    }
    this->_messageType = !this->_messageType;

    //check data integrity and record
    if(valid) {
      this->_sdQueue.push(recordBuffer);
      this->_totalRecordsReadSuccess++;
      // Serial.println(recordBuffer);
      // Serial.printf("GPS record stored (%d) %s\n", this->_sdQueue.getCount(), recordBuffer);
      Serial.printf("GPS record stored (%d)\n", this->_sdQueue.getCount());

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

  Serial.println("Upload: preparation");
  this->_sdQueue.flush();//avoid paralel flush() during server connection (too much mem)

  if(this->_sdQueue.getCount()==0) {
    Serial.println("Upload: no data to send");
    return;
  }

  //connect to server and send various POST on the same connection
  WiFiClient client;
  // Serial.printf("Host=%s:%d\n", this->_configNode.getUploadServerHost().c_str(), this->_configNode.getUploadServerPort());
  if (!client.connect(this->_uploadServerHost.c_str(), this->_uploadServerPort)) {
    Serial.println("Upload: server connection failed");
    return;
  }

  //send
  for(int i=0; i<100; i++) {
    // int len = 0;
    int sdRecordsCount = 0;
    int sdRecordsOK = 0;
    int sdRecordsError = 0;
    strcpy(this->_gpsUploadBuffer, "");

    // Serial.println("Upload: Preparing chunk");
    while(this->_sdQueue.peek(this->_gpsRecord, sdRecordsCount++) && strlen(this->_gpsUploadBuffer) < 1150) {
      //fill post with ~1200 bytes
      if(this->_validateNmeaChecksum(this->_gpsRecord)) {
        strcat(this->_gpsUploadBuffer, this->_gpsRecord);
        strcat(this->_gpsUploadBuffer, "\n");
        sdRecordsOK++;
      } else {
        Serial.printf("Upload: crc error %s\n", this->_gpsRecord);
        sdRecordsError++;
      }
      yield();
    }

    if(strlen(this->_gpsUploadBuffer)>0) {
      Serial.printf("Upload: ok=%d err=%d\n", sdRecordsOK, sdRecordsError);

      Serial.println("POST /" + String(this->_uploadServerUri));
      int startTime = millis();
      client.printf("POST /%s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n",
                      this->_uploadServerUri.c_str(),
                      this->_uploadServerHost.c_str(),
                      strlen(this->_gpsUploadBuffer)
      );
      client.print(this->_gpsUploadBuffer);
      // Serial.println("SENT");

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
      // Serial.println("RECEIVED RESPONSE");

      //process response
      bool success = false;
      if(client.readStringUntil(CHAR_SPACE).length()>0) {
        String code = client.readStringUntil(CHAR_SPACE);
        if(code == "201") {
          Serial.println("Upload: 201 Created");
          success = true;
          this->_totalUploadCountSuccess++;
          this->_totalUploadTimeSuccess+=(millis()-startTime);
          this->_totalUploadRecordsSuccess+=sdRecordsOK;
          this->_totalUploadRecordCRCError+=sdRecordsError;
          this->_sdQueue.removeElements(sdRecordsCount);
          // Serial.println("Upload: sent records removed from disk");
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
      //drain response data
      // Serial.println("RESPONSE DRAIN0");
      while(client.available()>0) {
        client.read();
      }
      // Serial.println("RESPONSE DRAIN1");
      // Serial.println("Upload: post finished");

    } else {
      Serial.println("Upload: no data pending");
      if(sdRecordsError>0) {
        this->_sdQueue.removeElements(sdRecordsError);
        this->_totalUploadRecordCRCError+=sdRecordsError;
      }
      break;
    }
    yield();
    // Serial.println("FINISHED POST");
  }

  client.stop();
}

void GPSNode::_reportMetrics() {
  this->_totalRecordsPendingUpload = this->_sdQueue.getCount();

  Homie.setNodeProperty(this->_homieNode, "totalUploadRecordCRCError", String(this->_totalUploadRecordCRCError));
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
    String tmp = Serial.readStringUntil('\n');
    strcpy(gpsRecord, prefix);
    //truncate readings
    memcpy(gpsRecord+strlen(prefix), tmp.c_str(), GPS_RECORD_LENGTH-strlen(prefix));
    //force string termination
    memset(gpsRecord+GPS_RECORD_LENGTH-1, 0, 1);
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
