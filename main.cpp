#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <map>
#include <set>

struct Order {
  int id;
  double price;
  int quantity;
  long long timestamp;

  bool operator<(const Order& other) const {
    if(timestamp != other.timestamp) return timestamp < other.timestamp;
    return id < other.id;
  }
};

class OrderBook {
private:
  std::map<double, std::set<Order>, std::greater<double>> bids;
  std::map<double, std::set<Order>> asks;

  std::unordered_map<int, double> orderIdPrice;
  std::unordered_map<int, bool> orderIdSide;

public:
  void addOrder(int id, double price, int quantity, bool isBuy) {
    auto now = std::chrono::system_clock::now();
    long long time = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
  
    Order newOrder = { id, price, quantity, time };
    
    orderIdPrice[id] = price;
    orderIdSide[id] = isBuy;

    if(isBuy) bids[price].insert(newOrder);
    else asks[price].insert(newOrder);
  }

  void cancelOrder(int id) {
    auto side_it = orderIdSide.find(id);
    if(side_it == orderIdSide.end()) return;

    bool isBuy = side_it->second;
    double price = orderIdPrice[id];

    Order keyOrder = { id, price, 0, 0 }; 

    if(isBuy) {
      auto priceLevelIt = bids.find(price);
      
      if(priceLevelIt != bids.end()) {
        priceLevelIt->second.erase(keyOrder); 

        if(priceLevelIt->second.empty()) bids.erase(priceLevelIt);
      }
    } else {
      auto priceLevelIt = asks.find(price);

      if(priceLevelIt != asks.end()) {
        priceLevelIt->second.erase(keyOrder);

        if(priceLevelIt->second.empty()) asks.erase(priceLevelIt);
      }
    }

    orderIdSide.erase(side_it);
    orderIdPrice.erase(id);
  }
};

int main() {
  OrderBook book;
  
  // TESTING
  std::random_device rd;
  std::mt19937 gen(rd());
  
  double currentPrice = 100.00;
  
  std::uniform_real_distribution<> priceMove(-0.50, 0.50);
  std::uniform_int_distribution<> qtyGen(1, 100);
  std::uniform_int_distribution<> sideGen(0, 1);
  std::uniform_int_distribution<> idGen(1, 1000);

  for(int i = 1; i <= 100000; ++i) {
    if(idGen(gen) > 100) {
      currentPrice += priceMove(gen);
      currentPrice = std::round(currentPrice * 100.0) / 100.0;

      int qty = qtyGen(gen);
      bool isBuy = sideGen(gen) == 0;

      book.addOrder(i, currentPrice, qty, isBuy);
      
    } else {
      int cancelID = idGen(gen) % i;
      if (cancelID > 0) {
        book.cancelOrder(cancelID);
      }
    }
  }

  return 0;
}