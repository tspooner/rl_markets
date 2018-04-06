#ifndef MARKET_TARGET_PRICE_H
#define MARKET_TARGET_PRICE_H

#include "market/book.h"
#include "utilities/accumulators.h"


namespace market {
namespace tp {

class TargetPrice
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

class MidPrice: public TargetPrice
{
    private:
        RollingMean<double> mp_;

    public:
        MidPrice(int lookback);

        bool ready();
        void update(market::AskBook<>& ab, market::BidBook<>& bb);
        void clear();
};

class MicroPrice: public TargetPrice
{
    private:
        RollingMean<double> mp_;

    public:
        MicroPrice(int lookback);

        bool ready();
        void update(market::AskBook<>& ab, market::BidBook<>& bb);
        void clear();
};

class VWAP: public TargetPrice
{
    private:
        Accumulator<double> numerator_,
                            denominator_;

    public:
        VWAP(int lookback);

        bool ready();
        void update(market::AskBook<>& ab, market::BidBook<>& bb);
        void clear();
};

}
}

#endif
