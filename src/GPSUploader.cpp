#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HttpClient.h>
#include "GPSUploader.hpp"

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

const char* GGPS_GGA = "$GPGGA";
const char* GGPS_RMC = "$GPRMC";
const char GCHAR_SPACE = ' ';

//String name, int maxQueueSize, int recordBytes, int bufferRecords
GPSUploader::GPSUploader(Watchdog watchdog) : HomieNode("gps-uploader", "gps"),
  _watchdog(watchdog),
  _uploadServerHost(HomieSetting<const char*>("uploadServerHost", "Host to receive POST with bulk GPS positions")),
  _uploadServerPort(HomieSetting<long>("uploadServerPort", "Port for the POST of bulk GPS positions")),
  _sdQueue(SDQueue("gpsqueue", GPS_STORAGE_MAX_RECORDS, GPS_RECORD_LENGTH, GPS_STORAGE_BUFFER_SIZE)),
  _gpsTimer(HomieInternals::Timer()),
  _metricsTimer(HomieInternals::Timer()) {
    this->_gpsRecord = (char*)malloc(GPS_RECORD_LENGTH);
    this->_gpsUploadBuffer = (char*)malloc(UPLOAD_BUFFER_LENGTH);
    this->_uploadServerHost.setDefaultValue("api.devices.stutzthings.com");
    this->_uploadServerPort.setDefaultValue(80);
    _watchdog.ping();
}

GPSUploader::~GPSUploader() {
  free(this->_gpsRecord);
  free(this->_gpsUploadBuffer);
}

void GPSUploader::setup() {
  Serial.println("\n--Initializing GPSUploader");
  this->_uploadServerUri = Homie.getConfiguration().mqtt.baseTopic + String(Homie.getConfiguration().deviceId) + String("/gps/raw");
  this->_sdQueue.setup();
  this->_gpsTimer.setInterval(1000, true);
  this->_metricsTimer.setInterval(60000, true);

  Serial.printf("Upload POST URL=http://%s:%d/%s\n", this->_uploadServerHost.get(), this->_uploadServerPort.get(), this->_uploadServerUri.c_str());

  this->_initialized = true;
  Serial.println("GPSUploader setup OK");

  advertise("clearData").settable([this](const HomieRange& range, const String& value) -> bool {
    if(strcmp(value.c_str(), "true") == 0) {
      Serial.println("Clearing pending messages");
      this->_sdQueue.removeElements(this->_sdQueue.getCount());
      this->setProperty("clearData").setRange(range).send("true");
    }
  });

  // advertise("configMode").settable([this](const HomieRange& range, const String& value) -> bool {
  //   if(strcmp(value.c_str(), "true") == 0) {
  //     Serial.println("CONFIG MODE requested");
  //     Homie.setNextBoot(Homie.MODE_CONFIG);
  //     Homie.reboot();
  //   }
  // });

  _watchdog.ping();
}

void GPSUploader::loop() {
  if(!this->_initialized) return;

  //upload gps data to cloud
  if(Homie.isConnected()) {
    if(this->_sdQueue.getCount() > UPLOAD_MIN_SAMPLES) {
      this->_sendNextGpsData();
    }

    if(this->_metricsTimer.check()) {
      this->_metricsTimer.tick();
      Serial.println("Reporting GPSUploader metrics");
      this->_reportMetrics();
    }
  }
}

void GPSUploader::_sendNextGpsData() {
  int startTimeUploading = millis();
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
  _watchdog.ping();
  int startTime = millis();
  if (!client.connect(this->_uploadServerHost.get(), this->_uploadServerPort.get())) {
     this->_totalUploadCountError++;
     this->_totalUploadTimeError+=(millis()-startTime);
     Serial.println("Upload: server connection failed");
     _watchdog.ping();
     return;
  }
  _watchdog.ping();

  //send
  //If this takes more than 10s, abort and loop again so that Wifi and MQTT won't lose connection
  for(int i=0; i<10 && ((millis()-startTimeUploading)<10000); i++) {
    // int len = 0;
    int sdRecordsCount = 0;
    int sdRecordsOK = 0;
    int sdRecordsError = 0;
    strcpy(this->_gpsUploadBuffer, "");

    // Serial.println("Upload: Preparing chunk");
    while(this->_sdQueue.peek(this->_gpsRecord, sdRecordsCount++) && strlen(this->_gpsUploadBuffer) < 1150) {
      //fill post with ~1200 bytes
      if(GPSUtils::validateNmeaChecksum(this->_gpsRecord)) {
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
      Serial.printf("Upload: records=%d err=%d\n", sdRecordsOK, sdRecordsError);

      Serial.println("POST /" + String(this->_uploadServerUri.c_str()));
      int startTime = millis();
      client.printf("POST /%s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n",
                      this->_uploadServerUri.c_str(),
                      "api.devices.stutzthings.com",
                      // this->_uploadServerHost.get(),
                      strlen(this->_gpsUploadBuffer)
      );
      client.print(this->_gpsUploadBuffer);
      _watchdog.ping();
      // Serial.println("SENT");

      //wait for response available
      int timeout = millis();
      while (client.available() < 15) {
        if (millis() - timeout > 5000) {
          Serial.println("Upload: server response timeout");
          client.stop();
          this->_totalUploadBytesError += strlen(this->_gpsUploadBuffer);
          this->_totalUploadCountError++;
          this->_totalUploadTimeError+=(millis()-startTime);
          return;
        }
        delay(50);
        _watchdog.ping();
      }
      // Serial.println("RECEIVED RESPONSE");

      //process response
      bool success = false;
      if(client.readStringUntil(GCHAR_SPACE).length()>0) {
        String code = client.readStringUntil(GCHAR_SPACE);
        _watchdog.ping();
        if(code == "201") {
          Serial.println("Upload: 201 Created");
          success = true;
          this->_totalUploadBytesSuccess += strlen(this->_gpsUploadBuffer);
          this->_totalUploadCountSuccess++;
          this->_totalUploadTimeSuccess+=(millis()-startTime);
          this->_totalUploadRecordsSuccess+=sdRecordsOK;
          this->_totalUploadRecordCRCError+=sdRecordsError;
          this->_sdQueue.removeElements(sdRecordsCount);
          // Serial.println("Upload: sent records removed from disk");
        } else {
          this->_totalUploadBytesError += strlen(this->_gpsUploadBuffer);
          this->_totalUploadCountError++;
          this->_totalUploadTimeError+=(millis()-startTime);
          Serial.println("Upload: server error " + String(code));
        }
      } else {
        this->_totalUploadBytesError += strlen(this->_gpsUploadBuffer);
        this->_totalUploadCountError++;
        this->_totalUploadTimeError+=(millis()-startTime);
        Serial.println("Upload: invalid server response");
      }
      //drain response data
      // Serial.println("RESPONSE DRAIN0");
      _watchdog.ping();
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
    _watchdog.ping();
    yield();
    _watchdog.ping();
    // Serial.println("FINISHED POST");
  }

  client.stop();
}

void GPSUploader::addGpsMessage(char* gpsMessage) {
  this->_sdQueue.push(gpsMessage);
  Serial.printf("GPS record stored (%d)\n", this->_sdQueue.getCount());
}

void GPSUploader::_reportMetrics() {
  if(!this->_initialized) return;
  this->_totalUploadsPendingRecords = this->_sdQueue.getCount();

  _watchdog.ping();
  setProperty("totalUploadScheduledMessages").send(String(this->_totalUploadRecordCRCError));
  setProperty("totalUploadRecordCRCError").send(String(this->_totalUploadRecordCRCError));
  setProperty("totalUploadCountSuccess").send(String(this->_totalUploadCountSuccess));
  setProperty("totalUploadTimeSuccess").send(String(this->_totalUploadTimeSuccess));
  setProperty("totalUploadRecordsSuccess").send(String(this->_totalUploadRecordsSuccess));
  setProperty("totalUploadCountError").send(String(this->_totalUploadCountError));
  setProperty("totalUploadTimeError").send(String(this->_totalUploadTimeError));
  setProperty("totalUploadPendingRecords").send(String(this->_totalUploadsPendingRecords));
  setProperty("totalUploadBytesSuccess").send(String(this->_totalUploadBytesSuccess));
  setProperty("totalUploadBytesError").send(String(this->_totalUploadBytesError));
  _watchdog.ping();
}
