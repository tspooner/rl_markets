#ifndef MARKET_MEASURES_H
#define MARKET_MEASURES_H

#include "book.h"

namespace market {
namespace measure {

inline double spread(AskBook<>& ask_book, BidBook<>& bid_book)
{
    return ask_book.price(0) - bid_book.price(0);
}

inline double last_spread(AskBook<>& ask_book, BidBook<>& bid_book)
{
    return ask_book.last_price(0) - bid_book.last_price(0);
}

inline double spread_move(AskBook<>& ask_book, BidBook<>& bid_book)
{
    return spread(ask_book, bid_book) - last_spread(ask_book, bid_book);
}

inline double midprice(AskBook<>& ask_book, BidBook<>& bid_book)
{
    return (ask_book.price(0) + bid_book.price(0)) / 2.0f;
}

inline double last_midprice(AskBook<>& ask_book, BidBook<>& bid_book)
{
    return (ask_book.last_price(0) + bid_book.last_price(0)) / 2.0f;
}

inline double midprice_move(AskBook<>& ask_book, BidBook<>& bid_book)
{
    return midprice(ask_book, bid_book) - last_midprice(ask_book, bid_book);
}

inline double microprice(AskBook<>& ask_book, BidBook<>& bid_book)
{
    // See: Todd, A.; Hayes, R.; Beling, P.; Scherer, W.,
    //      "Micro-price trading in an order-driven market,"
    double ap = ask_book.price(0),
           bp = bid_book.price(0);
    long av = ask_book.total_volume(),
         bv = bid_book.total_volume();

    double div = (double) (av + bv);
    double mpm_a = av * bp;
    double mpm_b = ap * bv;

    return (mpm_a + mpm_b) / div;
}

inline double last_microprice(AskBook<>& ask_book, BidBook<>& bid_book)
{
    // See: Todd, A.; Hayes, R.; Beling, P.; Scherer, W.,
    //      "Micro-price trading in an order-driven market,"
    double ap = ask_book.price(0),
           bp = bid_book.price(0);
    long av = ask_book.last_total_volume(),
         bv = bid_book.last_total_volume();

    double div = (double) (av + bv);
    double mpm_a = av * bp;
    double mpm_b = ap * bv;

    return (mpm_a + mpm_b) / div;
}

inline double microprice_move(AskBook<>& ask_book, BidBook<>& bid_book)
{
    return microprice(ask_book, bid_book) -
        last_microprice(ask_book, bid_book);
}

}
}

#endif
