#ifndef ENVIRONMENT_STATISTICS_H
#define ENVIRONMENT_STATISTICS_H

#include <string>

namespace environment {

struct ExperimentStatistics
{
    double reward = 0.0f;

    double pnl = 0.0f;
    double bandh = 0.0f;

    void write(std::string path);
    void reset();
};

struct TradeStatistics
{
    int ask_transactions = 0;
    int bid_transactions = 0;
    int bids_placed = 0;
    int asks_placed = 0;
    int bids_cancelled = 0;
    int asks_cancelled = 0;
    int market_buys = 0;
    int market_sells = 0;

    void write(std::string path);
    void reset();
};

struct TickStatistics
{
    int ticks_with_ask = 0;
    int ticks_with_no_ask = 0;
    int ticks_with_bid = 0;
    int ticks_with_no_bid = 0;
    int ticks_with_both = 0;
    int ticks_without_both = 0;
    int ticks_with_position = 0;
    int ticks_with_no_position = 0;
    int ticks_short = 0;
    int ticks_long = 0;
    int total_ticks = 0;

    void write(std::string path);
    void reset();
};

}

#endif
