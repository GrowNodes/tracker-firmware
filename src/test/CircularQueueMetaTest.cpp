#include <Arduino.h>
#include "../utils/CircularQueueMeta.hpp"

using namespace Tracker;

void testQueueMeta() {
  Serial.println("Starting circular queue meta tests...");
  CircularQueueMeta m = CircularQueueMeta(3);
  if(!m.isEmpty()) {
    Serial.println("1 Queue should be empty");return;
  }
  if(m.isFull()) {
    Serial.println("2 Queue should not be full");return;
  }
  if(m.getSize()!=3) {
    Serial.println("3 Queue size should be 3");return;
  }
  if(m.getCount()!=0) {
    Serial.println("4c ount should be 0");return;
  }
  if(m.getHead()!=0) {
    Serial.println("4a head should be 0");return;
  }
  if(m.getTail()!=0) {
    Serial.println("4b tail should be 0");return;
  }
  if(m.peek()!=-1) {
    Serial.println("5 peek() should return -1");return;
  }
  if(m.push()!=0) {
    Serial.println("6 push() should return 0");return;
  }
  if(m.getHead()!=1) {
    Serial.println("6a head should be 1");return;
  }
  if(m.getTail()!=0) {
    Serial.println("6b tail should be 0");return;
  }
  if(m.isFull()) {
    Serial.println("7 Queue is should not be full");return;
  }
  if(m.getCount()!=1) {
    Serial.println("8c ount should be 1");return;
  }
  if(m.peek()!=0) {
    Serial.println("9 peek() should return 0");return;
  }
  if(m.peek(1)!=-1) {
    Serial.println("9 peek(0) should return -1");return;
  }
  if(m.peek(100)!=-1) {
    Serial.println("9 peek(100) should return -1");return;
  }
  if(m.getHead()!=1) {
    Serial.println("9a ahead should be 1");return;
  }
  if(m.getTail()!=0) {
    Serial.println("9b tail should be 0");return;
  }

  m.empty();
  m.push();
  m.push();
  if(m.getHead()!=2) {
    Serial.println("9c ahead should be 2");return;
  }
  if(m.getTail()!=0) {
    Serial.println("9d tail should be 0");return;
  }
  if(m.push()!=2) {
    Serial.println("10 push() should return 2");return;
  }
  if(!m.isFull()) {
    Serial.println("10a should be full");return;
  }
  if(m.push()!=0) {
    Serial.println("11 push() should return 0");return;
  }
  if(m.getHead()!=1) {
    Serial.println("11a ahead should be 1");return;
  }
  if(m.getTail()!=1) {
    Serial.println("11d tail should be 1");return;
  }
  if(m.push()!=1) {
    Serial.println("12 push() should return 1");return;
  }
  if(m.getHead()!=2) {
    Serial.println("12a ahead should be 2");return;
  }
  if(m.getTail()!=2) {
    Serial.println("12b tail should be 2");
    Serial.println(m.getTail());
    return;
  }
  if(m.push()!=2) {
    Serial.println("13 push() should return 2");return;
  }
  if(m.push()!=0) {
    Serial.println("14 push() should return 0");return;
  }
  if(m.push()!=1) {
    Serial.println("14a push() should return 1");return;
  }

  if(m.peek(0)!=2) {
    Serial.println("15 peek(0) should return 2");return;
  }
  if(m.peek(1)!=0) {
    Serial.println("16 peek(1) should return 0");return;
  }
  if(m.peek(2)!=1) {
    Serial.println("17 peek(2) should return 1");return;
  }

  if(m.poll()!=2) {
    Serial.println("15a peek(0) should return 2");return;
  }
  if(m.poll()!=0) {
    Serial.println("15b poll() should return 0");return;
  }
  if(m.poll()!=1) {
    Serial.println("15c poll() should return 1");return;
  }

  if(!m.isEmpty()) {
    Serial.println("17a should be empty");return;
  }
  if(m.poll()!=-1) {
    Serial.println("18 poll() should return -1");return;
  }
  if(m.peek(0)!=-1) {
    Serial.println("18a peek(0) should return -1");return;
  }
  if(m.peek(5)!=-1) {
    Serial.println("18b peek(5) should return -1");return;
  }

  m.push();
  m.push();
  m.push();
  if(m.getSize()!=3) {
    Serial.println("20 count should be 3");return;
  }

  m.removeElements(0);
  if(m.getCount()!=3) {
    Serial.println("21 count should be 3");return;
  }

  m.removeElements(1);
  if(m.getCount()!=2) {
    Serial.println("22 count should be 2");return;
  }

  m.removeElements(2);
  if(m.getCount()!=0) {
    Serial.println("23 count should be 0");return;
  }

  m.removeElements(2);
  if(m.getCount()!=0) {
    Serial.println("24 count should be 0");return;
  }

  m.push();
  if(m.getCount()!=1) {
    Serial.println("25 count should be 1");return;
  }

  m.empty();
  if(!m.isEmpty()) {
    Serial.println("26 should be empty");return;
  }

  if(!m.initialize(5, 1, 0, 1)) {
    Serial.println("27a should be valid");return;
  }
  if(m.isEmpty()) {
    Serial.println("27b should not be empty");return;
  }

  if(!m.initialize(5, 5, 0, 0)) {
    Serial.println("28a should be valid");return;
  }
  if(!m.isFull()) {
    Serial.println("28b should be full");return;
  }

  if(!m.initialize(5, 3, 0, 3)) {
    Serial.println("29 should be valid");return;
  }

  if(m.initialize(3, 4, 0, 3)) {
    Serial.println("30 should be invalid");return;
  }
  if(m.initialize(3, 2, 9, 10)) {
    Serial.println("31 should be invalid");return;
  }
  if(m.initialize(30, 20, 9, 30)) {
    Serial.println("32 should be valid");return;
  }
  if(!m.initialize(30, 10, 25, 5)) {
    Serial.println("33 should be valid");return;
  }

  Serial.println("QUEUE META: ALL TESTS OK!");

}
