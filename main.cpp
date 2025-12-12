#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <map>
#include <set>
#include <unordered_map>

struct Order {
  int id;
  double price;
  int quantity;
  long long timestamp;

  Order(int id_, double price_, int quantity_, long long timestamp_) : id(id_), price(price_), quantity(quantity_), timestamp(timestamp_) {}

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
  void printOrderBook() const {
    std::cout << "\nBIDS (price desc):\n";
    for(const auto &[price, orders] : bids) {
      for(const auto &order : orders) {
        std::cout << "ID: " << order.id << ", Price: " << price << ", Qty: " << order.quantity << ", Time: " << order.timestamp << '\n';
      }
    }
    std::cout << "\nASKS (price asc):\n";
    for(const auto &[price, orders] : asks) {
      for(const auto &order : orders) {
        std::cout << "ID: " << order.id << ", Price: " << price << ", Qty: " << order.quantity << ", Time: " << order.timestamp << '\n';
      }
    }
  }

  void addOrder(int id, double price, int quantity, bool isBuy) {
    auto now = std::chrono::system_clock::now();
    long long time = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    orderIdPrice[id] = price;
    orderIdSide[id] = isBuy;

    if(isBuy) {
      auto it = asks.begin();
      while(it != asks.end() && it->first <= price && quantity > 0) {
        auto itSet = it->second.begin();
        
        while(itSet != it->second.end() && quantity > 0) {
          int curr = itSet->quantity;

          if(quantity >= curr) {
            quantity -= curr;
            itSet = it->second.erase(itSet);
          } else {
            Order updated = *itSet;
            updated.quantity -= quantity;

            quantity = 0;

            itSet = it->second.erase(itSet);
            it->second.insert(updated);
            break;
          }
        }

        ++it;
      }

      if(quantity > 0) bids[price].emplace(id, price, quantity, time);
    } else { 
      auto it = bids.begin();
      while(it != bids.end() && it->first >= price && quantity > 0) {
        auto itSet = it->second.begin();

        while(itSet != it->second.end() && quantity > 0) { 
          int curr = itSet->quantity;

          if(quantity >= curr) {
            quantity -= curr;
            itSet = it->second.erase(itSet);
          } else {
            Order updated = *itSet;
            updated.quantity -= quantity;

            quantity = 0;

            itSet = it->second.erase(itSet);
            it->second.insert(updated);
            break;
          }
        }

        ++it;
      }

      if(quantity > 0) asks[price].emplace(id, price, quantity, time);
    }
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