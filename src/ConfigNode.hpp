#include <Homie.h>

namespace Tracker {
  class ConfigNode {
    public:
      ConfigNode();
      void setup();
      void loop();
      // void setBootCount(int bootCount);
    private:
      HomieNode _homieNode;
      // int _bootCount = -1;
  };
}
