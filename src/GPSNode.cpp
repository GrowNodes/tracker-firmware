#include <ESP8266WiFi.h>
#include <Homie.hpp>
#include "GPSNode.hpp"

using namespace Tracker;

GPSNode::GPSNode() {
}

void GPSNode::setup() {
}

void GPSNode::loop() {
  if(WiFi.status()==WL_CONNECTED) {
    //Homie.setNodeProperty(gpsNode, "position", "true");
  }
}
