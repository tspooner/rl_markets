#ifndef MARKET_BOOK_H
#define MARKET_BOOK_H

#include <map>
#include <cmath>
#include <array>
#include <tuple>
#include <utility>

#include "market/order.h"
#include "utilities/comparison.h"

using namespace std;


namespace market {

enum class Side {
    ask, bid
};

template<typename C, size_t DEPTH>
class Book
{
    friend class BookUtils;

    private:
        typedef typename std::map<double, OrderPtr, C> OrderMap;
        typedef typename OrderMap::iterator OMI;

        C price_comparator_;

    protected:
        OrderMap open_orders;

        array<double, DEPTH> prices;
        array<double, DEPTH> last_prices;

        map<double, long, C> levels;
        map<double, long, C> last_levels;

        long total_volume_;
        long last_total_volume_;

        int n_transacted_;

        double observed_transaction_value_;
        long observed_transaction_volume_;

    protected:
        Book();

        inline OMI GetOrder(double price);
        inline bool OrderExists(OMI it);
        OMI UpdateOrder(OMI it, long transaction_volume);

    public:
        void StashState();
        bool HasStash();

        void ApplyChanges(
            const std::array<double, DEPTH>& new_prices,
            const std::array<long, DEPTH>& new_volumes,
            const std::map<double, long, FloatComparator<>>& transactions={});

        void Reset();

        // Price/volume getters
        int depth();

        double price(int level);
        double last_price(int level);

        int price_level(double price);
        int last_price_level(double price);

        long volume(double price);
        long last_volume(double price);

        long total_volume();
        long last_total_volume();

        int n_transacted();
        double observed_value();
        long observed_volume();

        // Order methods
        bool PlaceOrder(double price, long size);
        bool PlaceOrderAtLevel(int level, long size);

        bool HasOpenOrder(double price);
        void CancelOrder(double price);

        void CancelBest();
        void CancelWorst();
        void CancelAllOrders();

        int order_count();
        double best_open_order_price();
        double worst_open_order_price();

        long order_size(double price);
        long order_remaining_volume(double price);

        long queue_ahead(double price);
        long queue_behind(double price);
        long queue_progress();

        void PrintOrders();
};

// ----------------

template<size_t DEPTH = 5>
struct AskBook: public Book<FloatComparator<>, DEPTH>
{
    std::tuple<long, double, double>
        ApplyTransactions(
            const std::map<double, long, FloatComparator<>>& transactions,
            const double reference_price);

    std::tuple<long, double, double>
        WalkTheBook(double reference_price, long size);
};

template<size_t DEPTH = 5>
struct BidBook: public Book<ReverseFloatComparator<>, DEPTH>
{
    std::tuple<long, double, double>
        ApplyTransactions(
            const std::map<double, long, FloatComparator<>>& transactions,
            const double reference_price);

    std::tuple<long, double, double>
        WalkTheBook(double reference_price, long size);
};

// ----------------

class BookUtils
{
    public:
        template<size_t DEPTH>
        static std::tuple<long, double, double>
            HandleAdverseSelection(
                AskBook<DEPTH>& ask_book,
                BidBook<DEPTH>& bid_book);

        template<size_t DEPTH>
        static std::tuple<long, double, double>
            MarketOrder(
                long size,
                AskBook<DEPTH>& ask_book,
                BidBook<DEPTH>& bid_book);

        template<size_t DEPTH>
        static bool IsValidState(AskBook<DEPTH>& ask_book,
                                 BidBook<DEPTH>& bid_book);
};

}

#endif
