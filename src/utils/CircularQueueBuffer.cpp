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
  int pos = this->_meta.push();
  char* b = this->_buffer + pos*this->_bufferElementSize;
  strncpy(b, (char*)bufferElement, this->_bufferElementSize);
}

void CircularQueueBuffer::poll(const char* bufferElement) {
  const int pos = this->_meta.poll();
  char* element = this->_buffer + pos*this->_bufferElementSize;
  strncpy((char*)bufferElement, element, this->_bufferElementSize);
}

int CircularQueueBuffer::getSize() {
  return this->_meta.getSize();
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
