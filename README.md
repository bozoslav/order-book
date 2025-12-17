# High-Performance Order Book

A high-performance, low-latency order book implementation in C++17, designed for top-tier trading firm applications (IMC, Jump Trading, Citadel). This implementation features comprehensive testing, micro-benchmarking, and performance visualization capabilities.

## Features

- **Price-Time Priority Matching Engine**: Orders matched based on price-time priority
- **Multiple Order Types**: Support for GTC (Good-Till-Cancel), IOC (Immediate-or-Cancel), and FOK (Fill-or-Kill)
- **Self-Match Prevention**: Prevents orders from the same user from matching
- **Order Management**: Add, modify, and cancel orders
- **Comprehensive Testing**: Google Test-based unit test suite
- **Performance Benchmarking**: Google Benchmark-based micro-benchmarks
- **Visualization**: Python-based performance visualization tools

## Project Structure

```
order-book/
├── include/           # Header files
│   ├── OrderBook.h    # Main order book interface
│   ├── Order.h        # Order structure
│   ├── Trade.h        # Trade structure
│   └── Price.h        # Price type with fixed-point arithmetic
├── src/               # Source files
│   ├── OrderBook.cpp  # Order book implementation
│   └── main.cpp       # Example usage
├── tests/             # Unit tests
│   └── test_order_book.cpp
├── benchmarks/        # Performance benchmarks
│   └── benchmark_order_book.cpp
├── scripts/           # Utility scripts
│   └── visualize_benchmarks.py
└── CMakeLists.txt     # Build configuration
```

## Building

### Prerequisites

- CMake 3.14 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Python 3.7+ (for visualization scripts)

### Build Commands

```bash
# Clone the repository
git clone https://github.com/bozoslav/order-book.git
cd order-book

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build all targets
make

# Or build specific targets
make OrderBook           # Main executable
make OrderBookTests      # Test suite
make OrderBookBenchmarks # Benchmark suite
```

### Build Options

You can control what gets built using CMake options:

```bash
# Build without tests
cmake -DBUILD_TESTS=OFF ..

# Build without benchmarks
cmake -DBUILD_BENCHMARKS=OFF ..

# Build with optimizations for production
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## Running Tests

The test suite uses Google Test and covers:
- Order placement at different price levels
- Matching logic with price-time priority
- Full and partial fills
- Resting orders
- Order cancellations
- Edge cases (crossing spread, self-matching, invalid inputs)
- Different order types (GTC, IOC, FOK)

```bash
# Run all tests
./build/OrderBookTests

# Run with verbose output
./build/OrderBookTests --gtest_verbose

# Run specific test
./build/OrderBookTests --gtest_filter=OrderBookTest.SimpleMatchBuyAgainstSell
```

## Running Benchmarks

The benchmark suite uses Google Benchmark and measures:
- **AddOrder latency** (hot path performance)
- **CancelOrder latency**
- **Matching engine throughput** (orders per second)
- **Latency percentiles** (p50, p99, p99.9)

```bash
# Run all benchmarks
./build/OrderBookBenchmarks

# Run with JSON output for visualization
./build/OrderBookBenchmarks --benchmark_format=json --benchmark_out=results.json

# Run specific benchmark
./build/OrderBookBenchmarks --benchmark_filter=BM_AddOrder

# Control iterations and repetitions
./build/OrderBookBenchmarks --benchmark_min_time=1.0 --benchmark_repetitions=10
```

### Understanding Benchmark Results

Key metrics to look for:
- **Real Time**: Wall-clock time for the operation (includes system overhead)
- **CPU Time**: Actual CPU time spent (best indicator of computational cost)
- **Items per second**: Throughput measurement (for throughput benchmarks)
- **Iterations**: Number of times the benchmark was run

Example output:
```
BM_AddOrder_EmptyBook              185 ns          184 ns      3789474
BM_AddOrder_ImmediateMatch         453 ns          452 ns      1547891
BM_MatchingThroughput/1000    1547891 items/s
```

## Visualizing Results

Generate performance graphs from benchmark results:

```bash
# Install Python dependencies
pip install -r requirements.txt

# Run benchmarks with JSON output
./build/OrderBookBenchmarks --benchmark_format=json --benchmark_out=results.json

# Generate visualizations
python3 scripts/visualize_benchmarks.py results.json

# Specify custom output directory
python3 scripts/visualize_benchmarks.py results.json -o my_results/
```

This generates:
- **latency_comparison.png**: Bar chart of mean latencies and percentiles
- **throughput_comparison.png**: Throughput comparisons across scenarios
- **scaling_analysis.png**: How operations scale with book size
- **benchmark_summary.txt**: Text summary of all metrics

## Performance Characteristics

This order book is optimized for:

### Hot Path Operations
- **AddOrder (no match)**: ~150-300 ns
- **AddOrder (immediate match)**: ~400-600 ns
- **CancelOrder**: ~100-200 ns

### Throughput
- **Sequential operations**: 1-5 million orders/second
- **With matching**: 500k-2 million orders/second (depending on match rate)

### Scalability
- Logarithmic complexity for price level lookups
- Linear time priority within price levels
- Efficient memory usage with STL containers

## Design Decisions

### Data Structures
- **`std::map`**: For price level organization (sorted by price)
  - Bids: Descending order (highest price first)
  - Asks: Ascending order (lowest price first)
- **`std::set`**: For orders at each price level (sorted by timestamp)
- **`std::unordered_map`**: For O(1) order lookup by ID

### Price Representation
- Fixed-point arithmetic (2 decimal places)
- Avoids floating-point precision issues
- Efficient comparison and storage

### Order Types
- **GTC**: Orders rest in the book until filled or cancelled
- **IOC**: Immediately fill available quantity, cancel remainder
- **FOK**: Fill entire order immediately or reject completely

## Use Cases

This implementation is suitable for:
- ✅ Educational purposes and understanding market microstructure
- ✅ Backtesting trading strategies
- ✅ Simulating order book dynamics
- ✅ Performance benchmarking and optimization research
- ✅ Interview preparation for trading firms

For production HFT systems, consider:
- Lock-free data structures
- Custom memory allocators
- FPGA/hardware acceleration
- Kernel bypass networking (e.g., Solarflare, Mellanox)

## Contributing

Contributions are welcome! Areas for improvement:
- Additional order types (Stop, Market, Iceberg)
- Market data feeds (top-of-book, depth)
- More sophisticated benchmarks
- Multi-threading support
- Memory profiling

## License

MIT License - See LICENSE file for details

## References

- [Google Test Documentation](https://google.github.io/googletest/)
- [Google Benchmark Documentation](https://github.com/google/benchmark)
- [CMake Documentation](https://cmake.org/documentation/)
- [Low-Latency Trading Systems](https://www.amazon.com/Building-Low-Latency-Applications-Complete-Ultra-Low/dp/1803242809)

## Acknowledgments

Designed for high-performance trading applications with inspiration from industry best practices at firms like Jane Street, Jump Trading, IMC, and Citadel.
