// #include <Homie.h>
#include "SDQueue.hpp"

using namespace Tracker;

SDQueue::SDQueue(String name, int queueSize, int recordLength, int bufferSize) :
  _name(name),
  _meta(queueSize),
  _buffer(bufferSize, recordLength) {
}

void SDQueue::setup() {
  if (!SD.begin()) {
    Serial.println("✖ SD initialization error");
  }
}

void SDQueue::push(char* record) {
  this->_buffer.push(record);
  if(this->_buffer.isFull()) {
    this->flush();
  }
}

void SDQueue::poll(char* record) {
  this->flush();
  this->_file = SD.open(this->_name, FILE_READ);
  this->_file.close();
}

void SDQueue::flush() {
  this->_file = SD.open(this->_name, FILE_WRITE);
  if (!this->_file) {
    Serial.println("✖ Failed to open file for writing");

  } else {
    Serial.println("Opened file for flushing buffer");
    while(!this->_buffer.isEmpty()) {
      this->_buffer.poll(&record);
      int pos = this->_meta.push() * this->_buffer.getBufferElementSize();
      //expand file size as needed
      while(this->_file.size()<pos) {
        this->_file.seek(this->_file.size());
        this->_file.write((byte)0);
      }
      this->_file.seek(pos);
      this->_file.write(&record, this->_buffer.getBufferElementSize());
    }
    this->_file.close();
    this->_buffer.empty();
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
