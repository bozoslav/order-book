#include <benchmark/benchmark.h>
#include "OrderBook.h"
#include "Order.h"
#include "Trade.h"
#include <vector>
#include <random>

static void populate_book(OrderBook& book, int count, std::vector<Trade>& trades) {
  for (int i = 0; i < count; ++i) {
    if (i % 2 == 0) {
      book.addOrder(i, 100.0 - (i % 100) * 0.05, 10, true, 1000 + i, orderType::GTC, trades);
    } else {
      book.addOrder(i, 100.1 + (i % 100) * 0.05, 10, false, 1000 + i, orderType::GTC, trades);
    }
  }
  trades.clear();
}

static void BM_AddOrder_EmptyBook(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    {
      OrderBook book;
      std::vector<Trade> trades;
      state.ResumeTiming();

      book.addOrder(1, 100.0, 10, true, 1001, orderType::GTC, trades);

      state.PauseTiming();
      benchmark::DoNotOptimize(book);
      benchmark::DoNotOptimize(trades);
    }
    state.ResumeTiming();
  }
}
BENCHMARK(BM_AddOrder_EmptyBook);

static void BM_AddOrder_WithExistingOrders(benchmark::State& state) {
  int numExisting = state.range(0);

  OrderBook book;
  std::vector<Trade> trades;
  populate_book(book, numExisting, trades);

  int newOrderId = numExisting + 100;

  for (auto _ : state) {
    state.ResumeTiming();

    book.addOrder(newOrderId, 90.0, 10, true, 20000, orderType::GTC, trades);

    state.PauseTiming();

    book.cancelOrder(newOrderId);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
    state.ResumeTiming();
  }
}

BENCHMARK(BM_AddOrder_WithExistingOrders)->RangeMultiplier(10)->Range(100, 100000);

static void BM_AddOrder_ImmediateMatch(benchmark::State& state) {
  int numExisting = state.range(0);

  OrderBook book;
  std::vector<Trade> trades;
  populate_book(book, numExisting, trades);

  int restingId = numExisting + 1;
  book.addOrder(restingId, 100.05, 10, false, 20000, orderType::GTC, trades);

  int incomingId = numExisting + 2;

  for (auto _ : state) {
    state.ResumeTiming();

    book.addOrder(incomingId, 100.05, 10, true, 20001, orderType::GTC, trades);

    state.PauseTiming();

    book.addOrder(restingId, 100.05, 10, false, 20000, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
    state.ResumeTiming();
  }
}
BENCHMARK(BM_AddOrder_ImmediateMatch)->RangeMultiplier(10)->Range(100, 100000);

static void BM_AddOrder_PartialFill(benchmark::State& state) {
  int numExisting = state.range(0);

  OrderBook book;
  std::vector<Trade> trades;
  populate_book(book, numExisting, trades);

  int restingId = numExisting + 1;
  book.addOrder(restingId, 100.05, 50, false, 20000, orderType::GTC, trades);

  int incomingId = numExisting + 2;

  for (auto _ : state) {
    state.ResumeTiming();

    book.addOrder(incomingId, 100.05, 25, true, 20001, orderType::GTC, trades);

    state.PauseTiming();

    book.cancelOrder(restingId);
    book.addOrder(restingId, 100.05, 50, false, 20000, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
    state.ResumeTiming();
  }
}
BENCHMARK(BM_AddOrder_PartialFill)->RangeMultiplier(10)->Range(100, 100000);

static void BM_AddOrder_MultipleLevelFill(benchmark::State& state) {
  int numLevels = state.range(0);
  int backgroundSize = 1000;

  OrderBook setupBook;
  std::vector<Trade> setupTrades;
  populate_book(setupBook, backgroundSize, setupTrades);

  for (int i = 0; i < numLevels; ++i) {
    setupBook.addOrder(100000 + i, 100.02 + i * 0.001, 10, false, 30000 + i, orderType::GTC, setupTrades);
  }

  for (auto _ : state) {
    state.PauseTiming();
    {
      OrderBook book = setupBook;
      std::vector<Trade> trades;
      state.ResumeTiming();

      book.addOrder(99999, 200.0, 10 * numLevels, true, 40000, orderType::GTC, trades);

      state.PauseTiming();
      benchmark::DoNotOptimize(book);
      benchmark::DoNotOptimize(trades);
    }
    state.ResumeTiming();
  }
}
BENCHMARK(BM_AddOrder_MultipleLevelFill)->Range(5, 50);

static void BM_AddOrder_IOC(benchmark::State& state) {
  int numExisting = state.range(0);
  OrderBook book;
  std::vector<Trade> trades;
  populate_book(book, numExisting, trades);

  int restingId = numExisting + 1;
  book.addOrder(restingId, 100.05, 10, false, 20000, orderType::GTC, trades);

  int incomingId = numExisting + 2;

  for (auto _ : state) {
    state.ResumeTiming();

    book.addOrder(incomingId, 100.05, 10, true, 20001, orderType::IOC, trades);

    state.PauseTiming();

    book.addOrder(restingId, 100.05, 10, false, 20000, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
    state.ResumeTiming();
  }
}
BENCHMARK(BM_AddOrder_IOC)->RangeMultiplier(10)->Range(100, 100000);

static void BM_AddOrder_FOK(benchmark::State& state) {
  int numExisting = state.range(0);
  OrderBook book;
  std::vector<Trade> trades;
  populate_book(book, numExisting, trades);

  int restingId = numExisting + 1;
  book.addOrder(restingId, 100.05, 10, false, 20000, orderType::GTC, trades);

  int incomingId = numExisting + 2;

  for (auto _ : state) {
    state.ResumeTiming();
    book.addOrder(incomingId, 100.05, 10, true, 20001, orderType::FOK, trades);
    state.PauseTiming();

    book.addOrder(restingId, 100.05, 10, false, 20000, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
    state.ResumeTiming();
  }
}
BENCHMARK(BM_AddOrder_FOK)->RangeMultiplier(10)->Range(100, 100000);

static void BM_CancelOrder_DenseBook(benchmark::State& state) {
  int numOrders = state.range(0);

  OrderBook book;
  std::vector<Trade> trades;
  populate_book(book, numOrders, trades);

  int targetId = 999999;
  book.addOrder(targetId, 99.0, 10, true, 50000, orderType::GTC, trades);

  for (auto _ : state) {
    state.ResumeTiming();

    book.cancelOrder(targetId);

    state.PauseTiming();

    book.addOrder(targetId, 99.0, 10, true, 50000, orderType::GTC, trades);

    benchmark::DoNotOptimize(book);
    state.ResumeTiming();
  }
}
BENCHMARK(BM_CancelOrder_DenseBook)->RangeMultiplier(10)->Range(100, 100000);

static void BM_ModifyOrder(benchmark::State& state) {
  int numExisting = state.range(0);
  OrderBook book;
  std::vector<Trade> trades;
  populate_book(book, numExisting, trades);

  int targetId = 999999;
  book.addOrder(targetId, 99.0, 10, true, 50000, orderType::GTC, trades);

  for (auto _ : state) {
    state.ResumeTiming();

    book.modifyOrder(targetId, 99.1, 15, trades);

    state.PauseTiming();

    book.modifyOrder(targetId, 99.0, 10, trades);

    benchmark::DoNotOptimize(book);
    benchmark::DoNotOptimize(trades);
    state.ResumeTiming();
  }
}
BENCHMARK(BM_ModifyOrder)->RangeMultiplier(10)->Range(100, 100000);

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
    } else {
      book.addOrder(nextId++, 102.0, 5, true, nextId, orderType::IOC, trades);
    }

    benchmark::DoNotOptimize(book);
  }
}
BENCHMARK(BM_HighLoad_MixedOperations);

static void BM_WorstCase_DeepBook_Match(benchmark::State& state) {
  int depth = state.range(0);

  OrderBook setupBook;
  std::vector<Trade> setupTrades;
  for (int i = 0; i < depth; ++i) {
    setupBook.addOrder(i, 100.0, 10, false, 1000 + i, orderType::GTC, setupTrades);
  }

  for (auto _ : state) {
    state.PauseTiming();
    {
      OrderBook book = setupBook;
      std::vector<Trade> trades;

      state.ResumeTiming();

      book.addOrder(99999, 100.0, 10 * depth, true, 2000, orderType::GTC, trades);

      state.PauseTiming();
      benchmark::DoNotOptimize(book);
    }
    state.ResumeTiming();
  }
}
BENCHMARK(BM_WorstCase_DeepBook_Match)->Range(100, 2000);

BENCHMARK_MAIN();