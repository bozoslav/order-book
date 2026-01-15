#pragma once
#include <array>
#include "OrderQueue.h"
#include "OrderPool.h"

template<int N>
class PriceLevelArray {
private:
  static constexpr int CHUNK = 64;
  static constexpr int B = (N + CHUNK - 1) / CHUNK;

  std::array<OrderQueue, N> levels;
  std::array<unsigned long long, B> bitmap{};
  std::array<long long, N> totals{};
  OrderPool *pool = nullptr;

  int bitIndexToLevel(int chunk, int bit) const { return int(chunk * CHUNK + bit); }

  void setBit(int idx) { 
    bitmap[idx / CHUNK] |= (unsigned long long(1) << (idx % CHUNK)); 
  }
  void clearBit(int idx) { 
    bitmap[idx / CHUNK] &= ~(unsigned long long(1) << (idx % CHUNK)); 
  }

public:
  void addQtyAt(int idx, long long delta) {
    totals[idx] += delta;

    if (totals[idx] > 0) setBit(idx);
    else if (totals[idx] <= 0) {
      totals[idx] = 0;
      clearBit(idx);
    }
  }
  long long totalAt(int idx) const { return totals[idx]; }

public:
  void setPool(OrderPool &p) { pool = &p; for (int i = 0; i < N; ++i) levels[i].setPool(p); }

  bool isEmptyLevel(int idx) const { return levels[idx].isEmpty(); }

  void pushBackAt(int idx, int nodeIndex) {
    bool wasEmpty = levels[idx].isEmpty();
    levels[idx].pushBack(nodeIndex);

    if (pool) {
      const Node &n = pool->get(nodeIndex);
      addQtyAt(idx, n.order.quantity);
    } else {
      if (wasEmpty) setBit(idx);
    }
  }

  void unlinkAt(int idx, int nodeIndex) {
    levels[idx].unlink(nodeIndex);
    if (levels[idx].isEmpty()) clearBit(idx);
  }

  OrderQueue &level(int idx) { return levels[idx]; }
  const OrderQueue &level(int idx) const { return levels[idx]; }

  int findNextNonEmptyFrom(int start) const {
    if (start >= N) return -1;
    int chunk = start / CHUNK;
    int offset = int(start % CHUNK);

    unsigned long long w = bitmap[chunk];
    w &= (~unsigned long long(0)) << offset;

    if (w) { return bitIndexToLevel(chunk, __builtin_ctzll(w)); }

    for (int c = chunk + 1; c < B; ++c) {
      if (bitmap[c]) return bitIndexToLevel(c, __builtin_ctzll(bitmap[c]));
    }
    return -1;
  }

  int findPrevNonEmptyFrom(int start) const {
    if (start >= N) start = N - 1;
    int chunk = start / CHUNK;
    int offset = int(start % CHUNK);

    unsigned long long w = bitmap[chunk];

    if (offset < 63) {
      w &= ((unsigned long long(1) << (offset + 1)) - 1);
    }

    if (w) { return bitIndexToLevel(chunk, 63 - __builtin_clzll(w)); }

    for (int c = (chunk == 0 ? 0 : chunk - 1); ; --c) {
      if (bitmap[c]) return bitIndexToLevel(c, 63 - __builtin_clzll(bitmap[c]));
      if (c == 0) break;
    }

    return -1;
  }
};