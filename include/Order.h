#pragma once

struct Order {
  int id;
  double price;
  int quantity;
  long long timestamp;

  Order(int id_, double price_, int quantity_, long long timestamp_) : id(id_), price(price_), quantity(quantity_), timestamp(timestamp_) {}

  bool operator<(const Order& other) const {
    if (timestamp != other.timestamp) return timestamp < other.timestamp;
    return id < other.id;
  }
};