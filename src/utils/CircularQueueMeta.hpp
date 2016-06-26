
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
      bool initialize(int size, int count, int tail, int head);

    private:
      int _size;
      int _count;
      int _head;
      int _tail;

  };
}
