#pragma once

#include <vector>
#include "Order.h"

struct Node {
  Order order;
  int next = -1;
  int prev = -1;
};

class OrderPool {
private:
  std::vector<Node> nodes;
  std::vector<int> freeList;

public:
  OrderPool(size_t cap = 500000) {
    nodes.reserve(cap);
    freeList.reserve(cap);
  }

  template<typename... Args>
  int allocate(Args&&... args) {
    if (!freeList.empty()) {
      int index = freeList.back();
      freeList.pop_back();

      new (&nodes[index].order) Order(std::forward<Args>(args)...);
      nodes[index].next = -1;
      nodes[index].prev = -1;
      return index;
    }

    nodes.emplace_back(Node{Order(std::forward<Args>(args)...), -1, -1});
    return static_cast<int>(nodes.size() - 1);
  }

  int currSize() const {
    return (int)nodes.size() - (int)freeList.size();
  }

  void deallocate(int index) {
    freeList.push_back(index);
  }

  Node& get(int index) {
    return nodes[index];
  }

  const Node& get(int index) const {
    return nodes[index];
  }
};