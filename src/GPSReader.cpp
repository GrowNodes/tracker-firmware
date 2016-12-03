#include <Arduino.h>
#include "GPSReader.hpp"

using namespace Tracker;

const char* GPS_GGA = "$GPGGA";
const char* GPS_RMC = "$GPRMC";

//String name, int maxQueueSize, int recordBytes, int bufferRecords
GPSReader::GPSReader(Watchdog watchdog, GPSUploader gpsUploader) : HomieNode("gps-reader", "gps"),
  _watchdog(watchdog),
  _gpsUploader(gpsUploader),
  _gpsTimer(HomieInternals::Timer()),
  _metricsTimer(HomieInternals::Timer()) {
    this->_gpsRecord = (char*)malloc(GPS_RECORD_LENGTH);
    Serial.setTimeout(200);//~2 nmea messages at 9600bps
    _watchdog.ping();
}

GPSReader::~GPSReader() {
  free(this->_gpsRecord);
}

void GPSReader::setup() {
  Serial.println("\n--Initializing GPSReader");
  this->_gpsTimer.setInterval(1000, true);
  this->_metricsTimer.setInterval(60000, true);

  this->_initialized = true;
  Serial.println("GPSReader setup OK");

  _watchdog.ping();
}

void GPSReader::loop() {
  if(!this->_initialized) return;
  char* recordBuffer = this->_gpsRecord;

  if(Homie.isConnected()) {
    if(this->_metricsTimer.check()) {
      this->_metricsTimer.tick();
      Serial.println("Reporting GPSReader metrics");
      this->_reportMetrics();
    }
  }

  //record gps messages
  if(this->_gpsTimer.check()) {
    this->_gpsTimer.tick();

    //read message from module
    bool valid = false;
    _watchdog.ping();
    if(this->_messageType) {
      valid = this->_readGpsRecord(GPS_GGA, recordBuffer);
    } else {
      valid = this->_readGpsRecord(GPS_RMC, recordBuffer);
    }
    this->_messageType = !this->_messageType;
    _watchdog.ping();

    //check data integrity and record
    if(valid) {
      this->_gpsUploader.addGpsMessage(recordBuffer);
      this->_totalRecordsReadSuccess++;
      // Serial.println(recordBuffer);
      // Serial.printf("GPS record stored (%d) %s\n", this->_sdQueue.getCount(), recordBuffer);

      //send position online if connected
      if(Homie.isConnected()) {
        _watchdog.ping();
        if(this->_messageType) {
          setProperty("rmc").send(recordBuffer);
        } else {
          setProperty("gga").send(recordBuffer);
        }
      }

    } else {
      this->_totalRecordsReadError++;
      Serial.printf("GPS read error: %s\n", recordBuffer);
    }
  }
}

void GPSReader::_reportMetrics() {
  if(!this->_initialized) return;
  _watchdog.ping();
  setProperty("totalRecordsReadSuccess").send(String(this->_totalRecordsReadSuccess));
  setProperty("totalRecordsReadError").send(String(this->_totalRecordsReadError));
  _watchdog.ping();
}

bool GPSReader::_readGpsRecord(const char* prefix, char* gpsRecord) {
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
    valid = GPSUtils::validateNmeaChecksum(gpsRecord);
    yield();
    _watchdog.ping();
  } while(!valid && t++<2);//4

  return valid;
}
