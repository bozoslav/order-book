#pragma once

#include <iostream>

struct Price {
  long long value;

  Price(double price) : value(static_cast<long long>(price * 100 + 0.5)) {}
  Price(long long v) : value(v) {}
  Price() : value(0) {}

  double to_double() const { return value / 100.0; }

  bool operator<(const Price& other) const { return value < other.value; }
  bool operator>(const Price& other) const { return value > other.value; }
  bool operator==(const Price& other) const { return value == other.value; }
  bool operator!=(const Price& other) const { return value != other.value; }
  bool operator<=(const Price& other) const { return value <= other.value; }
  bool operator>=(const Price& other) const { return value >= other.value; }

  friend std::ostream& operator<<(std::ostream& os, const Price& p) {
    os << p.to_double();
    return os;
  }
};