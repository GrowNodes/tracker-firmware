#include "CircularQueueBuffer.hpp"
#include <Arduino.h>

namespace Tracker {
  class SDQueue {
    public:
      SDQueue(String name, int maxQueueSize, int recordBytes, int bufferRecords);
      ~SDQueue();
      void setup();
      void push(char* record);
      bool poll(char* record);
      bool peek(char* record);
      bool peek(char* record, int n);
      void removeElements(int n);
      void flush();
      int getSize();
      int getCount();
      bool isFull();
      bool isEmpty();
    private:
      CircularQueueMeta _meta;
      CircularQueueBuffer _buffer;
      String _name;
      String _nameMeta;
      char* _tmprecord;
      int _maxFileSize;
      void _writeMetaFile();
      void _loadMetaFile();
  };
}
