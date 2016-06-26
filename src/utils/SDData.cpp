#include "SDData.hpp"
#include <SD.h>

using namespace Tracker;

SDData::SDData(String name) :
  _name(name + ".p") {
}

void SDData::setup() {
  if (!SD.begin(4, SPI_HALF_SPEED)) {
    Serial.println("✖ SD initialization error");
  } else {
    Serial.println("SD initialized OK");
  }
}

int SDData::getIntValue() {
  File _file = SD.open(this->_name, FILE_READ);
  if(_file) {
    int v = _file.parseInt();
    _file.close();
    return v;
  } else {
    return 0;
  }
}

void SDData::setIntValue(int value) {
  File _file = SD.open(this->_name, FILE_WRITE);
  if(_file) {
    _file.seek(0);
    _file.printf("#%d", value);
    _file.close();
  }
}
