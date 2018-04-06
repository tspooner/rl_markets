#ifndef DATA_RECORDS_H
#define DATA_RECORDS_H

#include <map>
#include <array>

#include "utilities/comparison.h"

namespace data
{

struct Record
{
    int date;
    long time;

    void clear();
};

struct MarketDepthRecord: Record
{
    std::array<double, 5> ask_prices;
    std::array<long, 5> ask_volumes;

    std::array<double, 5> bid_prices;
    std::array<long, 5> bid_volumes;

    void clear();
};

struct TimeAndSalesRecord: Record
{
    std::map<double, long, FloatComparator<>> transactions;

    double mean_price() const;

    void clear();
};

}

#endif
