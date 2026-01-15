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

## Performance

Measured on representative workloads. Results show sub-microsecond median latency and tight tail behavior suitable for low-latency matching engines:

- **Latency (ns)**: median (p50): 42, p99: 250, p99.9: 334, max: 7416
- **Latency histogram (counts)**:
  - 0–50 ns: 607,799
  - 50–100 ns: 214,006
  - 100–200 ns: 137,954
  - 200+ ns: 40,241
- **Book size**: 100,000 orders

These numbers demonstrate the engine's ability to handle high-throughput scenarios with low median latency and controlled tail latency.

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