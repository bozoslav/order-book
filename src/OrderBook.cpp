#include "OrderBook.h"
#include <iostream>
#include <chrono>

void OrderBook::addOrder(int id, double price, int quantity, bool isBuy, long long userId, orderType type, std::vector<Trade>& trades) {
  auto now = std::chrono::system_clock::now();
  long long time = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

  if(type == orderType::FOK) {
    int availableQty = 0;
    bool canFill = false;

    if (isBuy) {
      auto it = asks.begin();

      while (it != asks.end() && it->first <= price) {
        for (const auto& order : it->second) {
          if (order.userId == userId) continue;

          availableQty += order.quantity;
          if (availableQty >= quantity) {
            canFill = true;
            break;
          }
        }
        if (canFill) break;
        ++it;
      }
    } else {
      auto it = bids.begin();

      while (it != bids.end() && it->first >= price) {
        for (const auto& order : it->second) {
          if (order.userId == userId) continue;

          availableQty += order.quantity;
          if (availableQty >= quantity) {
            canFill = true;
            break;
          }
        }
        if (canFill) break;
        ++it;
      }
    }

    if (!canFill) return;
  }

  if(type == orderType::GTC) {
    orderIdPrice[id] = price;
    orderIdSide[id] = isBuy;
  }
  
  if (isBuy) {
    auto it = asks.begin();

    while (it != asks.end() && it->first <= price && quantity > 0) {
      auto itSet = it->second.begin();
    
      while (itSet != it->second.end() && quantity > 0) {
        if(itSet->userId == userId) {
          ++itSet;
          continue;
        }
        int curr = itSet->quantity;

        int tradeQty = std::min(quantity, curr);
        trades.emplace_back(itSet->id, id, it->first, tradeQty, time);
        
        if (quantity >= curr) {
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
      if (it->second.empty()) it = asks.erase(it);
      else ++it;
    }
    if (quantity > 0 && type == orderType::GTC) bids[price].emplace(id, price, quantity, time, userId);
  } else { 
    auto it = bids.begin();

    while (it != bids.end() && it->first >= price && quantity > 0) {
      auto itSet = it->second.begin();
    
      while (itSet != it->second.end() && quantity > 0) {
        if(itSet->userId == userId) {
          ++itSet;
          continue;
        }
        int curr = itSet->quantity;

        int tradeQty = std::min(quantity, curr);
        trades.emplace_back(itSet->id, id, it->first, tradeQty, time);
        
        if (quantity >= curr) {
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
      if (it->second.empty()) it = bids.erase(it);
      else ++it;
    }
    if (quantity > 0 && type == orderType::GTC) asks[price].emplace(id, price, quantity, time, userId);
  }
}

void OrderBook::cancelOrder(int id) {
  auto side_it = orderIdSide.find(id);
  if (side_it == orderIdSide.end()) return;

  bool isBuy = side_it->second;
  double price = orderIdPrice[id];

  Order keyOrder = { id, price, 0, 0, 0 }; 

  if (isBuy) {
    auto priceLevelIt = bids.find(price);
    
    if (priceLevelIt != bids.end()) {
      priceLevelIt->second.erase(keyOrder); 

      if (priceLevelIt->second.empty()) bids.erase(priceLevelIt);
    }
  } else {
    auto priceLevelIt = asks.find(price);

    if (priceLevelIt != asks.end()) {
      priceLevelIt->second.erase(keyOrder);

      if (priceLevelIt->second.empty()) asks.erase(priceLevelIt);
    }
  }

  orderIdSide.erase(side_it);
  orderIdPrice.erase(id);
}

void OrderBook::printOrderBook() const {
  std::cout << "\nBIDS (price desc):\n";
  for (const auto &[price, orders] : bids) {
    for (const auto &order : orders) {
      std::cout << "ID: " << order.id << ", Price: " << price << ", Qty: " << order.quantity << ", Time: " << order.timestamp << '\n';
    }
  }
  std::cout << "\nASKS (price asc):\n";
  for (const auto &[price, orders] : asks) {
    for (const auto &order : orders) {
      std::cout << "ID: " << order.id << ", Price: " << price << ", Qty: " << order.quantity << ", Time: " << order.timestamp << '\n';
    }
  }
}