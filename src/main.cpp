#include "Trade.h"
#include "Order.h"
#include "OrderBook.h"
#include <unordered_set>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <cmath>

int main() {
  std::string outDir = "../out";
  mkdir(outDir.c_str(), 0777);

  std::ofstream logFile((outDir + "/log.txt").c_str());
  std::cout.rdbuf(logFile.rdbuf());



  OrderBook book;
  std::vector<Trade> trades;

  std::random_device rd;
  std::mt19937 gen(rd());
  double currentPrice = 100.00;
  std::uniform_int_distribution<> sideGen(0, 1);
  std::uniform_int_distribution<> qtyGen(1, 100);
  std::uniform_real_distribution<> priceMove(-0.50, 0.50);
  std::uniform_real_distribution<> cancelChance(0.0, 1.0);
  std::uniform_int_distribution<> usrId(1, 100000000);

  std::unordered_set<int> activeOrderIds;
  std::vector<int> idVec;
  idVec.reserve(100000);
  int nextOrderId = 1;

  std::uniform_real_distribution<> modifyChance(0.0, 1.0);
  std::uniform_real_distribution<> newPriceMove(-1.0, 1.0);
  std::uniform_int_distribution<> newQtyGen(1, 100);

  for(int i = 1; i <= 10000000; ++i) {
    double r = modifyChance(gen);
    if (!idVec.empty() && r < 0.3) {
      // Modify order 30% of the time
      std::uniform_int_distribution<> activeIdGen(0, static_cast<int>(idVec.size()) - 1);
      int idx = activeIdGen(gen);
      int modID = idVec[idx];
      double newPrice = currentPrice + newPriceMove(gen);
      newPrice = std::round(newPrice * 100.0) / 100.0;
      int newQty = newQtyGen(gen);
      book.modifyOrder(modID, newPrice, newQty, trades);
    } else if (!idVec.empty() && r < 0.6) {
      // Cancel order 30% of the time (0.3-0.6)
      std::uniform_int_distribution<> activeIdGen(0, static_cast<int>(idVec.size()) - 1);
      int idx = activeIdGen(gen);
      int cancelID = idVec[idx];
      book.cancelOrder(cancelID);
      activeOrderIds.erase(cancelID);
      idVec[idx] = idVec.back();
      idVec.pop_back();
    } else {
      // Add order
      currentPrice += priceMove(gen);
      currentPrice = std::round(currentPrice * 100.0) / 100.0;
      int qty = qtyGen(gen);
      bool isBuy = sideGen(gen) == 0;
      book.addOrder(nextOrderId, currentPrice, qty, isBuy, usrId(gen), orderType::GTC, trades);
      activeOrderIds.insert(nextOrderId);
      idVec.push_back(nextOrderId);
      ++nextOrderId;
    }
  }

  std::cin.tie(NULL)->sync_with_stdio(false);
  for(auto i = trades.begin(); i != trades.end(); ++i) {
    std::cout << "passive: " << i->passiveId << " agressive: " << i->agressiveId << " price: " << i->price
              << " quantity: " << i->quantity << " time: " << i->timestamp << "\n";
  }

  return 0;
}