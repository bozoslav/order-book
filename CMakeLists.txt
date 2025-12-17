cmake_minimum_required(VERSION 3.14)

project(OrderBook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Options for building tests and benchmarks
option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_BENCHMARKS "Build the benchmarks" ON)

include_directories(include)

# OrderBook library (for reuse in tests and benchmarks)
add_library(OrderBookLib
    src/OrderBook.cpp
)
target_include_directories(OrderBookLib PUBLIC include)

# Main executable
add_executable(OrderBook 
    src/main.cpp
)
target_link_libraries(OrderBook OrderBookLib)

# Fetch Google Test and Google Benchmark
if(BUILD_TESTS OR BUILD_BENCHMARKS)
    include(FetchContent)
endif()

# Google Test
if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    enable_testing()
    
    add_executable(OrderBookTests
        tests/test_order_book.cpp
    )
    target_link_libraries(OrderBookTests
        OrderBookLib
        gtest
        gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(OrderBookTests)
endif()

# Google Benchmark
if(BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
    
    add_executable(OrderBookBenchmarks
        benchmarks/benchmark_order_book.cpp
    )
    target_link_libraries(OrderBookBenchmarks
        OrderBookLib
        benchmark::benchmark
        benchmark::benchmark_main
    )
endif()

#[[ # for recording in instruments
if (APPLE) #cmake -DCMAKE_BUILD_TYPE=Debug ..
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(OrderBook PRIVATE -g)
    endif()

    find_program(DSYMUTIL_PROGRAM dsymutil)
    if (DSYMUTIL_PROGRAM)
        add_custom_command(
            TARGET OrderBook
            POST_BUILD
            # The command runs dsymutil on the newly built executable.
            COMMAND ${DSYMUTIL_PROGRAM} "$<TARGET_FILE:OrderBook>"
            COMMENT "Generating dSYM bundle for OrderBook"
        )
    endif()
endif()
]]#

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")