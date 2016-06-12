
namespace Tracker {
  class CircularQueueMeta {
    public:
      CircularQueueMeta(int size);
      void empty();
      int push();
      int poll();
      int peek();
      int peek(int n);
      void removeElements(int n);
      int getSize();
      int getCount();
      int getHead();
      int getTail();
      bool isFull();
      bool isEmpty();
      void initialize(int size, int count, int head, int tail);

    private:
      int _size;
      int _count;
      int _head;
      int _tail;

  };
}
