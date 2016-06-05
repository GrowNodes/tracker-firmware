#include "SDQueue.hpp"
#include <SD.h>

using namespace Tracker;

SDQueue::SDQueue(String name, int maxQueueSize, int recordBytes, int bufferRecords) :
  _name(name),
  _meta(maxQueueSize),
  _buffer(bufferRecords, recordBytes),
  _maxFileSize(maxQueueSize*recordBytes) {
}

void SDQueue::setup() {
  if (!SD.begin()) {
    Serial.println("✖ SD initialization error");
  } else {
    Serial.println("SD initialized OK");
  }
}

void SDQueue::push(char* record) {
  this->_buffer.push(record);
  if(this->_buffer.isFull()) {
    this->flush();
  }
}

bool SDQueue::poll(char* record) {
  this->flush();
  if(!this->_meta.isEmpty()) {
    File _file = SD.open(this->_name, FILE_READ);
    int pos = this->_meta.poll() * this->_buffer.getBufferElementSize();
    _file.readBytes(&this->_record, this->_buffer.getBufferElementSize());
    _file.close();
    return true;
  } else {
    return false;
  }
}

void SDQueue::flush() {
  File _file = SD.open(this->_name, FILE_WRITE);
  if (!_file) {
    Serial.println("✖ Failed to open file for writing");

  } else {
    Serial.printf("Opened file for flushing buffer. buffer records=%d\n", this->_buffer.getCount());
    while(!this->_buffer.isEmpty()) {
      this->_buffer.poll(&_record);
      int pos = this->_meta.push() * this->_buffer.getBufferElementSize();
      //expand file size as needed
      if(_file.size()<pos) {
        Serial.println("Expanding queue file");
        _file.seek(_file.size());
        int expectedSize = min(_maxFileSize, pos + (this->_buffer.getBufferElementSize()*1000)) - 1;
        while(_file.size() < expectedSize) {
          _file.write((byte)0);
        }
      } else {
        _file.seek(pos);
      }
      _file.write(&this->_record, this->_buffer.getBufferElementSize());
      _file.write("\n");
    }
    _file.close();
    this->_buffer.empty();
    Serial.println("Buffer flushed to disk");
  }
}

int SDQueue::getSize() {
  return this->_meta.getSize();
}

int SDQueue::getCount() {
  return this->_meta.getCount();
}

bool SDQueue::isFull() {
  return this->_meta.isFull();
}

bool SDQueue::isEmpty() {
  return this->_meta.isEmpty();
}

SDQueue::~SDQueue() {
}
