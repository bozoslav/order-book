#pragma once

#include "Order.h"
#include "Trade.h"
#include "Price.h"
#include "OrderPool.h"
#include "OrderQueue.h"
#include "PriceArray.h"
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

  static constexpr int PRICE_OFFSET = 10000;
  static constexpr int MAX_LEVELS = 1001; // prices [100, 110]

  PriceLevelArray<MAX_LEVELS> bids;
  PriceLevelArray<MAX_LEVELS> asks;

  int bestAskIndex = -1;
  int bestBidIndex = -1;

  int priceToIndex(const Price &p) const;
  
  std::unordered_map<int, Price> idToPrice;
  std::unordered_map<int, bool> idToSide;
  std::unordered_map<int, int> idToIndex;

public:
  OrderBook();

  void addOrder(int id, Price price, int quantity, bool isBuy, long long userId, orderType type, std::vector<Trade>& trades);
  void modifyOrder(int id, Price newPrice, int newQuantity, std::vector<Trade>& trades);
  void cancelOrder(int id);
  void printOrderBook() const;
  int numOrders() const;
};