#include "market/book.h"
#include "market/measures.h"
#include "market/target_price.h"

using namespace market::tp;


TargetPrice::TargetPrice():
    val_(-1.0)
{}

double TargetPrice::get()
{
    return val_;
}

bool TargetPrice::ready()
{
    return val_ > 0.0;
}

void TargetPrice::update(market::AskBook<>&, market::BidBook<>&) {}

void TargetPrice::clear() {}


MidPrice::MidPrice(int lookback):
    TargetPrice(),

    mp_(lookback)
{}

bool MidPrice::ready()
{
    return mp_.full();
}

void MidPrice::update(market::AskBook<>& ab, market::BidBook<>& bb)
{
    mp_.push(market::measure::midprice(ab, bb));

    val_ = mp_.mean();
}

void MidPrice::clear() { mp_.clear(); }


MicroPrice::MicroPrice(int lookback):
    TargetPrice(),

    mp_(lookback)
{}

bool MicroPrice::ready()
{
    return mp_.full();
}

void MicroPrice::update(market::AskBook<>& ab, market::BidBook<>& bb)
{
    mp_.push(market::measure::microprice(ab, bb));

    val_ = mp_.mean();
}

void MicroPrice::clear() { mp_.clear(); }


VWAP::VWAP(int lookback):
    TargetPrice(),

    numerator_(lookback),
    denominator_(lookback)
{}

bool VWAP::ready()
{
    return numerator_.full() and denominator_.full();
}

void VWAP::update(market::AskBook<>& ab, market::BidBook<>& bb)
{
    numerator_.push(ab.observed_value() + bb.observed_value());
    denominator_.push(ab.observed_volume() + bb.observed_volume());

    val_ = numerator_.sum() / denominator_.sum();
}

void VWAP::clear()
{
    numerator_.clear();
    denominator_.clear();
}

