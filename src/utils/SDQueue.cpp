#include "SDQueue.hpp"
#include <SD.h>

using namespace Tracker;

SDQueue::SDQueue(String name, int maxQueueSize, int recordSize, int bufferSize) :
  _name(name),
  _nameMeta(name + ".m"),
  _meta(maxQueueSize),
  _buffer(bufferSize, recordSize),
  _sdRecordSize(recordSize+1),
  _maxFileSize(maxQueueSize*(recordSize+1)) {
  this->_tmprecord = (char*)malloc(recordSize+1);
}

SDQueue::~SDQueue() {
  free(this->_tmprecord);
}

void SDQueue::setup() {
  if (!SD.begin(4, SPI_HALF_SPEED)) {
    Serial.println("✖ SD initialization error");
  } else {
    Serial.println("SD initialized OK");
  }
  this->_loadMetaFile();
}

void SDQueue::push(char* record) {
  // memset((char*)(record+this->getRecordSize()-1), 0, 1);
  this->_buffer.push(record);
  if(this->_buffer.isFull()) {
    this->flush();
  }
}

void SDQueue::removeElements(int n) {
  this->flush();
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
    int pos = this->_meta.peek(n) * this->_sdRecordSize;
    _file.seek(pos);
    _file.readBytes(record, this->_sdRecordSize);
    //force string termination
    // memset((char*)(record+this->getRecordSize()-1), 0, 1);
    _file.close();
    return true;
  } else {
    return false;
  }
}

void SDQueue::_writeMetaFile() {
  File _file = SD.open(this->_nameMeta, FILE_WRITE);
  if(_file) {
    _file.seek(0);
    _file.print("#"+ String(this->_meta.getSize()));
    _file.print("#"+ String(this->_meta.getCount()));
    _file.print("#"+ String(this->_meta.getTail()));
    _file.print("#"+ String(this->_meta.getHead()));
    _file.print(" ");
    _file.close();
    // Serial.println("Queue metadata saved to disk");
    // Serial.printf("   size: %d", this->_meta.getSize());
    // Serial.printf("   count: %d", this->_meta.getCount());
    // Serial.printf("   tail: %d", this->_meta.getTail());
    // Serial.printf("   head: %d\n", this->_meta.getHead());
  } else {
    Serial.println("Could not open metadata file for writing");
  }
}

void SDQueue::_loadMetaFile() {
  File _file = SD.open(this->_nameMeta, FILE_READ);
  if(_file) {
    _file.seek(0);
    int size = _file.parseInt();
    int count = _file.parseInt();
    int tail = _file.parseInt();
    int head = _file.parseInt();
    if(this->_meta.initialize(size, count, tail, head)) {
      Serial.println("Metadata loaded from disk successfuly");
      // Serial.printf("   size: %d", size);
      // Serial.printf("   count: %d", count);
      // Serial.printf("   tail: %d", tail);
      // Serial.printf("   head: %d\n", head);
    } else {
      Serial.println("Invalid metadata contents from disk");
    }
    _file.close();
  } else {
    Serial.println("Metadata file doesn't exist yet");
  }
}

void SDQueue::flush() {
  if(!this->_buffer.isEmpty()) {
    File _file = SD.open(this->_name, FILE_WRITE);
    if (!_file) {
      Serial.println("✖ Failed to open file for writing");

    } else {
      // Serial.printf("Opened file for flushing buffer. buffer records=%d\n", this->_buffer.getCount());
      bool atLeastOne = false;
      while(!this->_buffer.isEmpty()) {
        // Serial.println("Buffer element flush");
        atLeastOne = true;
        this->_buffer.poll(this->_tmprecord);
        int pos = this->_meta.push() * this->_sdRecordSize;
        // Serial.printf("File size=%d; pos=%d\n", _file.size(), pos);
        //expand file size as needed
        if(_file.size()<pos) {
          // Serial.println("Expanding queue file");
          _file.seek(_file.size());
          int expectedSize = min(_maxFileSize, pos + (this->_sdRecordSize*1000)) - 1;
          while(_file.size() < expectedSize) {
            _file.write((byte)0);
          }
        } else {
          // Serial.println("Seeking inside file");
          _file.seek(pos);
        }
        // _file.write("zoaaaaandooo");//zoando arquivo
        _file.write(this->_tmprecord, this->_buffer.getBufferElementSize());
        _file.print("\n");//record separator
      }
      _file.close();
      if(atLeastOne) {
        this->_writeMetaFile();
        this->_buffer.empty();
        // Serial.println("Buffer flushed to disk");
      }
    }
  } else {
    // Serial.println("Buffer is empty. Skipping flush()");
  }
}

int SDQueue::getRecordSize() {
  return this->_buffer.getBufferElementSize();
}

int SDQueue::getBufferSize() {
  return this->_buffer.getSize();
}

int SDQueue::getSize() {
  return this->_meta.getSize();
}

int SDQueue::getCount() {
  int c = this->_meta.getCount() + this->_buffer.getCount();
  if(c>this->_meta.getSize()) {
    return this->_meta.getSize();
  } else {
    return c;
  }
}

bool SDQueue::isFull() {
  this->flush();
  return this->_meta.isFull();
}

bool SDQueue::isEmpty() {
  return this->_meta.isEmpty();
}
