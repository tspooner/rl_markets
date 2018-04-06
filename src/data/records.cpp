#include "data/records.h"

#include <numeric>
#include <algorithm>

using namespace data;


void Record::clear()
{
    date = 0;
    time = 0;
}


void MarketDepthRecord::clear()
{
    Record::clear();

    ask_prices.fill(0);
    ask_volumes.fill(0);

    bid_prices.fill(0);
    bid_volumes.fill(0);
}


double TimeAndSalesRecord::mean_price() const
{
    double sum = std::accumulate(transactions.begin(),
                                 transactions.end(), 0.0,
                                 [](double a, const std::pair<double, long>& b) {
                                    return a + b.first;
                                 });

    return sum / (double) transactions.size();
}

void TimeAndSalesRecord::clear()
{
    Record::clear();

    transactions.clear();
}
