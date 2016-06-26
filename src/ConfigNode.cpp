#include "ConfigNode.hpp"

using namespace Tracker;

//String name, int maxQueueSize, int recordBytes, int bufferRecords
ConfigNode::ConfigNode() :
  _homieNode(HomieNode("$config", "$config")) {
}

void ConfigNode::setup() {
  // this->_homieNode = HomieNode("$config", "$config");
  HomieNode h = this->_homieNode;
  h.subscribe("factoryReset", [h](String value) {
    if(value == "true") {
      Serial.println("Factory reset requested");
      Homie.setNodeProperty(h, "factoryReset", "false");
      Homie.eraseConfig();
      ESP.restart();
    }
    return true;
  });
  // Serial.println("CONFIG NODE SETUP()");
  // Serial.println(this->_uploadServerUri);
}

void ConfigNode::loop() {
  // Homie.setNodeProperty(this->_homieNode, "bootCount", String(this->_bootCount));
}

// void ConfigNode::setBootCount(int bootCount) {
//   this->_bootCount = bootCount;
// }
