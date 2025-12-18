#include <benchmark/benchmark.h>
#include "OrderBook.h"
#include "Order.h"
#include "Trade.h"
#include <vector>
#include <random>

static void BM_AddOrder_EmptyBook(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;
    state.ResumeTiming();
        
    book.addOrder(1, 100.0, 10, true, 1001, orderType::GTC, trades);
        
    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}
BENCHMARK(BM_AddOrder_EmptyBook);

static void BM_AddOrder_WithExistingOrders(benchmark::State& state) {
  int numExisting = state.range(0);
    
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;

    for (int i = 0; i < numExisting; ++i) {
      book.addOrder(i, 100.0 + (i % 10) * 0.1, 10, i % 2 == 0, 1000 + i, orderType::GTC, trades);
    }

    trades.clear();
    state.ResumeTiming();

    book.addOrder(numExisting + 1, 105.0, 10, true, 2001, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}
BENCHMARK(BM_AddOrder_WithExistingOrders)->Range(10, 1000);

static void BM_AddOrder_ImmediateMatch(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;
    book.addOrder(1, 100.0, 10, false, 1001, orderType::GTC, trades);
    trades.clear();
    state.ResumeTiming();

    book.addOrder(2, 100.0, 10, true, 1002, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}
BENCHMARK(BM_AddOrder_ImmediateMatch);

static void BM_AddOrder_PartialFill(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;
    book.addOrder(1, 100.0, 50, false, 1001, orderType::GTC, trades);
    trades.clear();
    state.ResumeTiming();

    book.addOrder(2, 100.0, 25, true, 1002, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}
BENCHMARK(BM_AddOrder_PartialFill);

static void BM_AddOrder_MultipleLevelFill(benchmark::State& state) {
  int numLevels = state.range(0);

  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;
        
    for (int i = 0; i < numLevels; ++i) {
      book.addOrder(i, 100.0 + i * 0.1, 10, false, 1000 + i, orderType::GTC, trades);
    }

    trades.clear();
    state.ResumeTiming();
        
    book.addOrder(1000, 100.0 + numLevels * 0.1, numLevels * 10, true, 2001, orderType::GTC, trades);
        
    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}
BENCHMARK(BM_AddOrder_MultipleLevelFill)->Range(2, 20);

static void BM_AddOrder_IOC(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;
    book.addOrder(1, 100.0, 10, false, 1001, orderType::GTC, trades);
    trades.clear();
    state.ResumeTiming();

    book.addOrder(2, 100.0, 10, true, 1002, orderType::IOC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}
BENCHMARK(BM_AddOrder_IOC);

static void BM_AddOrder_FOK(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;
    book.addOrder(1, 100.0, 10, false, 1001, orderType::GTC, trades);
    trades.clear();
    state.ResumeTiming();

    book.addOrder(2, 100.0, 10, true, 1002, orderType::FOK, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}
BENCHMARK(BM_AddOrder_FOK);

static void BM_CancelOrder_SparseBook(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;
    book.addOrder(1, 100.0, 10, true, 1001, orderType::GTC, trades);
    book.addOrder(2, 101.0, 10, true, 1002, orderType::GTC, trades);
    book.addOrder(3, 102.0, 10, true, 1003, orderType::GTC, trades);
    state.ResumeTiming();
        
    book.cancelOrder(2);
        
    benchmark::DoNotOptimize(book);
  }
}
BENCHMARK(BM_CancelOrder_SparseBook);

static void BM_CancelOrder_DenseBook(benchmark::State& state) {
  int numOrders = state.range(0);
    
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;

    for (int i = 0; i < numOrders; ++i) {
      book.addOrder(i, 100.0, 10, true, 1000 + i, orderType::GTC, trades);
    }
    
    state.ResumeTiming();
        
    book.cancelOrder(numOrders / 2);
        
    benchmark::DoNotOptimize(book);
  }
}
BENCHMARK(BM_CancelOrder_DenseBook)->Range(10, 1000);

static void BM_CancelOrder_NonExistent(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;
    book.addOrder(1, 100.0, 10, true, 1001, orderType::GTC, trades);
    state.ResumeTiming();

    book.cancelOrder(999);

    benchmark::DoNotOptimize(book);
  }
}
BENCHMARK(BM_CancelOrder_NonExistent);

static void BM_ModifyOrder(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;
    book.addOrder(1, 100.0, 10, true, 1001, orderType::GTC, trades);
    trades.clear();
    state.ResumeTiming();

    book.modifyOrder(1, 101.0, 15, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}
BENCHMARK(BM_ModifyOrder);

static void BM_MatchingThroughput(benchmark::State& state) {
  std::random_device rd;
  std::mt19937 gen(42);
  std::uniform_int_distribution<> sideGen(0, 1);
  std::uniform_int_distribution<> qtyGen(1, 100);
  std::uniform_real_distribution<> priceOffset(-0.5, 0.5);

  long long totalOrders = 0;

  for (auto _ : state) {
    OrderBook book;
    std::vector<Trade> trades;
    double basePrice = 100.0;
        
    for (int i = 0; i < state.range(0); ++i) {
      bool isBuy = sideGen(gen) == 0;
      double price = basePrice + priceOffset(gen);
      int qty = qtyGen(gen);
            
      book.addOrder(i, price, qty, isBuy, 1000 + i, orderType::GTC, trades);
      totalOrders++;
    }

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
    
  state.SetItemsProcessed(totalOrders);
}
BENCHMARK(BM_MatchingThroughput)->Range(100, 10000);

static void BM_MatchingThroughput_HighMatchRate(benchmark::State& state) {
  std::random_device rd;
  std::mt19937 gen(42);
  std::uniform_int_distribution<> qtyGen(1, 50);

  long long totalOrders = 0;

  for (auto _ : state) {
    OrderBook book;
    std::vector<Trade> trades;
    double basePrice = 100.0;
        
    for (int i = 0; i < state.range(0); i++) {
      bool isBuy = i % 2 == 0;
      int qty = qtyGen(gen);

      book.addOrder(i, basePrice, qty, isBuy, 1000 + i, orderType::GTC, trades);
      totalOrders++;
    }
        
    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
    
  state.SetItemsProcessed(totalOrders);
}
BENCHMARK(BM_MatchingThroughput_HighMatchRate)->Range(100, 10000);

static void BM_MatchingThroughput_Realistic(benchmark::State& state) {
  std::random_device rd;
  std::mt19937 gen(42);
  std::uniform_int_distribution<> sideGen(0, 1);
  std::uniform_int_distribution<> qtyGen(1, 100);
  std::uniform_real_distribution<> priceOffset(-2.0, 2.0);
  std::uniform_real_distribution<> actionGen(0.0, 1.0);

  long long totalOrders = 0;

  for (auto _ : state) {
    OrderBook book;
    std::vector<Trade> trades;
    std::vector<int> activeOrders;
    double basePrice = 100.0;
    int nextOrderId = 1;

    for (int i = 0; i < state.range(0); ++i) {
      double action = actionGen(gen);
            
      if (!activeOrders.empty() && action < 0.2) {
        int idx = gen() % activeOrders.size();
        int cancelId = activeOrders[idx];
        book.cancelOrder(cancelId);
        activeOrders.erase(activeOrders.begin() + idx);
      } else {
        bool isBuy = sideGen(gen) == 0;
        double price = basePrice + priceOffset(gen);
        int qty = qtyGen(gen);
                
        book.addOrder(nextOrderId, price, qty, isBuy, 1000 + nextOrderId, orderType::GTC, trades);
        activeOrders.push_back(nextOrderId);
        nextOrderId++;
        totalOrders++;
      }
    }

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
    
  state.SetItemsProcessed(totalOrders);
}
BENCHMARK(BM_MatchingThroughput_Realistic)->Range(100, 10000);

static void BM_AddOrder_LatencyDistribution(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;

    for (int i = 0; i < 50; ++i) {
      book.addOrder(i, 100.0 + (i % 10) * 0.1, 10, i % 2 == 0, 1000 + i, orderType::GTC, trades);
    }

    trades.clear();
    state.ResumeTiming();

    book.addOrder(100, 105.0, 10, true, 2001, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}
BENCHMARK(BM_AddOrder_LatencyDistribution);

static void BM_CancelOrder_LatencyDistribution(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;

    for (int i = 0; i < 100; ++i) {
      book.addOrder(i, 100.0 + (i % 10) * 0.1, 10, i % 2 == 0, 1000 + i, orderType::GTC, trades);
    }

    state.ResumeTiming();
        
    book.cancelOrder(50);
        
    benchmark::DoNotOptimize(book);
  }
}
BENCHMARK(BM_CancelOrder_LatencyDistribution);

BENCHMARK_MAIN();
