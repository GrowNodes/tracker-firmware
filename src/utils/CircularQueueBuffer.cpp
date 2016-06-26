#include <Arduino.h>
#include "CircularQueueBuffer.hpp"

using namespace Tracker;

CircularQueueBuffer::CircularQueueBuffer(int maxElements, int bufferElementSize) :
  _meta(maxElements) {
  this->_bufferElementSize = bufferElementSize;
  this->empty();
  this->_buffer = (char*)malloc(bufferElementSize*maxElements);
}

CircularQueueBuffer::~CircularQueueBuffer() {
  free(this->_buffer);
}

void CircularQueueBuffer::empty() {
  this->_meta.empty();
}

void CircularQueueBuffer::push(const char* bufferElement) {
  // memset((char*)(bufferElement+this->_bufferElementSize-1), 0, 1);
  int pos = this->_meta.push();
  char* b = this->_buffer + pos*this->_bufferElementSize;
  strncpy(b, (char*)bufferElement, this->_bufferElementSize);
}

bool CircularQueueBuffer::poll(const char* bufferElement) {
  const int pos = this->_meta.poll();
  if(pos!=-1) {
    char* element = this->_buffer + (pos*this->_bufferElementSize);
    strncpy((char*)bufferElement, element, this->_bufferElementSize);
    return true;
  } else {
    return false;
  }
}

bool CircularQueueBuffer::peek(const char* bufferElement) {
  return this->peek(bufferElement, 0);
}

bool CircularQueueBuffer::peek(const char* bufferElement, int n) {
  const int pos = this->_meta.peek(n);
  if(pos!=-1) {
    char* element = this->_buffer + (pos*this->_bufferElementSize);
    strncpy((char*)bufferElement, element, this->_bufferElementSize);
    return true;
  } else {
    return false;
  }
}

void CircularQueueBuffer::removeElements(int n) {
  this->_meta.removeElements(n);
}

int CircularQueueBuffer::getSize() {
  return this->_meta.getSize();
}

int CircularQueueBuffer::getCount() {
  return this->_meta.getCount();
}

int CircularQueueBuffer::getBufferElementSize() {
  return this->_bufferElementSize;
}

bool CircularQueueBuffer::isFull() {
  return this->_meta.isFull();
}

bool CircularQueueBuffer::isEmpty() {
  return this->_meta.isEmpty();
}
