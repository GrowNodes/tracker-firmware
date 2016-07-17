#include <Arduino.h>
#include "CircularQueueBuffer.hpp"

namespace Tracker {
  class SDQueue {
    public:
      SDQueue(String name, int maxQueueSize, int recordSize, int bufferSize);
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
      int getRecordSize();
      int getBufferSize();
    private:
      CircularQueueMeta _meta;
      CircularQueueBuffer _buffer;
      String _name;
      String _nameMeta;
      char* _tmprecord;
      int _sdRecordSize;
      int _maxFileSize;
      void _writeMetaFile();
      void _loadMetaFile();
  };
}
