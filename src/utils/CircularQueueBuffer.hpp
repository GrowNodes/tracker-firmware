#include "CircularQueueMeta.hpp"

namespace Tracker {
  class CircularQueueBuffer {
    public:
      CircularQueueBuffer(int maxElements, int bufferElementSize);
      ~CircularQueueBuffer();
      void empty();
      void push(const char* bufferElement);
      void poll(const char* bufferElement);
      int getSize();
      int getCount();
      bool isFull();
      bool isEmpty();
      int getBufferElementSize();

    private:
      int _bufferElementSize;
      CircularQueueMeta _meta;
      char* _buffer;

  };
}
