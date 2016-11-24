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
  _uploadServerHost(HomieSetting<const char*>("uploadServerHost", "Host to receive POST with bulk GPS positions")),
  _uploadServerPort(HomieSetting<long>("uploadServerPort", "Port for the POST of bulk GPS positions")),
  _sdQueue(SDQueue("gpsqueue", GPS_STORAGE_MAX_RECORDS, GPS_RECORD_LENGTH, GPS_STORAGE_BUFFER_SIZE)),
  _gpsTimer(HomieInternals::Timer()),
  _metricsTimer(HomieInternals::Timer()) {
    this->_gpsRecord = (char*)malloc(GPS_RECORD_LENGTH);
    this->_gpsUploadBuffer = (char*)malloc(UPLOAD_BUFFER_LENGTH);
    Serial.setTimeout(200);//~2 nmea messages at 9600bps
    this->_uploadServerHost.setDefaultValue("api.devices.stutzthings.com");
    this->_uploadServerPort.setDefaultValue(80);
}

GPSNode::~GPSNode() {
  free(this->_gpsRecord);
  free(this->_gpsUploadBuffer);
}

bool gpsNode_configMode(const HomieRange& range, const String& value) {
  Serial.printf("ENTERCONFIG CHANGED TO %s", value.c_str());
  Homie.setBootModeNextBoot(Homie.MODE_CONFIG);
  Homie.reboot();
}

void GPSNode::setup() {
  this->_uploadServerUri = Homie.getConfiguration().mqtt.baseTopic + String(Homie.getConfiguration().deviceId) + String("/gps/raw");
  this->_sdQueue.setup();
  this->_gpsTimer.setInterval(1000, true);
  this->_metricsTimer.setInterval(60000, true);

  Serial.printf("Upload POST URL=http://%s:%d/%s\n", this->_uploadServerHost.get(), this->_uploadServerPort.get(), this->_uploadServerUri.c_str());

  // this->_homieNode.advertise("clearPendingData").settable(this->_onSetClearPendingData);
  this->_initialized = true;
  Serial.println("GPS setup OK");

  // this->_homieNode.advertise("enterConfig").settable(std::function<bool(const HomieRange&, const String&)> ([] (const HomieRange& range, const String& value) {
  // }));
  this->_homieNode.advertise("enterConfig").settable(gpsNode_configMode);
}

void GPSNode::loop() {
  if(!this->_initialized) return;
  char* recordBuffer = this->_gpsRecord;

  //upload gps data to cloud
  if(Homie.isConnected()) {
    if(this->_sdQueue.getCount() > UPLOAD_MIN_SAMPLES) {
      //this->_sendNextGpsData();
    }

    if(this->_metricsTimer.check()) {
      this->_metricsTimer.tick();
      Serial.println("Reporting GPS metrics");
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
      if(Homie.isConnected()) {
        if(this->_messageType) {
          this->_homieNode.setProperty("rmc").send(recordBuffer);
        } else {
          this->_homieNode.setProperty("gga").send(recordBuffer);
        }
      }

    } else {
      this->_totalRecordsReadError++;
      Serial.printf("GPS read error: %s\n", recordBuffer);
    }
  }
}

// bool GPSNode::_onSetClearPendingData(const HomieRange& range, const String& value) {
//   if(strcmp(value.c_str(), "true") == 0) {
//     Serial.println("Clearing pending messages from internal persistent queue");
//     this->_sdQueue.removeElements(this->_sdQueue.getCount());
//     this->_homieNode.setProperty("clearPendingData").setRange(range).send("done");
//   }
// }

void GPSNode::_sendNextGpsData() {
  if(!this->_initialized) return;
  Serial.printf("Upload: Pending messages: %d\n", this->_sdQueue.getCount());
  Serial.println("Upload: preparation");
  this->_sdQueue.flush();//avoid parallel flush() during server connection (too much mem)

  if(this->_sdQueue.getCount()==0) {
    Serial.println("Upload: no data to send");
    return;
  }

  //connect to server and send various POST on the same connection
  WiFiClient client;
  // Serial.printf("Host=%s:%d\n", this->_configNode.getUploadServerHost().c_str(), this->_configNode.getUploadServerPort());
  int startTime = millis();
  if (!client.connect(this->_uploadServerHost.get(), this->_uploadServerPort.get())) {
     this->_totalUploadCountError++;
     this->_totalUploadTimeError+=(millis()-startTime);
     Serial.println("Upload: server connection failed");
     return;
  }

  //send
  //TODO Test best numbers for optimal throughput without losing mqtt connection
  for(int i=0; i<1; i++) {
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

      Serial.println("POST /" + String(this->_uploadServerUri.c_str()));
      int startTime = millis();
      client.printf("POST /%s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n",
                      this->_uploadServerUri.c_str(),
                      "api.devices.stutzthings.com",
                      // this->_uploadServerHost.get(),
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
  if(!this->_initialized) return;
  this->_totalRecordsPendingUpload = this->_sdQueue.getCount();

  this->_homieNode.setProperty("totalUploadRecordCRCError").send(String(this->_totalUploadRecordCRCError));
  this->_homieNode.setProperty("totalUploadCountSuccess").send(String(this->_totalUploadCountSuccess));
  this->_homieNode.setProperty("totalUploadTimeSuccess").send(String(this->_totalUploadTimeSuccess));
  this->_homieNode.setProperty("totalUploadRecordsSuccess").send(String(this->_totalUploadRecordsSuccess));
  this->_homieNode.setProperty("totalUploadCountError").send(String(this->_totalUploadCountError));
  this->_homieNode.setProperty("totalUploadTimeError").send(String(this->_totalUploadTimeError));
  this->_homieNode.setProperty("totalRecordsReadSuccess").send(String(this->_totalRecordsReadSuccess));
  this->_homieNode.setProperty("totalRecordsReadError").send(String(this->_totalRecordsReadError));
  this->_homieNode.setProperty("totalRecordsPendingUpload").send(String(this->_totalRecordsPendingUpload));
}

bool GPSNode::_readGpsRecord(const char* prefix, char* gpsRecord) {
  int t = 0;
  bool valid = false;
  do {
    Serial.find(prefix);
    // Serial.println("GPS READ UNTIL1");
    String tmp = Serial.readStringUntil('\n');
    // Serial.println("GPS READ UNTIL2");
    strcpy(gpsRecord, prefix);
    //truncate readings
    memcpy(gpsRecord+strlen(prefix), tmp.c_str(), GPS_RECORD_LENGTH-strlen(prefix));
    //force string termination
    memset(gpsRecord+GPS_RECORD_LENGTH-1, 0, 1);
    valid = _validateNmeaChecksum(gpsRecord);
    yield();
  } while(!valid && t++<2);//4

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
