#include "CircularQueueMeta.hpp"

using namespace Tracker;

CircularQueueMeta::CircularQueueMeta(int size) {
  this->_size = size;
  this->empty();
}

void CircularQueueMeta::empty() {
  this->_tail = 0;
  this->_head = 0;
  this->_count = 0;
}

int CircularQueueMeta::push() {
  int lastHead = this->_head;
  this->_head = (this->_head+1) % this->_size;

  if(this->isFull()) {
    this->_tail = (this->_tail+1) % this->_size;
  } else {
    this->_count++;
  }

  return lastHead;
}

int CircularQueueMeta::poll() {
  if(!this->isEmpty()) {
    int lastTail = this->_tail;
    this->_tail = (this->_tail+1) % this->_size;
    this->_count--;
    return lastTail;
  } else {
    return -1;
  }
}

int CircularQueueMeta::peek() {
  return this->peek(0);
}

int CircularQueueMeta::peek(int n) {
  if(n<this->getCount()) {
    return (this->_tail + n) % this->_size;
  } else {
    return -1;
  }
}

void CircularQueueMeta::removeElements(int n) {
  for(int i=0; i<n; i++) {
    this->poll();
  }
}

bool CircularQueueMeta::initialize(int size, int count, int tail, int head) {
  bool valid = false;
  if(count <= size) {
    if(head == tail) {
      valid = (count==0) || (count==size);
    } else if(head > tail) {
      valid = (count==(head-tail));
    } else if(head < tail) {
      valid = (count==((size-tail+1)+(head-1)));
    }
  }
  if(valid) {
    this->_size = size;
    this->_count = count;
    this->_head = head;
    this->_tail = tail;
    return true;
  } else {
    return false;
  }
}


int CircularQueueMeta::getHead() {
  return this->_head;
}

int CircularQueueMeta::getTail() {
  return this->_tail;
}

int CircularQueueMeta::getSize() {
  return this->_size;
}

int CircularQueueMeta::getCount() {
  return this->_count;
}

bool CircularQueueMeta::isFull() {
  return this->_count == this->_size;
}

bool CircularQueueMeta::isEmpty() {
  return this->_count == 0;
}
