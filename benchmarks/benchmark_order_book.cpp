#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "OrderBook.h"
#include "Order.h"
#include "Trade.h"

// ANSI color codes
#define COL_GREEN   "\033[32m"
#define COL_YELLOW  "\033[33m"
#define COL_MAGENTA "\033[35m"
#define COL_RED     "\033[31m"
#define COL_RESET   "\033[0m"

constexpr int WARMUP_RUNS = 1e7;
constexpr int BENCH_RUNS = 1e6;
constexpr int RANGE_LOW = 1e5; 

static int currIdGlobal = 1;

const std::vector<unsigned long long> histogram_edges = { 50, 100, 200, ULLONG_MAX };

void populate_book(OrderBook& book, int count, std::vector<Trade>& trades, std::mt19937& rng) {
  // realistic per-side price dynamics using geometric Brownian motion
  double buy_curr = 102.5;
  double ask_curr = 107.5;
  const double mu = 0.0;
  const double sigma = 0.008; // volatility ~0.8% per step
  std::normal_distribution<double> z(0.0, 1.0);

  while(book.numOrders() < count) {
    bool is_buy = (currIdGlobal % 2 == 0);
    double price = 100.0;

    if (is_buy) {
      double step = (mu - 0.5 * sigma * sigma) + sigma * z(rng);
      buy_curr = buy_curr * std::exp(step);
      if (buy_curr < 100.0) buy_curr = 100.0;
      if (buy_curr >= 105.0) buy_curr = 104.999999;
      price = buy_curr;
    } else {
      double step = (mu - 0.5 * sigma * sigma) + sigma * z(rng);
      ask_curr = ask_curr * std::exp(step);
      if (ask_curr > 110.0) ask_curr = 110.0;
      if (ask_curr < 105.0) ask_curr = 105.0;
      price = ask_curr;
    }

    book.addOrder(currIdGlobal, price, 20, is_buy, currIdGlobal, orderType::GTC, trades);
    ++currIdGlobal;
  }
  trades.clear();
}

void print_stats(const std::vector<unsigned long long>& lat) {
  if (lat.empty()) return;
  
  std::vector<unsigned long long> sorted = lat;
  std::sort(sorted.begin(), sorted.end());
  
  auto p50 = sorted[sorted.size() / 2];
  auto p99 = sorted[static_cast<int>(sorted.size() * 0.99)];
  auto p999 = sorted[static_cast<int>(sorted.size() * 0.999)];
  auto max = sorted.back();

  std::cout << "Latency ns: median (p50): " << p50
            << ", p99: " << p99
            << ", p99.9: " << p999
            << ", max: " << max << "\n";
}

void print_histogram(const std::vector<unsigned long long>& lat) {
  std::vector<int> bins(histogram_edges.size(), 0);
  for (unsigned long long ns : lat) {
    for (int i = 0; i < histogram_edges.size(); ++i) {
      if (ns < histogram_edges[i]) {
        bins[i]++;
        break;
      }
    }
  }

  const char* colors[] = {COL_GREEN, COL_YELLOW, COL_MAGENTA, COL_RED};
  const char* labels[] = {"0-50ns", "50-100ns", "100-200ns", "200+ns"};
  
  for (int i = 0; i < bins.size(); ++i) {
    int bar_len = bins[i] > 0 ? 1 + int(30.0 * bins[i] / lat.size()) : 0;
    std::cout << "[ " << colors[i];
    
    for (int j = 0; j < bar_len; ++j) std::cout << "■";
    
    std::cout << COL_RESET << std::string(33 - bar_len, ' ')
              << "] " << labels[i]
              << ": " << bins[i] << "\n";
    }
}

int main() {
  std::mt19937 rng(1234);
  std::uniform_int_distribution<int> op_picker(0, 2);
  std::uniform_real_distribution<double> uniform_price(100.0, 110.0);
  std::normal_distribution<double> normal_price(105.0, 2.0);
  std::exponential_distribution<double> exp_price(1.0 / 3.0);
  std::student_t_distribution<double> t_price(3.0);
  std::lognormal_distribution<double> logn_price(std::log(105.0), 0.08);

  auto sample_price = [&](int dist_idx) {
    double v = 100.0;
    switch (dist_idx) {
      case 0: v = uniform_price(rng); break;
      case 1: v = normal_price(rng); break;
      case 2: v = 100.0 + std::min(exp_price(rng), 10.0); break;
      case 3: v = 105.0 + t_price(rng) * 1.5; break;
      case 4: v = logn_price(rng); break;
      default: v = uniform_price(rng); break;
    }
    if (v < 100.0) v = 100.0;
    if (v > 110.0) v = 110.0;
    return v;
  };

  int desired_size = RANGE_LOW;
  
  const char* dist_names[] = {"uniform", "normal", "exponential", "student_t", "lognormal", "uniform"};

  for (int distribution = 0; distribution < 6; ++distribution) {
    OrderBook hot_book;
    std::vector<Trade> trades;
    populate_book(hot_book, desired_size, trades, rng);

    int beginSize = hot_book.numOrders();
    
    std::vector<int> active_order_ids;
    active_order_ids.reserve(desired_size + BENCH_RUNS);
    for (int i = 0; i < desired_size; ++i) active_order_ids.push_back(i);

    std::vector<unsigned long long> latencies;
    latencies.reserve(BENCH_RUNS);

    for (int i = 0; i < BENCH_RUNS; ++i) {
      int op = op_picker(rng);

      unsigned long long t0 = 0, t1 = 0;

      if (op == 0 || active_order_ids.size() < 5) {
        int order_id = desired_size + i;
        bool is_buy = (order_id & 1) == 0;
        double price = sample_price(distribution);
        int size = 10 + (order_id % 10);

        int before = hot_book.numOrders();
        t0 = std::chrono::steady_clock::now().time_since_epoch().count();
        hot_book.addOrder(order_id, price, size, is_buy, 800000 + i, orderType::GTC, trades);
        t1 = std::chrono::steady_clock::now().time_since_epoch().count();
        int after = hot_book.numOrders();

        active_order_ids.push_back(order_id);
      } else if (op == 1) {
        std::uniform_int_distribution<int> idx_picker(0, active_order_ids.size() - 1);
        int idx = idx_picker(rng);
        int cancel_id = active_order_ids[idx];

        int before = hot_book.numOrders();
        t0 = std::chrono::steady_clock::now().time_since_epoch().count();
        hot_book.cancelOrder(cancel_id);
        t1 = std::chrono::steady_clock::now().time_since_epoch().count();
        int after = hot_book.numOrders();

        std::swap(active_order_ids[idx], active_order_ids.back());
        active_order_ids.pop_back();
      } else {
        std::uniform_int_distribution<int> idx_picker(0, active_order_ids.size() - 1);
        int idx = idx_picker(rng);
        int mod_id = active_order_ids[idx];
        double new_price = sample_price(distribution);
        int new_qty = 10 + (mod_id % 10);

        int before = hot_book.numOrders();
        t0 = std::chrono::steady_clock::now().time_since_epoch().count();
        hot_book.modifyOrder(mod_id, new_price, new_qty, trades);
        t1 = std::chrono::steady_clock::now().time_since_epoch().count();
        int after = hot_book.numOrders();
      }

      populate_book(hot_book, desired_size, trades, rng);
      
      latencies.push_back((t1 > t0) ? (t1 - t0) : 0);
    }

    if(distribution > 0) {
      std::cout << "=== Distribution " << distribution << " (" << dist_names[distribution] << ") ===\n";
      print_stats(latencies);
      print_histogram(latencies);
      std::cout << "begin book size: " << beginSize << '\n';
      std::cout << "end book size: " << hot_book.numOrders() << '\n';
    }
  }
  

  return 0;
}