#pragma once

#include "OrderPool.h"

class OrderQueue {
private:
  int head = -1;
  int tail = -1;
  OrderPool &pool;

public:
  OrderQueue(OrderPool &globalPool) : pool(globalPool) {}

  OrderQueue() = delete;

  bool isEmpty() const { return head == -1; }
  int getHead() const { return head; }

  void pushBack(int index) {
    Node &node = pool.get(index);

    if (tail == -1) {
      head = index;
      tail = index;
      node.prev = -1;
      node.next = -1;
    } else {
      Node &tailNode = pool.get(tail);
      tailNode.next = index;
      node.prev = tail;
      node.next = -1;
      tail = index;
    }
  }

  void unlink(int32_t index) {
    Node &node = pool.get(index);
        
    if (node.prev != -1) {
      pool.get(node.prev).next = node.next;
    } else {
      head = node.next;
    }

    if (node.next != -1) {
      pool.get(node.next).prev = node.prev;
    } else {
      tail = node.prev;
    }
        
    node.prev = -1;
    node.next = -1;
  }
};