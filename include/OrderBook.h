#pragma once

#include "Order.h"
#include <unordered_map>
#include <iostream>
#include <map>
#include <set>

class OrderBook {
private:
  std::map<double, std::set<Order>, std::greater<double>> bids;
  std::map<double, std::set<Order>> asks;

  std::unordered_map<int, double> orderIdPrice;
  std::unordered_map<int, bool> orderIdSide;

public:
  void addOrder(int id, double price, int quantity, bool isBuy);
  void cancelOrder(int id);
  void printOrderBook() const;
};