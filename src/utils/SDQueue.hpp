#include "CircularQueueMeta.hpp"
#include "CircularQueueBuffer.hpp"
#include <SPI.h>
#include <SD.h>

namespace Tracker {
  class SDQueue {
    public:
      SDQueue(String name, int queueSize, int recordSize, int bufferSize);
      ~SDQueue();
      void setup();
      void push(char* record);
      void poll(char* record);
      void flush();
      int getSize();
      int getCount();
      bool isFull();
      bool isEmpty();
    private:
      CircularQueueMeta _meta;
      CircularQueueBuffer _buffer;
      String _name;
      File _file;
      char record;
  };
}
