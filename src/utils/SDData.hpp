#include <Arduino.h>

namespace Tracker {
  class SDData {
    public:
      SDData(String name);
      void setup();
      int getIntValue();
      void setIntValue(int value);
    private:
      String _name;
  };
}
