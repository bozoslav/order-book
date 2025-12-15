#pragma once

#include "Order.h"
#include "Trade.h"
#include "Price.h"
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
  std::map<Price, std::set<Order>, std::greater<Price>> bids;
  std::map<Price, std::set<Order>> asks;

  std::unordered_map<int, long long> orderIdUserId;
  std::unordered_map<int, Price> orderIdPrice;
  std::unordered_map<int, bool> orderIdSide;

public:
  void addOrder(int id, Price price, int quantity, bool isBuy, long long userId, orderType type, std::vector<Trade>& trades);
  void modifyOrder(int id, Price newPrice, int newQuantity, std::vector<Trade>& trades);
  void printOrderBook() const;
  void cancelOrder(int id);
};