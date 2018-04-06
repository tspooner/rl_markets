#include "market/book.h"

#include "market/measures.h"
#include "utilities/memory.h"
#include "utilities/comparison.h"

#include <tuple>
#include <string>
#include <memory>
#include <iomanip>
#include <iostream>
#include <algorithm>

using namespace market;
using namespace market::measure;

template<typename C, size_t DEPTH>
Book<C, DEPTH>::Book():
    price_comparator_(),

    open_orders(price_comparator_),

    prices(),
    last_prices(),

    levels(price_comparator_),
    last_levels(price_comparator_),

    total_volume_(0L),
    last_total_volume_(0L),

    observed_transaction_value_(0.0),
    observed_transaction_volume_(0L)
{
    Reset();
}

template<typename C, size_t DEPTH>
typename Book<C, DEPTH>::OMI Book<C, DEPTH>::GetOrder(double price)
{
    return open_orders.find(price);
}

template<typename C, size_t DEPTH>
bool Book<C, DEPTH>::OrderExists(Book<C, DEPTH>::OMI it)
{
    return (it != open_orders.end());
}

template<typename C, size_t DEPTH>
void Book<C, DEPTH>::StashState()
{
    prices.swap(last_prices);
    levels.swap(last_levels);
}

template<typename C, size_t DEPTH>
bool Book<C, DEPTH>::HasStash()
{
    return not approx_equal(last_prices[0], 0.0);
}

template<typename C, size_t DEPTH>
void Book<C, DEPTH>::ApplyChanges(
    const std::array<double, DEPTH>& new_prices,
    const std::array<long, DEPTH>& new_volumes,
    const std::map<double, long, FloatComparator<>>& transactions)
{
    levels.clear();

    last_total_volume_ = total_volume_;

    for (unsigned int l = 0; l < DEPTH; l++) {
        if (new_prices[l] <= 0.0)
            throw runtime_error("Prices must be non-zero positive: " + to_string(new_prices[l]));
        else if (new_volumes[l] <= 0)
            throw runtime_error("Volumes must be non-zero positive: " + to_string(new_prices[l]));
        else {
            prices[l] = new_prices[l];
            levels[new_prices[l]] = new_volumes[l];
            total_volume_ += new_volumes[l];
        }
    }

    std::sort(prices.begin(), prices.end(), price_comparator_);

    // Iterate over all agent orders and update their queues
    auto it = open_orders.begin();
    auto it_t_end = transactions.end();
    while (true) {
        if (it == open_orders.end())
            break;

        auto it_t = transactions.find(it->first);
        long tv = (it_t == it_t_end) ? 0L : it_t->second;

        it = UpdateOrder(it, tv);
    }
}

template<typename C, size_t DEPTH>
typename Book<C, DEPTH>::OMI Book<C, DEPTH>::UpdateOrder(Book<C, DEPTH>::OMI it,
                                                         long transaction_volume)
{
    double price = it->first;
    market::OrderPtr& o = it->second;

    if (o->isExecuted())
        return open_orders.erase(it);

    long lv = last_volume(price);
    if (lv == 0) {
        // Was previously at undefined level, so the last volume should be
        // zero. In this case we don't need to deal with cancellations.
        ++it;
        return it;
    }

    long v = volume(price);
    if (v == 0) {
        // There are no other orders at that price (from data)
        // Order was buried and defined it's own new price in the book:
        o->clearQueues(); // Zero-out queues
        ++it;

        return it;
    }

    long vol_diff = lv - v;
    if (vol_diff >= 0) {
        long cancelled_volume = vol_diff - transaction_volume;

        if (cancelled_volume > 0)
            o->doCancellation(cancelled_volume);

    } else
        o->addVolumeBehind(vol_diff);

    ++it;
    return it;
}

template<typename C, size_t DEPTH>
void Book<C, DEPTH>::Reset()
{
    n_transacted_ = 0;
    observed_transaction_value_ = 0.0;
    observed_transaction_volume_ = 0L;

    total_volume_ = 0L;
    last_total_volume_ = 0L;

    prices.fill(0.0);
    last_prices.fill(0.0);

    levels.clear();
    last_levels.clear();

    open_orders.clear();
}

template<typename C, size_t DEPTH>
int Book<C, DEPTH>::depth() { return DEPTH; }

// Price/volume getters ---------------------------------------------
template<typename C, size_t DEPTH>
double Book<C, DEPTH>::price(int level)
{
    if (level < 0) level = DEPTH+level;

    if ((unsigned int) level >= DEPTH or level < 0 or prices[level] == 0.0)
        throw runtime_error("Attempted to access an undefined price at level: " +
                            to_string(level));
    else
        return prices[level];
}

template<typename C, size_t DEPTH>
double Book<C, DEPTH>::last_price(int level)
{
    if (level < 0) level = DEPTH+level;

    if ((unsigned int) level >= DEPTH or level < 0 or last_prices[level] == 0.0)
        throw runtime_error("Attempted to access undefined last price at level: " +
                            to_string(level));
    else
        return last_prices[level];
}

template<typename C, size_t DEPTH>
int Book<C, DEPTH>::price_level(double target_price)
{
    for (unsigned int l = 0; l < DEPTH; l++)
        if (approx_equal(price(l), target_price)) return l;

    return -1;
}

template<typename C, size_t DEPTH>
int Book<C, DEPTH>::last_price_level(double target_price)
{
    for (unsigned int l = 0; l < DEPTH; l++)
        if (approx_equal(last_price(l), target_price)) return l;

    return -1;
}

template<typename C, size_t DEPTH>
long Book<C, DEPTH>::volume(double price)
{
    auto it = levels.find(price);

    return (it == levels.end()) ? 0L : it->second;
}

template<typename C, size_t DEPTH>
long Book<C, DEPTH>::last_volume(double price)
{
    auto it = last_levels.find(price);

    return (it == last_levels.end()) ? 0L : it->second;
}

template<typename C, size_t DEPTH>
long Book<C, DEPTH>::total_volume()
{
    return total_volume_;
}

template<typename C, size_t DEPTH>
long Book<C, DEPTH>::last_total_volume()
{
    return last_total_volume_;
}

template<typename C, size_t DEPTH>
int Book<C, DEPTH>::n_transacted()
{ return n_transacted_; }

template<typename C, size_t DEPTH>
double Book<C, DEPTH>::observed_value()
{ return observed_transaction_value_; }

template<typename C, size_t DEPTH>
long Book<C, DEPTH>::observed_volume()
{ return observed_transaction_volume_; }

// Order methods ----------------------------------------------------
template<typename C, size_t DEPTH>
bool Book<C, DEPTH>::PlaceOrder(double price, long size)
{
    auto it = GetOrder(price);

    if (OrderExists(it))
        return false;
    else {
        open_orders.emplace(price, make_unique<Order>(price, size, volume(price)));

        return true;
    }
}

template<typename C, size_t DEPTH>
bool Book<C, DEPTH>::PlaceOrderAtLevel(int level, long size)
{
    return PlaceOrder(price(level), size);
}

template<typename C, size_t DEPTH>
bool Book<C, DEPTH>::HasOpenOrder(double price)
{
    auto it = GetOrder(price);

    return (OrderExists(it) and !it->second->isExecuted());
}

template<typename C, size_t DEPTH>
void Book<C, DEPTH>::CancelOrder(double price)
{
    open_orders.erase(price);
}

template<typename C, size_t DEPTH>
void Book<C, DEPTH>::CancelBest()
{
    open_orders.erase(open_orders.begin());
}

template<typename C, size_t DEPTH>
void Book<C, DEPTH>::CancelWorst()
{
    open_orders.erase(--open_orders.rbegin().base());
}

template<typename C, size_t DEPTH>
void Book<C, DEPTH>::CancelAllOrders()
{
    open_orders.clear();
}

template<typename C, size_t DEPTH>
int Book<C, DEPTH>::order_count()
{
    return open_orders.size();
}

template<typename C, size_t DEPTH>
double Book<C, DEPTH>::best_open_order_price()
{
    return open_orders.begin()->first;
}

template<typename C, size_t DEPTH>
double Book<C, DEPTH>::worst_open_order_price()
{
    return open_orders.rbegin()->first;
}

template<typename C, size_t DEPTH>
long Book<C, DEPTH>::order_size(double price)
{
    auto it = GetOrder(price);

    return OrderExists(it) ? it->second->size : -1;
}

template<typename C, size_t DEPTH>
long Book<C, DEPTH>::order_remaining_volume(double price)
{
    auto it = GetOrder(price);

    return OrderExists(it) ? it->second->remaining() : -1;
}

template<typename C, size_t DEPTH>
long Book<C, DEPTH>::queue_ahead(double price)
{
    auto it = GetOrder(price);

    return OrderExists(it) ? it->second->getQueueAhead() : -1;
}

template<typename C, size_t DEPTH>
long Book<C, DEPTH>::queue_behind(double price)
{
    auto it = GetOrder(price);

    return OrderExists(it) ? it->second->getQueueBehind() : -1;
}

template<typename C, size_t DEPTH>
long Book<C, DEPTH>::queue_progress()
{
    auto it = open_orders.begin();

    return OrderExists(it) ? it->second->getQueueProgress() : -1;
}

template<typename C, size_t DEPTH>
void Book<C, DEPTH>::PrintOrders()
{
    for (const auto& kv : open_orders) {
        std::cout << kv.second->ToString() << std::endl;
    }
}

// Template specialisations
template class market::Book<FloatComparator<>, 1>;
template class market::Book<FloatComparator<>, 2>;
template class market::Book<FloatComparator<>, 3>;
template class market::Book<FloatComparator<>, 4>;
template class market::Book<FloatComparator<>, 5>;

template class market::Book<ReverseFloatComparator<>, 1>;
template class market::Book<ReverseFloatComparator<>, 2>;
template class market::Book<ReverseFloatComparator<>, 3>;
template class market::Book<ReverseFloatComparator<>, 4>;
template class market::Book<ReverseFloatComparator<>, 5>;

// ----------------
// Ask book
template<size_t DEPTH>
std::tuple<long, double, double>
    AskBook<DEPTH>::ApplyTransactions(
        const std::map<double, long, FloatComparator<>>& transactions,
        const double reference_price
    )
{
    auto o_it = this->open_orders.begin();

    this->observed_transaction_value_ = 0.0;
    this->observed_transaction_volume_ = 0L;

    long volume = 0L;
    double proxy = 0.0;
    double value = 0.0;

    for (auto it = transactions.begin(); it != transactions.end(); ++it) {
        if (it->first < reference_price) continue;

        long vol = it->second;

        this->observed_transaction_value_ += it->first * vol;
        this->observed_transaction_volume_ += vol;

        while (o_it != this->open_orders.end() and o_it->first <= it->first) {
            long _order_rem = o_it->second->remaining();

            vol = o_it->second->doTransaction(vol);
            long order_exec = _order_rem - o_it->second->remaining();

            volume -= order_exec;
            proxy += (o_it->first - reference_price) * order_exec;
            value += o_it->first * order_exec;

            if (o_it->second->isExecuted()) {
                // Order executed:
                o_it = this->open_orders.erase(o_it);
                this->n_transacted_++;
            }

            if (vol <= 0) break;
        }
    }

    return std::make_tuple(volume, proxy, value);
}

template<size_t DEPTH>
std::tuple<long, double, double>
    AskBook<DEPTH>::WalkTheBook(double reference_price, long size)
{
    long abs_size = abs(size);

    if (abs_size > this->total_volume())
        return std::make_tuple(0L, 0.0, 0.0);

    long executed = 0L;
    double proxy = 0.0;
    double value = 0.0;
    for (auto const& kv : this->levels) {
        long lvol = kv.second,
             l_ex = min(lvol, (abs_size - executed));

        executed += l_ex;
        proxy -= l_ex * fabs(kv.first - reference_price);
        value -= l_ex * kv.first;

        if (executed >= abs_size) {
            this->n_transacted_++;
            break;
        }
    }

    return std::make_tuple(executed, proxy, value);
}

// Template specialisations
template struct market::AskBook<1>;
template struct market::AskBook<2>;
template struct market::AskBook<3>;
template struct market::AskBook<4>;
template struct market::AskBook<5>;

// ----------------
// Bid book
template<size_t DEPTH>
std::tuple<long, double, double> BidBook<DEPTH>::ApplyTransactions(
    const std::map<double, long, FloatComparator<>>& transactions,
    const double reference_price)
{
    auto o_it = this->open_orders.rbegin();

    this->observed_transaction_value_ = 0.0;
    this->observed_transaction_volume_ = 0L;

    long volume = 0L;
    double proxy = 0.0;
    double value = 0.0;

    for (auto it = transactions.rbegin(); it != transactions.rend(); ++it) {
        if (it->first > reference_price) continue;

        long vol = it->second;

        this->observed_transaction_value_ += it->first * vol;
        this->observed_transaction_volume_ += vol;

        while (o_it != this->open_orders.rend() and o_it->first >= it->first) {
            long _order_rem = o_it->second->remaining();

            vol = o_it->second->doTransaction(vol);
            long order_exec = _order_rem - o_it->second->remaining();

            volume += order_exec;
            proxy += (reference_price - o_it->first) * order_exec;
            value -= o_it->first * order_exec;

            if (o_it->second->isExecuted()) {
                // Order executed:
                this->open_orders.erase(--(o_it.base()));
                this->n_transacted_++;
            }

            if (vol <= 0) break;
        }
    }

    return std::make_tuple(volume, proxy, value);
}

template<size_t DEPTH>
std::tuple<long, double, double>
    BidBook<DEPTH>::WalkTheBook(double reference_price, long size)
{
    long abs_size = abs(size);

    if (abs_size > this->total_volume())
        return std::make_tuple(0L, 0.0, 0.0);

    long executed = 0L;
    double proxy = 0.0;
    double value = 0.0;
    for (auto const& kv : this->levels) {
        long lvol = kv.second,
             l_ex = min(lvol, (abs_size - executed));

        executed += l_ex;
        proxy -= l_ex * fabs(kv.first - reference_price);
        value += l_ex * kv.first;

        if (executed >= abs_size) {
            this->n_transacted_++;
            break;
        }
    }

    return std::make_tuple(-executed, proxy, value);
}

// Template specialisations
template struct market::BidBook<1>;
template struct market::BidBook<2>;
template struct market::BidBook<3>;
template struct market::BidBook<4>;
template struct market::BidBook<5>;

// ----------------
// Book Utils
template<size_t DEPTH>
std::tuple<long, double, double> BookUtils::HandleAdverseSelection(
    AskBook<DEPTH>& ask_book, BidBook<DEPTH>& bid_book)
{
    // TODO: Improve this logic to handle orders > adverse level.
    const double bap = ask_book.price(0),
                 bbp = bid_book.price(0),
                 rp  = last_midprice(ask_book, bid_book);

    long volume = 0L;
    double proxy = 0.0;
    double value = 0.0;

    auto it_a = ask_book.open_orders.begin();
    while (it_a != ask_book.open_orders.end() and it_a->first <= bbp) {
        long rem = it_a->second->remaining();

        volume -= rem;
        proxy += rem * (it_a->first - rp);
        value += rem * it_a->first;

        it_a->second->doTransaction(rem);

        it_a = ask_book.open_orders.erase(it_a);
        ask_book.n_transacted_++;
    }

    auto it_b = bid_book.open_orders.begin();
    while (it_b != bid_book.open_orders.end() and it_b->first >= bap) {
        long rem = it_b->second->remaining();

        volume += rem;
        proxy += rem * (rp - it_b->first);
        value -= rem * it_b->first;

        it_b->second->doTransaction(rem);

        it_b = bid_book.open_orders.erase(it_b);
        bid_book.n_transacted_++;
    }

    return std::make_tuple(volume, proxy, value);
}

template<size_t DEPTH>
std::tuple<long, double, double>
    BookUtils::MarketOrder(
        long size, AskBook<DEPTH>& ask_book, BidBook<DEPTH>& bid_book)
{
    double mip = midprice(ask_book, bid_book);

    if (size == 0L)
        return std::make_tuple(0L, 0.0, 0.0);

    else if (size > 0)
        // Market buy -> Walk ask book:
        return ask_book.WalkTheBook(mip, size);
    else
        // Market sell -> Walk the bid book:
        return bid_book.WalkTheBook(mip, size);
}

template<size_t DEPTH>
bool BookUtils::IsValidState(AskBook<DEPTH>& ask_book,
                             BidBook<DEPTH>& bid_book)
{
    double mp = midprice(ask_book, bid_book);

    if (ask_book.HasStash() and bid_book.HasStash())
        return
            (spread(ask_book, bid_book) >= 0.0)
            and (mp > 0.0)
            and (fabs(midprice_move(ask_book, bid_book)) < mp);
    else
        return true;
}

// Template specialisations
template std::tuple<long, double, double>
    BookUtils::HandleAdverseSelection<5>(AskBook<5>&, BidBook<5>&);
template std::tuple<long, double, double>
    BookUtils::MarketOrder<5>(long, AskBook<5>&, BidBook<5>&);
template bool BookUtils::IsValidState<5>(AskBook<5>&, BidBook<5>&);
