#include <benchmark/benchmark.h>
#include "OrderBook.h"
#include "Order.h"
#include "Trade.h"
#include <vector>
#include <random>

double generate_price(int i) {
    return 100.0 + (i % 1000) * 0.05;
}

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
      book.addOrder(i, generate_price(i), 10, i % 2 == 0, 1000 + i, orderType::GTC, trades);
    }

    trades.clear();
    state.ResumeTiming();

    book.addOrder(numExisting + 1, 105.0, 10, true, 2001, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}

BENCHMARK(BM_AddOrder_WithExistingOrders)->RangeMultiplier(10)->Range(100, 100000);

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
      // Create sell orders at different price levels
      book.addOrder(i, 100.0 + i * 0.1, 10, false, 1000 + i, orderType::GTC, trades);
    }
    trades.clear();
    state.ResumeTiming();

    // Buy order that sweeps through all levels
    book.addOrder(99999, 200.0, 10 * numLevels, true, 2000, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
  }
}
BENCHMARK(BM_AddOrder_MultipleLevelFill)->Range(5, 50);

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
BENCHMARK(BM_CancelOrder_DenseBook)->RangeMultiplier(10)->Range(100, 100000);

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

static void BM_HighLoad_MixedOperations(benchmark::State& state) {
  int initialOrders = 10000;
  
  OrderBook book;
  std::vector<Trade> trades;
  for (int i = 0; i < initialOrders; ++i) {
    bool isBuy = (i % 2 == 0);
    double price = isBuy ? (99.0 - (i%100)*0.01) : (101.0 + (i%100)*0.01);
    book.addOrder(i, price, 100, isBuy, i, orderType::GTC, trades);
  }
  
  int nextId = initialOrders;
  std::mt19937 rng(42);
  std::uniform_int_distribution<int> op_dist(0, 2);
  std::uniform_int_distribution<int> id_dist(0, initialOrders - 1);

  for (auto _ : state) {
    int op = op_dist(rng);
      
    if (op == 0) {
      book.addOrder(nextId++, 90.0, 10, true, nextId, orderType::GTC, trades);
    } else if (op == 1) {
      int cancelId = id_dist(rng); 
      book.cancelOrder(cancelId);
    }
    else book.addOrder(nextId++, 102.0, 5, true, nextId, orderType::IOC, trades);
      
    benchmark::DoNotOptimize(book);
  }
}
BENCHMARK(BM_HighLoad_MixedOperations);

static void BM_WorstCase_DeepBook_Match(benchmark::State& state) {
  int depth = state.range(0);

  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    std::vector<Trade> trades;
        
    for(int i = 0; i < depth; ++i) {
      book.addOrder(i, 100.0, 10, false, 1000+i, orderType::GTC, trades);
    }
    
    state.ResumeTiming();

    book.addOrder(99999, 100.0, 10 * depth, true, 2000, orderType::GTC, trades);
        
    benchmark::DoNotOptimize(book);
  }
}
BENCHMARK(BM_WorstCase_DeepBook_Match)->Range(100, 2000);

BENCHMARK_MAIN();