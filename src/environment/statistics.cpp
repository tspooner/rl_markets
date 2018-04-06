#include "environment/statistics.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace environment;

void ExperimentStatistics::write(std::string path)
{
    try {
        std::ofstream ofs(path, std::ofstream::out);

        ofs << "reward," << reward << std::endl;
        ofs << "pnl," << pnl << std::endl;
        ofs << "bandh," << bandh << std::endl;

        ofs.close();

    } catch (std::runtime_error& e) {
        std::cout << "Error writing experiment stats to: " << path << std::endl;
        std::cout << e.what() << std::endl;
    }
}

void ExperimentStatistics::reset()
{
    reward = 0.0;
    pnl = 0.0;
    bandh = 0.0;
}

void TradeStatistics::write(std::string path)
{
    try {
        std::ofstream ofs(path, std::ofstream::out);

        ofs << "asks_placed," << asks_placed << std::endl;
        ofs << "bids_placed," << bids_placed << std::endl;
        ofs << "asks_cancelled," << asks_cancelled << std::endl;
        ofs << "bids_cancelled," << bids_cancelled << std::endl;
        ofs << "ask_transactions," << ask_transactions << std::endl;
        ofs << "bid_transactions," << bid_transactions << std::endl;
        ofs << "market_sells," << market_sells << std::endl;
        ofs << "market_buys," << market_buys << std::endl;

        ofs.close();

    } catch (std::runtime_error& e) {
        std::cout << "Error writing trade stats to: " << path << std::endl;
        std::cout << e.what() << std::endl;
    }
}

void TradeStatistics::reset()
{
    ask_transactions = 0;
    bid_transactions = 0;
    bids_placed = 0;
    asks_placed = 0;
    bids_cancelled = 0;
    asks_cancelled = 0;
    market_buys = 0;
    market_sells = 0;
}

void TickStatistics::write(std::string path)
{
    try {
        std::ofstream ofs(path, std::ofstream::out);

        float ask_occ = 100*float(ticks_with_ask)/total_ticks;
        float bid_occ = 100*float(ticks_with_bid)/total_ticks;
        float both_occ = 100*float(ticks_with_both)/total_ticks;
        float position_occ = 100 * float(ticks_with_position)/total_ticks;
        float short_occ = 100 * float(ticks_short)/total_ticks;
        float long_occ = 100 * float(ticks_long)/total_ticks;

        ofs << "ask_occupancy," << ask_occ << "%" << std::endl;
        ofs << "bid_occupancy," << bid_occ << "%" << std::endl;
        ofs << "both_occupancy," << both_occ << "%" << std::endl;
        ofs << "pos_occupancy," << position_occ << "%" << std::endl;
        ofs << "short_occupancy," << short_occ << "%" << std::endl;
        ofs << "long_occupancy," << long_occ << "%" << std::endl;

        ofs.close();

    } catch (std::runtime_error& e) {
        std::cout << "Error writing tick stats to: " << path << std::endl;
        std::cout << e.what() << std::endl;
    }
}

void TickStatistics::reset()
{
    ticks_with_ask = 0;
    ticks_with_no_ask = 0;
    ticks_with_bid = 0;
    ticks_with_no_bid = 0;
    ticks_with_both = 0;
    ticks_without_both = 0;
    ticks_with_position = 0;
    ticks_with_no_position = 0;
    ticks_short = 0;
    ticks_long = 0;
    total_ticks = 0;
}
