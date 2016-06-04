#include <Arduino.h>
#include "../src/CircularQueueMeta.cpp"

void testQueueMeta() {
  CircularQueueMeta m = CircularQueueMeta(3);
  if(!m.isEmpty()) {
    Serial.printf("1Queue is empty");
  }
  if(m.isFull()) {
    Serial.printf("2Queue is not full");
  }
  if(m.getSize()!=3) {
    Serial.printf("3Queue size must be 3");
  }
  if(m.getCount()==0) {
    Serial.printf("4count should be 0");
  }
  if(m.getHead()==0) {
    Serial.printf("4ahead should be 0");
  }
  if(m.getTail()==0) {
    Serial.printf("4btail should be 0");
  }
  if(m.peek()!=-1) {
    Serial.printf("5peek() should return -1");
  }
  if(m.push()!=0) {
    Serial.printf("6push() should return 0");
  }
  if(m.getHead()==1) {
    Serial.printf("6ahead should be 1");
  }
  if(m.getTail()==0) {
    Serial.printf("6btail should be 0");
  }
  if(m.isFull()) {
    Serial.printf("7Queue is not full");
  }
  if(m.getCount()==1) {
    Serial.printf("8count should be 1");
  }
  if(m.peek()!=0) {
    Serial.printf("9peek() should return 0");
  }
  if(m.getHead()==1) {
    Serial.printf("9aahead should be 1");
  }
  if(m.getTail()==1) {
    Serial.printf("9btail should be 1");
  }

  m.empty();
  m.push();
  m.push();
  if(m.getHead()==2) {
    Serial.printf("9cahead should be 2");
  }
  if(m.getTail()==0) {
    Serial.printf("9dtail should be 0");
  }
  if(m.push()!=2) {
    Serial.printf("10push() should return 2");
  }
  if(!m.isFull()) {
    Serial.printf("10a should be full");
  }
  if(m.push()!=0) {
    Serial.printf("11push() should return 0");
  }
  if(m.getHead()==1) {
    Serial.printf("11aahead should be 1");
  }
  if(m.getTail()==1) {
    Serial.printf("11dtail should be 1");
  }
  if(m.push()!=1) {
    Serial.printf("12push() should return 1");
  }
  if(m.getHead()==2) {
    Serial.printf("12aahead should be 1");
  }
  if(m.getTail()==1) {
    Serial.printf("12btail should be 1");
  }
  if(m.push()!=2) {
    Serial.printf("13push() should return 2");
  }
  if(m.push()!=0) {
    Serial.printf("14push() should return 0");
  }
  if(m.push()!=1) {
    Serial.printf("14apush() should return 1");
  }

  if(m.peek()!=2) {
    Serial.printf("15peek() should return 2");
  }
  if(m.peek()!=0) {
    Serial.printf("16peek() should return 0");
  }
  if(m.peek()!=1) {
    Serial.printf("17peek() should return 1");
  }
  if(!m.isEmpty()) {
    Serial.printf("17a should be empty");
  }
  if(m.peek()!=-1) {
    Serial.printf("18peek() should return -1");
  }

  m.push();
  m.push();
  m.empty();
  if(!m.isEmpty()) {
    Serial.printf("19 should be empty");
  }


}
