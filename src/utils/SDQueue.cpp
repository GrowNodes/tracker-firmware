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
    this->_loadMetaFile();
  }
}

void SDQueue::push(char* record) {
  this->_buffer.push(record);
  if(this->_buffer.isFull()) {
    this->flush();
  }
}

void SDQueue::removeElements(int n) {
  this->_meta.removeElements(n);
  //update metadata file
  this->_writeMetaFile();
}

bool SDQueue::poll(char* record) {
  //load last element
  bool result = this->peek(record);
  if(result) {
    this->removeElements(1);
  }
  return result;
}

bool SDQueue::peek(char* record) {
  return this->peek(record, 0);
}

bool SDQueue::peek(char* record, int n) {
  this->flush();
  if(this->_meta.peek(n)!=-1) {
    File _file = SD.open(this->_name, FILE_READ);
    int pos = this->_meta.peek(n) * this->_buffer.getBufferElementSize();
    _file.readBytes(&this->_record, this->_buffer.getBufferElementSize());
    _file.close();
    return true;
  } else {
    return false;
  }
}

void SDQueue::_writeMetaFile() {
  File _file = SD.open(this->_name + ".meta", FILE_WRITE);
  _file.seek(0);
  _file.print(this->_meta.getCount());
  _file.print(this->_meta.getHead());
  _file.print(this->_meta.getSize());
  _file.print(this->_meta.getTail());
  _file.close();
  Serial.println("Queue metadata saved to disk");
}
void SDQueue::_loadMetaFile() {
  File _file = SD.open(this->_name + ".meta", FILE_READ);
  if(_file) {
    _file.seek(0);
    int count = _file.parseInt();
    int head = _file.parseInt();
    int size = _file.parseInt();
    int tail = _file.parseInt();
    this->_meta.initialize(size, count, head, tail);
    _file.close();
    Serial.println("Metadata loaded from disk");
  } else {
    Serial.println("Metadata file doesn't exist yet");
  }
}

void SDQueue::flush() {
  File _file = SD.open(this->_name, FILE_WRITE);
  if (!_file) {
    Serial.println("✖ Failed to open file for writing");

  } else {
    Serial.printf("Opened file for flushing buffer. buffer records=%d\n", this->_buffer.getCount());
    bool atLeastOne = false;
    while(!this->_buffer.isEmpty()) {
      atLeastOne = true;
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
    if(atLeastOne) {
      this->_writeMetaFile();
      this->_buffer.empty();
      Serial.println("Buffer flushed to disk");
    }
  }
  _file.close();
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
