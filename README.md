# High-Performance Order Book

A low-latency Order Book implementation in C++ designed for high-frequency trading simulations. This project implements a standard price-time priority matching engine, supporting various order types and real-time order lifecycle management.

## Overview

This engine is built to handle the core mechanics of an exchange matching engine with a focus on correctness and baseline performance. It supports:
- **Order Matching**: Price-Time priority (FIFO).
- **Order Types**: 
  - Limit Orders (GTC - Good Till Cancel)
  - IOC (Immediate or Cancel)
  - FOK (Fill or Kill)
- **Operations**: Add, Cancel, Modify (Price/Quantity).

## Performance Benchmarks

*Note: These are baseline benchmarks for the current architecture using standard STL containers. Optimization passes for memory locality and cache coherence are planned.*

**Environment**: C++17, Single-threaded execution.

### Latency Metrics (Nanoseconds)

| Operation | Scenario | Time (ns) | CPU (ns) |
|-----------|----------|-----------|----------|
| **Add Order** | High Load (Mixed Ops) | **429** | **429** |
| **Add Order** | Immediate Match | 1,250 | 1,250 |
| **Add Order** | IOC (Immediate or Cancel) | 905 | 906 |
| **Add Order** | FOK (Fill or Kill) | 960 | 962 |
| **Add Order** | Empty Book | 1,246 | 1,248 |
| **Modify** | Standard Modify | 1,789 | 1,790 |

### Scalability & Depth Analysis
Analysis of insertion and cancellation costs as book depth grows. The current node-based implementation exhibits O(log N) behavior; future iterations will target constant-time O(1) performance for these operations.

| Existing Orders | Add Order Latency (ns) | Cancel Order Latency (ns) |
|-----------------|------------------------|---------------------------|
| 100 | 6,041 | 7,197 |
| 1,000 | 52,295 | 66,790 |
| 10,000 | 533,634 | 689,485 |
| **100,000** | **5,395,257** | **6,885,175** |

### Worst-Case Matching (Deep Book)
Performance when matching against deep queues:

| Depth | Scenario | Time (ns) |
|-------|----------|-----------|
| 2,000 | Deep Book Match | 307,454 |

## Build & Run

### Prerequisites
- C++17 compatible compiler
- CMake 3.10+

### Build Instructions
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Running Benchmarks
```bash
./build/OrderBookBenchmarks
```

### Running Tests
```bash
./build/OrderBookTests
```