#include "OrderBook.h"
#include <iostream>
#include <chrono>

OrderBook::OrderBook() {
  bids.setPool(globalPool);
  asks.setPool(globalPool);
  bestAskIndex = -1;
  bestBidIndex = -1;
}

int OrderBook::priceToIndex(const Price &p) const {
  return static_cast<int>(p.value - PRICE_OFFSET);
}

void OrderBook::addOrder(int id, Price price, int quantity, bool isBuy, long long userId, orderType type, std::vector<Trade>& trades) {
  auto now = std::chrono::system_clock::now();
  long long time = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

  int remainingQty = quantity;

  if (type == orderType::FOK) {
    int availableQty = 0;
    bool canFill = false;

    if (isBuy) {
      int maxIdx = priceToIndex(price);
      if (maxIdx >= 0) {
        if (maxIdx >= MAX_LEVELS) maxIdx = MAX_LEVELS - 1;
        int startLevel = (bestAskIndex != -1) ? bestAskIndex : asks.findNextNonEmptyFrom(0);
        int level = startLevel;
        while (level != -1 && level <= maxIdx) {
          OrderQueue &queue = asks.level(level);
          int curr = queue.getHead();
          while (curr != -1) {
            const Node &node = globalPool.get(curr);
            if (node.order.userId != userId) {
              availableQty += node.order.quantity;
              if (availableQty >= quantity) { canFill = true; goto fok_check_done; }
            }
            curr = node.next;
          }
          level = asks.findNextNonEmptyFrom(level + 1);
        }
      }
    } else {
      int minIdx = priceToIndex(price);
      if (minIdx < MAX_LEVELS) {
        if (minIdx < 0) minIdx = 0;
        int startLevel = (bestBidIndex != -1) ? bestBidIndex : bids.findPrevNonEmptyFrom(MAX_LEVELS - 1);
        int level = startLevel;
        while (level != -1 && level >= minIdx) {
          OrderQueue &queue = bids.level(level);
          int curr = queue.getHead();
          while (curr != -1) {
            const Node &node = globalPool.get(curr);
            if (node.order.userId != userId) {
              availableQty += node.order.quantity;
              if (availableQty >= quantity) { canFill = true; goto fok_check_done; }
            }
            curr = node.next;
          }
          if (level == 0) break;
          level = bids.findPrevNonEmptyFrom(level - 1);
        }
      }
    }

    fok_check_done:
    if (!canFill) return;
  }

  if (isBuy) {
      int maxIdx = priceToIndex(price);
      if (maxIdx >= 0) {
        if (maxIdx >= MAX_LEVELS) maxIdx = MAX_LEVELS - 1;
        int startLevel = (bestAskIndex != -1) ? bestAskIndex : asks.findNextNonEmptyFrom(0);
        int level = startLevel;
        while (level != -1 && level <= maxIdx && remainingQty > 0) {
          OrderQueue &queue = asks.level(level);
          int currIdx = queue.getHead();

          while (currIdx != -1 && remainingQty > 0) {
            Node &node = globalPool.get(currIdx);
            Order &bookOrder = node.order;

            if (bookOrder.userId == userId) { currIdx = node.next; continue; }

            int tradeQty = std::min(remainingQty, bookOrder.quantity);

            Price tradePrice(static_cast<long long>(level + PRICE_OFFSET));
            trades.emplace_back(bookOrder.id, id, tradePrice, tradeQty, time);

            // adjust per-level total
            asks.addQtyAt(level, -tradeQty);

            bookOrder.quantity -= tradeQty;
            remainingQty -= tradeQty;

            int nextIdx = node.next;

            if (bookOrder.quantity == 0) {
              asks.unlinkAt(level, currIdx);
              idToIndex.erase(bookOrder.id);
              idToPrice.erase(bookOrder.id);
              idToSide.erase(bookOrder.id);
              globalPool.deallocate(currIdx);
              // update bestAskIndex if this level emptied
              if (asks.isEmptyLevel(level) && bestAskIndex == level) {
                int nxt = asks.findNextNonEmptyFrom(level + 1);
                bestAskIndex = nxt;
              }
            }

            currIdx = nextIdx;
          }

          level = asks.findNextNonEmptyFrom(level + 1);
          }
        }
      } else {
      int minIdx = priceToIndex(price);
      if (minIdx < MAX_LEVELS) {
      if (minIdx < 0) minIdx = 0;
      int startLevel = (bestBidIndex != -1) ? bestBidIndex : bids.findPrevNonEmptyFrom(MAX_LEVELS - 1);
      int level = startLevel;
      while (level != -1 && level >= minIdx && remainingQty > 0) {
      OrderQueue &queue = bids.level(level);
      int currIdx = queue.getHead();

      while (currIdx != -1 && remainingQty > 0) {
        Node &node = globalPool.get(currIdx);
        Order &bookOrder = node.order;

        if (bookOrder.userId == userId) { currIdx = node.next; continue; }

        int tradeQty = std::min(remainingQty, bookOrder.quantity);

        Price tradePrice(static_cast<long long>(level + PRICE_OFFSET));
        trades.emplace_back(bookOrder.id, id, tradePrice, tradeQty, time);

        // adjust per-level total
        bids.addQtyAt(level, -tradeQty);

        bookOrder.quantity -= tradeQty;
        remainingQty -= tradeQty;

        int nextIdx = node.next;

        if (bookOrder.quantity == 0) {
          bids.unlinkAt(level, currIdx);
          idToIndex.erase(bookOrder.id);
          idToPrice.erase(bookOrder.id);
          idToSide.erase(bookOrder.id);
          globalPool.deallocate(currIdx);
          if (bids.isEmptyLevel(level) && bestBidIndex == level) {
            int prev = (level == 0) ? -1 : bids.findPrevNonEmptyFrom(level - 1);
            bestBidIndex = prev;
          }
        }

        currIdx = nextIdx;
      }

      if (level == 0) break;
      level = bids.findPrevNonEmptyFrom(level - 1);
      }
    }
  }

  if (remainingQty > 0 && type == orderType::GTC) {
    Order newOrder(id, price, remainingQty, time, userId);

    int idx = globalPool.allocate(newOrder);

    idToIndex[id] = idx;
    idToPrice[id] = price;
    idToSide[id] = isBuy;

    int pIdx = priceToIndex(price);
    if (pIdx < 0) pIdx = 0;
    if (pIdx >= MAX_LEVELS) pIdx = MAX_LEVELS - 1;
    if (isBuy) {
      bool wasEmpty = bids.isEmptyLevel(pIdx);
      bids.pushBackAt(pIdx, idx);
      if (wasEmpty) {
        if (bestBidIndex == -1 || pIdx > bestBidIndex) bestBidIndex = pIdx;
      }
    } else {
      bool wasEmpty = asks.isEmptyLevel(pIdx);
      asks.pushBackAt(pIdx, idx);
      if (wasEmpty) {
        if (bestAskIndex == -1 || pIdx < bestAskIndex) bestAskIndex = pIdx;
      }
    }
  }
}

void OrderBook::cancelOrder(int id) {
  auto it = idToIndex.find(id);
  if (it == idToIndex.end()) return;

  int idx = it->second;
  Price p = idToPrice[id];
  bool isBuy = idToSide[id];
  int pIdx = priceToIndex(p);
  if (pIdx < 0) pIdx = 0;
  if (pIdx >= MAX_LEVELS) pIdx = MAX_LEVELS - 1;
  if (isBuy) {
    // subtract remaining quantity from totals
    {
      const Node &n = globalPool.get(idx);
      bids.addQtyAt(pIdx, -static_cast<long long>(n.order.quantity));
    }
    bids.unlinkAt(pIdx, idx);
    if (bids.isEmptyLevel(pIdx) && bestBidIndex == pIdx) {
      int prev = (pIdx == 0) ? -1 : bids.findPrevNonEmptyFrom(pIdx - 1);
      bestBidIndex = prev;
    }
  } else {
    // subtract remaining quantity from totals
    {
      const Node &n = globalPool.get(idx);
      asks.addQtyAt(pIdx, -static_cast<long long>(n.order.quantity));
    }
    asks.unlinkAt(pIdx, idx);
    if (asks.isEmptyLevel(pIdx) && bestAskIndex == pIdx) {
      int nxt = asks.findNextNonEmptyFrom(pIdx + 1);
      bestAskIndex = nxt;
    }
  }

  globalPool.deallocate(idx);

  idToIndex.erase(it);
  idToPrice.erase(id);
  idToSide.erase(id);
}

void OrderBook::modifyOrder(int id, Price newPrice, int newQuantity, std::vector<Trade>& trades) {
  if (idToIndex.find(id) == idToIndex.end()) return;

  bool isBuy = idToSide[id];
  long long userId = globalPool.get(idToIndex[id]).order.userId;

  cancelOrder(id);
  addOrder(id, newPrice, newQuantity, isBuy, userId, orderType::GTC, trades);
}

void OrderBook::printOrderBook() const {
  std::cout << "\nBIDS (price desc):\n";
  int level = (bestBidIndex != -1) ? bestBidIndex : bids.findPrevNonEmptyFrom(MAX_LEVELS - 1);
  while (level != -1) {
    const OrderQueue &queue = bids.level(level);
    int curr = queue.getHead();
    while (curr != -1) {
      const Node &node = const_cast<OrderPool &>(globalPool).get(curr);
      Price p(static_cast<long long>(level + PRICE_OFFSET));
      std::cout << "ID: " << node.order.id
                << ", Price: " << p.value
                << ", Qty: " << node.order.quantity << "\n";
      curr = node.next;
    }
    if (level == 0) break;
    level = bids.findPrevNonEmptyFrom(level - 1);
  }
}

int OrderBook::numOrders() const {
  return globalPool.currSize();
}