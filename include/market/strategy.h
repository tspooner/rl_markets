#ifndef MARKET_STRATEGY_H
#define MARKET_STRATEGY_H

#include "market/book.h"
#include "utilities/accumulators.h"


namespace market {
namespace strategy {

class Strategy
{
    protected:
        double val_;

    public:
        TargetPrice();
        virtual ~TargetPrice() = default;

        double get();

        virtual bool ready();
        virtual void update(market::AskBook<>& ab, market::BidBook<>& bb);
        virtual void clear();
};

}
}
