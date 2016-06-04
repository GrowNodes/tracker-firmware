#include <Arduino.h>
#include "../src/CircularQueueBuffer.cpp"

void testQueueBuffer() {
  CircularQueueBuffer m = CircularQueueBuffer(3, 50);
  char buffer[50];
  char* b = &buffer[0];
  m.push("test0");
  m.push("test1");
  m.push("test2");

  if(!m.isFull()) {
    Serial.printf("1 buffer should be full");
  }

  m.peek(b);
  if(String(b)!="test0") {
    Serial.printf("1 element should be test0");
  }
  if(m.isFull()) {
    Serial.printf("1 buffer should not be full");
  }

  m.peek(b);
  if(String(b)!="test1") {
    Serial.printf("2 element should be test1");
  }

  m.push("test3");

  m.peek(b);
  if(String(b)!="test2") {
    Serial.printf("3 element should be test2");
  }

  m.peek(b);
  if(String(b)!="test3") {
    Serial.printf("4 element should be test3");
  }

  if(!m.isEmpty()) {
    Serial.printf("5 buffer should be empty");
  }

}
