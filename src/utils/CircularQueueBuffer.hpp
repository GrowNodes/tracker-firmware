#include "CircularQueueMeta.hpp"

namespace Tracker {
  class CircularQueueBuffer {
    public:
      CircularQueueBuffer(int maxElements, int bufferElementSize);
      ~CircularQueueBuffer();
      void empty();
      void push(const char* bufferElement);
      bool poll(const char* bufferElement);
      bool peek(const char* bufferElement);
      bool peek(const char* bufferElement, int n);
      int getSize();
      int getCount();
      bool isFull();
      bool isEmpty();
      int getBufferElementSize();
      void removeElements(int n);

    private:
      int _bufferElementSize;
      CircularQueueMeta _meta;
      char* _buffer;

  };
}
