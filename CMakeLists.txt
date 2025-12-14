cmake_minimum_required(VERSION 3.10)

project(OrderBook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(OrderBook 
    src/main.cpp 
    src/OrderBook.cpp
)