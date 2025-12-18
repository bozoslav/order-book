#pragma once

#include <Price.h>

struct Order {
  int id;
  Price price;
  int quantity;
  long long timestamp;
  long long userId;

  Order(int id_, double price_, int quantity_, long long timestamp_, long long userId_) 
    : id(id_), price(price_), quantity(quantity_), timestamp(timestamp_), userId(userId_) {}

  Order(int id_, Price price_, int quantity_, long long timestamp_, long long userId_)
    : id(id_), price(price_), quantity(quantity_), timestamp(timestamp_), userId(userId_) {}

  bool operator<(const Order& other) const {
    if (timestamp != other.timestamp) return timestamp < other.timestamp;
    return id < other.id;
  }
};