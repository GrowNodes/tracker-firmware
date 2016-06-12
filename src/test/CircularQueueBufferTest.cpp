#include <Arduino.h>
#include "../utils/CircularQueueBuffer.hpp"

using namespace Tracker;

void testQueueBuffer() {
  CircularQueueBuffer m = CircularQueueBuffer(3, 50);
  char buffer[50];
  char* b = &buffer[0];
  m.push("test0");
  m.push("test1");
  m.push("test2");

  if(!m.isFull()) {
    Serial.printf("1 buffer should be full");return;
  }

  m.poll(b);
  if(String(b)!="test0") {
    Serial.printf("1 element should be test0");return;
  }
  if(m.isFull()) {
    Serial.printf("1 buffer should not be full");return;
  }

  m.poll(b);
  if(String(b)!="test1") {
    Serial.printf("2 element should be test1");return;
  }

  m.push("test3");

  if(!m.poll(b) || String(b)!="test2") {
    Serial.printf("3 element should be test2");return;
  }

  if(!m.poll(b) || String(b)!="test3") {
    Serial.printf("4 element should be test3");return;
  }

  if(!m.isEmpty()) {
    Serial.printf("5 buffer should be empty");return;
  }

  if(m.poll(b)) {
    Serial.printf("6 poll() should return false");return;
  }
  if(m.peek(b)) {
    Serial.printf("7 peek() should return false");return;
  }

  m.push("test4");
  m.push("test5");
  if(!m.peek(b) || String(b) != "test4") {
    Serial.printf("8 peek() should return test4");return;
  }
  if(!m.peek(b,0) || String(b) != "test4") {
    Serial.printf("8b peek() should return test4");return;
  }
  if(!m.peek(b,1) || String(b) != "test5") {
    Serial.printf("9 peek() should return test5");return;
  }
  if(m.peek(b,2)) {
    Serial.printf("10 peek() should return false");return;
  }
  if(m.peek(b,3)) {
    Serial.printf("11 peek() should return false");return;
  }
  if(m.peek(b,555)) {
    Serial.printf("12 peek() should return false");return;
  }

  m.push("test6");
  if(!m.peek(b,2) || String(b) != "test6") {
    Serial.printf("13 peek() should return test6");return;
  }

  Serial.println("QUEUE BUFFER: ALL TESTS OK!");

}
