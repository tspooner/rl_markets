#ifndef ENVIRONMENT_RISK_MANAGER_H
#define ENVIRONMENT_RISK_MANAGER_H

#include "market/book.h"
#include "market/order.h"

#include <map>
#include <tuple>
#include <memory>
#include <utility>
#include <spdlog/spdlog.h>

namespace environment {

class RiskManager
{
    private:
        const int ORDER_LIMIT;
        const long POS_LB;
        const long POS_UB;

        long position_;

        market::AskBook<>& ask_book_;
        market::BidBook<>& bid_book_;

    public:
        RiskManager(market::AskBook<>& ab, market::BidBook<>& bb,
                    std::pair<long, long> position_bounds = {-LONG_MAX, LONG_MAX},
                    int order_limit = INT_MAX);

        void CheckOrders();
        void Update(long executed_volume);

        long exposure();

        bool at_upper();
        bool at_lower();
        bool at_bound();

        bool PlaceOrder(market::Side side, double price, long size,
                        bool auto_cancel = true);

        std::tuple<long, double, double> ClearInventory();
        std::tuple<long, double, double> MarketOrder(long size);
        void ExitMarket();
};

}

#endif
