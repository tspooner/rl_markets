#include "environment/risk_manager.h"

#include "market/book.h"
#include "market/measures.h"
#include "utilities/comparison.h"

#include <iostream>
#include <spdlog/spdlog.h>

using namespace environment;
using namespace market::measure;

RiskManager::RiskManager(market::AskBook<>& ab, market::BidBook<>& bb,
                         std::pair<long, long> position_bounds,
                         int order_limit):
    ORDER_LIMIT(order_limit),
    POS_LB(position_bounds.first),
    POS_UB(position_bounds.second),

    position_(0),

    ask_book_(ab),
    bid_book_(bb)
{}

void RiskManager::CheckOrders()
{
    if (position_ >= POS_UB)
        bid_book_.CancelAllOrders();
    else if (position_ <= POS_LB)
        ask_book_.CancelAllOrders();
}

void RiskManager::Update(long executed_volume)
{
    position_ += executed_volume;

    CheckOrders();
}

long RiskManager::exposure()
{
    return position_;
}

bool RiskManager::at_upper()
{
    return position_ >= POS_UB;
}

bool RiskManager::at_lower()
{
    return position_ <= POS_LB;
}

bool RiskManager::at_bound()
{
    return at_upper() or at_lower();
}

bool RiskManager::PlaceOrder(market::Side side, double price, long size,
                             bool auto_cancel)
{
    switch (side) {
        case market::Side::ask: {
            int oc = ask_book_.order_count();
            if (oc < ORDER_LIMIT)
                return ask_book_.PlaceOrder(price, size);

            else if (oc > ORDER_LIMIT)
                throw runtime_error("Order count has exceeded ORDER_LIMIT!");

            else if (auto_cancel) {
                ask_book_.CancelWorst();
                ask_book_.PlaceOrder(price, size);
            }

            break;
        }

        case market::Side::bid: {
            int oc = bid_book_.order_count();
            if (oc < ORDER_LIMIT)
                return bid_book_.PlaceOrder(price, size);

            else if (oc > ORDER_LIMIT)
                throw runtime_error("Order count has exceeded ORDER_LIMIT!");

            else if (auto_cancel) {
                bid_book_.CancelWorst();
                bid_book_.PlaceOrder(price, size);
            }

            break;
        }
    }

    return false;
}

std::tuple<long, double, double> RiskManager::MarketOrder(long size)
{
    auto ret = market::BookUtils::MarketOrder(size, ask_book_, bid_book_);

    position_ += std::get<0>(ret);

    return ret;
}

std::tuple<long, double, double> RiskManager::ClearInventory()
{
    return this->MarketOrder(-position_);
}
