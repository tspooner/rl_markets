#include "environment/base.h"

#include "market/market.h"
#include "market/measures.h"
#include "market/target_price.h"

#include <utility>
#include <iostream>

using namespace std;
using namespace environment;
using namespace market::measure;

Base::Base(Config &c):
    ask_book_(),
    bid_book_(),

    risk_manager_(ask_book_, bid_book_,
                  std::make_pair(c["market"]["pos_lb"].as<long>(),
                                 c["market"]["pos_ub"].as<long>()),
                  1),

    TRANSACTION_FEE(c["market"]["TRANSACTION_FEE"].as<double>(0.0)),

    ORDER_SIZE(c["market"]["order_size"].as<int>(1)),

    // Reward space:
    r_pos_weight(c["reward"]["pos_weight"].as<float>(0.0)),
    r_trd_weight(c["reward"]["trd_weight"].as<float>(0.0)),
    r_pnl_weight(c["reward"]["pnl_weight"].as<float>(1.0)),

    r_damping_factor(c["reward"]["damping_factor"].as<float>(1.0)),

    // State
    f_vwap_numer(max(c["state"]["lookback"]["vwap"].as<int>(0), 1)),
    f_vwap_denom(max(c["state"]["lookback"]["vwap"].as<int>(0), 1)),

    f_midprice(max(c["state"]["lookback"]["mpm"].as<int>(0), 1)),
    f_volatility(max(c["state"]["lookback"]["vlt"].as<int>(0), 1)),

    f_ask_transactions(max(c["state"]["lookback"]["svl"].as<int>(0), 1)),
    f_bid_transactions(max(c["state"]["lookback"]["svl"].as<int>(0), 1)),

    spread_window(max(c["policy"]["spread_lookback"].as<int>(10), 1)),

    pnl_ups(max(c["reward"]["pnl_lookback"].as<int>(0), 1)),
    pnl_downs(max(c["reward"]["pnl_lookback"].as<int>(0), 1)),

    return_ups(max(c["state"]["lookback"]["rsi"].as<int>(0), 1)),
    return_downs(max(c["state"]["lookback"]["rsi"].as<int>(0), 1)),

    // Debugging:
    INSPECT_BOOKS(c["debug"]["inspect_books"].as<bool>(false))
{
    string rm = c["reward"]["measure"].as<string>("pnl");
    if (rm == "none")
        reward = RewardMeasure::none;
    else if (rm == "pnl")
        reward = RewardMeasure::pnl;
    else if (rm == "pnl_damped")
        reward = RewardMeasure::pnl_damped;
    else if (rm == "spread")
        reward = RewardMeasure::spread;
    else if (rm == "normed")
        reward = RewardMeasure::normed;
    else if (rm == "lovol")
        reward = RewardMeasure::lovol;
    else if (rm == "mm_linear")
        reward = RewardMeasure::mm_linear;
    else if (rm == "mm_exp")
        reward = RewardMeasure::mm_exp;
    else if (rm == "mm_div")
        reward = RewardMeasure::mm_div;
    else
        throw runtime_error("Unknown reward measure: " + rm);

    // Initialie latency sampler:
    string lt = c["market"]["latency"]["type"].as<string>("fixed");
    float lt_floor = c["market"]["latency"]["floor"].as<float>(0.0f);
    if (lt != "fixed") {
        unsigned seed = c["debug"]["random_seed"].as<unsigned>(random_device{}());
        float lt_mu = c["market"]["latency"]["mu"].as<float>(0.0f);

        if (lt == "normal")
            latency_ = new market::NormalLatency(lt_mu,
                                                 c["market"]["latency"]["sigma"].as<float>(),
                                                 seed,
                                                 lt_floor);
        else if (lt == "lognormal")
            latency_ = new market::LognormalLatency(lt_mu,
                                                    c["market"]["latency"]["beta"].as<float>(),
                                                    seed,
                                                    lt_floor);
        else
            throw invalid_argument("Unknown latency type: " + lt);
    }
    else
        latency_ = new market::Latency(lt_floor);

    // Initialise target pricing strategy:
    string tp = c["market"]["target_price"]["type"].as<string>("midprice");
    int tp_lb = c["market"]["target_price"]["lookback"].as<int>(1);
    if (tp != "midprice")
        target_price_ = new market::tp::MidPrice(tp_lb);
    else if (tp != "microprice")
        target_price_ = new market::tp::MicroPrice(tp_lb);
    else if (tp != "vwap")
        target_price_ = new market::tp::VWAP(tp_lb);
    else if (tp != "book")
        target_price_ = new market::tp::TargetPrice();
    else
        throw invalid_argument("Unknown target price type: " + tp);

    Base::Initialise();
}

Base::~Base()
{
    delete latency_;
    delete target_price_;
}

bool Base::Initialise()
{
    ask_quote = 0.0;
    bid_quote = 0.0;

    ask_book_.Reset();
    bid_book_.Reset();

    ClearStats();
    ClearWindows();

    return true;
}

void Base::ClearStats()
{
    episode_stats.reset();
    trade_stats.reset();
    tick_stats.reset();
}

void Base::ClearWindows()
{
    spread_window.clear();
    target_price_->clear();

    f_midprice.clear();
    f_volatility.clear();

    f_vwap_numer.clear();
    f_vwap_denom.clear();

    pnl_ups.clear();
    pnl_downs.clear();

    return_ups.clear();
    return_downs.clear();

    f_ask_transactions.clear();
    f_bid_transactions.clear();
}

// Learning ---------------------------------------------------------
double Base::getReward()
{
    double r = 0.0;
    int abs_pos = abs(risk_manager_.exposure());

    switch (reward) {
        case RewardMeasure::none:
            break;

        case RewardMeasure::pnl:
            r = pnl_step;
            break;

        case RewardMeasure::pnl_damped:
            r = pnl_step - r_damping_factor*max(0.0, momentum_pnl_step);
            break;

        case RewardMeasure::spread:
            r = pnl_step / spread_window.mean();
            break;

        case RewardMeasure::normed:
            if (not (pnl_ups.full() && pnl_downs.full()))
                r = 0.0;
            else {
                double u = pnl_ups.mean(),
                       d = pnl_downs.mean();
                double su = pnl_ups.std(),
                       sd = pnl_downs.std();

                double numer = (u*sd - d*su),
                       denom = (su + sd);

                if (::isnan(numer) || ::isinf(numer)) numer = 0.0;
                if (::isnan(denom) || ::isinf(denom)) denom = 0.0;

                r = (abs(denom) < 1e-5) ? numer : (numer / denom);
            }

            break;

        case RewardMeasure::lovol:
            r = lo_vol_step;
            break;

        case RewardMeasure::mm_linear:
            // Punish holding a position:
            r = -r_pos_weight * abs_pos;

            // Reward/punish profit/loss from trades:
            r += r_pnl_weight * pnl_step;

            break;

        case RewardMeasure::mm_exp:
            // Punish holding a position:
            r = -pow(1.0 - exp(r_pos_weight * abs_pos), 2);

            // Reward/punish profit/loss from trades:
            r += r_pnl_weight * pnl_step;

            break;

        case RewardMeasure::mm_div:
            if (pnl_step > 0)
                r = pnl_step / max(1.0, (double) abs_pos);
            else
                r = pnl_step;
    }

    return r*100;
}

double Base::getPotential()
{
    return 0.0;
}

double Base::getEpisodeReward()
{
    return episode_stats.reward;
}

double Base::getMeanEpisodeReward()
{
    return episode_stats.reward/tick_stats.total_ticks;
}

bool Base::performAction(int action)
{
    /* BLOCK 1 - Apply action */
    if (INSPECT_BOOKS)
        printInfo(action);

    last_action = action;
    lo_vol_step = 0L;

    pnl_step = 0.0;
    momentum_pnl_step = 0.0;

    // Given that the environment is first initialised we can assume
    // that we are currently in a valid state. This means that we should call
    // NextState() at the end of this method rather than the start.
    // Essentially performAction() leaves the environment in a ready state for
    // the next action.

    // Do agent's action -> risk manager
    DoAction(action);

    risk_manager_.CheckOrders();

    // Update occupancy statistics: ticks_with...
    UpdateStats();

    /* BLOCK 2 - Execute state transition */
    double agg_r = getReward();
    double agg_pnl = pnl_step;
    double agg_mpm = 0.0;
    long observed_volume = 0L;
    do {
        pnl_step = 0.0;

        /* Handle state transition */
        if (not NextState())
            return false;

        // Add in the reward for moving to the new state
        double mpm = midprice_move(ask_book_, bid_book_);
        pnl_step += risk_manager_.exposure() * mpm;
        momentum_pnl_step += risk_manager_.exposure() * mpm;

        agg_r += getReward();
        agg_pnl += pnl_step;
        agg_mpm += mpm;

        observed_volume += ask_book_.observed_volume() +
                           bid_book_.observed_volume();

    // } while (false);
    } while (not isTerminal() and abs(agg_mpm) < 1e-5);

    /* BLOCK 3 - Apply inventory constraints */
    // if (risk_manager_.at_bound()) {
        // pnl_step = 0.0;

        // ClearInventory();

        // agg_pnl += pnl_step;
        // agg_r += getReward();
    // }

    /* BLOCK 4 - Update statistics */
    pnl_step = agg_pnl;

    pnl_ups.push(max(0.0, pnl_step));
    pnl_downs.push(abs(min(0.0, pnl_step)));

    // Logging {
    episode_stats.reward += agg_r;
    experiment_stats.reward += agg_r;

    // episode_stats.pnl += agg_pnl;
    experiment_stats.pnl += agg_pnl;

    episode_stats.bandh += agg_mpm;
    experiment_stats.bandh += agg_mpm;

    LogProfit(action, pnl_step, agg_mpm);
    // }

    return true;
}

void Base::ClearInventory()
{
    auto out = risk_manager_.ClearInventory();
    pnl_step += get<1>(out);
    lo_vol_step += abs(get<0>(out));

    episode_stats.pnl += get<2>(out);

    if (get<0>(out) > 0) trade_stats.market_buys++;
    else if (get<0>(out) < 0) trade_stats.market_sells++;
}

void Base::printInfo(int action)
{
    if (action != -1)
        cout << "Chosen action: " << action << endl;
    cout << "Target Price: " << target_price_->get() << endl;
    cout << "Spread lookback: " << spread_window.mean() << endl << endl;

    cout << "Transacted volume (from last step): " << lo_vol_step << endl;
    cout << "Reward (from last step): " << getReward() << endl;
    cout << "Position (current step): " << risk_manager_.exposure() << endl << endl;

    cout << "Current PnL: " << episode_stats.pnl << endl << endl;

    cout << "Ask orders:" << endl;
    ask_book_.PrintOrders();

    cout << endl << "Bid orders:" << endl;
    bid_book_.PrintOrders();

    cout << endl << "The books:" << endl;

    cout << "Ask (ov == " << ask_book_.observed_volume() << ")" << endl;
    int d = ask_book_.depth();
    for (int i = d-1; i >= 0; i--) {
        double price = ask_book_.price(i);
        cout << "\tLevel " << i+1 << ": " << price << "\t"
             << ask_book_.volume(price) << endl;
    }
    cout << endl;

    cout << "\tMid price (-): " << midprice(ask_book_, bid_book_) << endl;
    cout << "\tMicro price (-): " << microprice(ask_book_, bid_book_) << endl << endl;

    cout << "Bid (ov == " << bid_book_.observed_volume() << ")" << endl;
    d = bid_book_.depth();
    for (int i = 0; i <= d-1; i++) {
        double price = bid_book_.price(i);
        cout << "\tLevel " << i+1 << ": " << price << "\t"
            << bid_book_.volume(price) << endl;
    }

    cout << "\n--Press any key to continue--\n" << flush;
    cin.sync();
    cin.get();
}

void Base::start_logging()
{
    profit_logger = spdlog::get("profit_log");
    trade_logger = spdlog::get("trade_log");

    if (profit_logger == nullptr or trade_logger == nullptr)
        throw runtime_error("Loggers not registered!");
}

void Base::stop_logging()
{
    profit_logger = nullptr;
    trade_logger = nullptr;
}

void Base::UpdateStats()
{
    // Episode stats:
    trade_stats.ask_transactions = ask_book_.n_transacted();
    trade_stats.bid_transactions = bid_book_.n_transacted();

    // Tick stats:
    tick_stats.total_ticks++;

    bool has_ask = ask_book_.order_count() > 0,
         has_bid = bid_book_.order_count() > 0;

    if (has_ask)
        tick_stats.ticks_with_ask++;

    if (has_bid)
        tick_stats.ticks_with_bid++;

    if (has_ask and has_bid)
        tick_stats.ticks_with_both++;

    long exposure = risk_manager_.exposure();

    if (exposure != 0)
        tick_stats.ticks_with_position++;

    if (exposure > 0)
        tick_stats.ticks_long++;
    else if (exposure < 0)
        tick_stats.ticks_short++;
}

void Base::resetStats()
{
    trade_stats.reset();
    tick_stats.reset();
    episode_stats.reset();
}

void Base::writeStats(string path)
{
    experiment_stats.write(path);
    tick_stats.write(path);
    trade_stats.write(path);
}

double Base::getEpisodePnL()
{
    return episode_stats.pnl;
}

float Base::getOrderRatio()
{
    return float(trade_stats.ask_transactions + trade_stats.bid_transactions) /
        (trade_stats.market_buys + trade_stats.market_sells);
}

int Base::getTotalTransactions()
{
    return trade_stats.ask_transactions + trade_stats.bid_transactions +
        trade_stats.market_buys + trade_stats.market_sells;
}
