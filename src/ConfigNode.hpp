#include <Homie.h>

namespace Tracker {
  class ConfigNode {
    public:
      ConfigNode();
      void setup();
      void loop();
      String getUploadServerHost();
      int getUploadServerPort();
      String getUploadServerUri();
    private:
      HomieNode _homieNode;
      String _uploadServerHost = "api.stutzthings.com";
      int _uploadServerPort = 80;
      String _uploadServerUri;
  };
}
