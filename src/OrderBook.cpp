#include "OrderBook.h"
#include <iostream>
#include <chrono>

void OrderBook::addOrder(int id, Price price, int quantity, bool isBuy, long long userId, orderType type, std::vector<Trade>& trades) {
  auto now = std::chrono::system_clock::now();
  long long time = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

  int remainingQty = quantity;

  if (type == orderType::FOK) {
    int availableQty = 0;
    bool canFill = false;

    if (isBuy) {
      for (const auto& [askPrice, queue] : asks) {
        if (askPrice > price) break;

        int32_t curr = queue.getHead();
        while (curr != -1) {
          const Node& node = globalPool.get(curr);

          if (node.order.userId != userId) {
            availableQty += node.order.quantity;
            if (availableQty >= quantity) {
              canFill = true;
              goto fok_check_done;
            }
          }
          curr = node.next;
        }
      }
    } else {
      for (const auto& [bidPrice, queue] : bids) {
        if (bidPrice < price) break;

        int32_t curr = queue.getHead();
        while (curr != -1) {
          const Node& node = globalPool.get(curr);

          if (node.order.userId != userId) {
            availableQty += node.order.quantity;
            if (availableQty >= quantity) {
              canFill = true;
              goto fok_check_done;
            }
          }
          curr = node.next;
        }
      }
    }

    fok_check_done:
    if (!canFill) return;
  }

  if (isBuy) {
    auto it = asks.begin();
    while (it != asks.end() && it->first <= price && remainingQty > 0) {
      OrderQueue& queue = it->second;
      int32_t currIdx = queue.getHead();

      while (currIdx != -1 && remainingQty > 0) {
        Node& node = globalPool.get(currIdx);
        Order& bookOrder = node.order;

        if (bookOrder.userId == userId) {
          currIdx = node.next;
          continue;
        }

        int tradeQty = std::min(remainingQty, bookOrder.quantity);

        trades.emplace_back(bookOrder.id, id, it->first, tradeQty, time);

        bookOrder.quantity -= tradeQty;
        remainingQty -= tradeQty;

        int32_t nextIdx = node.next;

        if (bookOrder.quantity == 0) {
          queue.unlink(currIdx);

          idToIndex.erase(bookOrder.id);
          idToPrice.erase(bookOrder.id);
          idToSide.erase(bookOrder.id);

          globalPool.deallocate(currIdx);
        }

        currIdx = nextIdx;
      }

      if (queue.isEmpty()) {
        it = asks.erase(it);
      } else {
        ++it;
      }
    }
  } else {
    auto it = bids.begin();
    while (it != bids.end() && it->first >= price && remainingQty > 0) {
      OrderQueue& queue = it->second;
      int32_t currIdx = queue.getHead();

      while (currIdx != -1 && remainingQty > 0) {
        Node& node = globalPool.get(currIdx);
        Order& bookOrder = node.order;

        if (bookOrder.userId == userId) {
          currIdx = node.next;
          continue;
        }

        int tradeQty = std::min(remainingQty, bookOrder.quantity);

        trades.emplace_back(bookOrder.id, id, it->first, tradeQty, time);

        bookOrder.quantity -= tradeQty;
        remainingQty -= tradeQty;

        int32_t nextIdx = node.next;

        if (bookOrder.quantity == 0) {
          queue.unlink(currIdx);
          idToIndex.erase(bookOrder.id);
          idToPrice.erase(bookOrder.id);
          idToSide.erase(bookOrder.id);
          globalPool.deallocate(currIdx);
        }

        currIdx = nextIdx;
      }

      if (queue.isEmpty()) {
        it = bids.erase(it);
      } else {
        ++it;
      }
    }
  }

  if (remainingQty > 0 && type == orderType::GTC) {
    Order newOrder(id, price, remainingQty, time, userId);

    int32_t idx = globalPool.allocate(newOrder);

    idToIndex[id] = idx;
    idToPrice[id] = price;
    idToSide[id] = isBuy;

    if (isBuy) {
      auto it = bids.find(price);
      if (it == bids.end()) {
        it = bids.emplace(price, globalPool).first;
      }
      it->second.pushBack(idx);
    } else {
      auto it = asks.find(price);
      if (it == asks.end()) {
        it = asks.emplace(price, globalPool).first;
      }
      it->second.pushBack(idx);
    }
  }
}

void OrderBook::cancelOrder(int id) {
  auto it = idToIndex.find(id);
  if (it == idToIndex.end()) return;

  int32_t idx = it->second;
  Price p = idToPrice[id];
  bool isBuy = idToSide[id];

  if (isBuy) {
    auto pIt = bids.find(p);
    if (pIt != bids.end()) {
      pIt->second.unlink(idx);
      if (pIt->second.isEmpty()) bids.erase(pIt);
    }
  } else {
    auto pIt = asks.find(p);
    if (pIt != asks.end()) {
      pIt->second.unlink(idx);
      if (pIt->second.isEmpty()) asks.erase(pIt);
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
  for (const auto& [price, queue] : bids) {
    int32_t curr = queue.getHead();
    while (curr != -1) {
      const Node& node = const_cast<OrderPool&>(globalPool).get(curr);
      std::cout << "ID: " << node.order.id
                << ", Price: " << price.value
                << ", Qty: " << node.order.quantity << "\n";
      curr = node.next;
    }
  }
}