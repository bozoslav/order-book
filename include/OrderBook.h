#pragma once

#include "Order.h"
#include "Trade.h"
#include "Price.h"
#include "OrderPool.h"
#include "OrderQueue.h"
#include <unordered_map>
#include <iostream>
#include <vector>
#include <map>
#include <set>

enum struct orderType {
  GTC,
  IOC,
  FOK
};

class OrderBook {
private:
  OrderPool globalPool;

  std::map<Price, OrderQueue, std::greater<Price>> bids;
  std::map<Price, OrderQueue> asks;

  std::unordered_map<int, Price> idToPrice;
  std::unordered_map<int, bool> idToSide;
  std::unordered_map<int, int> idToIndex;

public:
  OrderBook() = default;

  void addOrder(int id, Price price, int quantity, bool isBuy, long long userId, orderType type, std::vector<Trade>& trades);
  void modifyOrder(int id, Price newPrice, int newQuantity, std::vector<Trade>& trades);
  void cancelOrder(int id);
  void printOrderBook() const;
};