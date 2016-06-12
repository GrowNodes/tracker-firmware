#include "ConfigNode.hpp"

using namespace Tracker;

//String name, int maxQueueSize, int recordBytes, int bufferRecords
ConfigNode::ConfigNode() :
  _homieNode(HomieNode("$config", "$config")),
  _uploadServerHost("api.stutzthings.com"),
  _uploadServerPort(80),
  _uploadServerUri(Homie.getBaseTopic() + String("gps/data")) {
}

void ConfigNode::setup() {
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

  // this->_uploadServerUri = Homie.getBaseTopic();
  this->_uploadServerUri = "test";
}

void ConfigNode::loop() {
}

String ConfigNode::getUploadServerHost() {
  return this->_uploadServerHost;
}

int ConfigNode::getUploadServerPort() {
  return this->_uploadServerPort;
}

String ConfigNode::getUploadServerUri() {
  return this->_uploadServerUri;
}
