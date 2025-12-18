#pragma once

#include <Price.h>

struct Trade {
  long long passiveId;
  long long agressiveId;
  Price price;
  int quantity;
  long long timestamp;

  Trade(long long passiveId_, long long agressiveId_, Price price_, int quantity_, long long timestamp_)
    : passiveId(passiveId_), agressiveId(agressiveId_), price(price_), quantity(quantity_), timestamp(timestamp_) {}
};