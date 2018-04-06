#ifndef ENVIRONMENT_BASE_H
#define ENVIRONMENT_BASE_H

#include <map>
#include <list>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <spdlog/spdlog.h>

#include "market/book.h"
#include "market/order.h"
#include "market/latency.h"
#include "market/target_price.h"
#include "utilities/config.h"
#include "utilities/accumulators.h"
#include "environment/statistics.h"
#include "environment/risk_manager.h"

namespace environment {

enum class RewardMeasure
{
    none,
    pnl,
    pnl_damped,
    spread,
    normed,
    lovol,
    mm_linear,
    mm_exp,
    mm_div
};

class Base
{
    protected:
        // Order books
        market::AskBook<> ask_book_;
        market::BidBook<> bid_book_;

        // Orders management
        RiskManager risk_manager_;

        // Frictions
        market::Latency* latency_;
        const double TRANSACTION_FEE;

        // Agent
        market::tp::TargetPrice* target_price_;

        int last_action = 0;
        int lo_vol_step = 0;

        double pnl_step = 0.0;
        double momentum_pnl_step = 0.0;

        const int ORDER_SIZE;

        double ask_quote = 0.0;
        double bid_quote = 0.0;

        // Rewards
        RewardMeasure reward;
        const float r_pos_weight;
        const float r_trd_weight;
        const float r_pnl_weight;
        const float r_damping_factor;

        // Windowed components
        Accumulator<double> f_vwap_numer;
        Accumulator<double> f_vwap_denom;

        RollingMean<double> f_midprice;
        RollingMean<double> f_volatility;
        RollingMean<double> f_ask_transactions;
        RollingMean<double> f_bid_transactions;

        RollingMean<double> spread_window;
        RollingMean<double> pnl_ups, pnl_downs;

        EWMA<double> return_ups, return_downs;

        // Inspection
        const bool INSPECT_BOOKS;

        // Statistics
        ExperimentStatistics episode_stats;

        ExperimentStatistics experiment_stats;
        TradeStatistics trade_stats;
        TickStatistics tick_stats;

        // Logging
        shared_ptr<spdlog::logger> profit_logger = nullptr;
        shared_ptr<spdlog::logger> trade_logger = nullptr;

    protected:
        // Interaction functions:
        virtual void DoAction(int action) = 0;

        // State transition handlers:
        virtual bool NextState() = 0;

        // Logging & stats:
        void UpdateStats();
        void ClearStats();

        void ClearWindows();

        virtual void LogProfit(int action, double pnl, double bandh) = 0;
        virtual void LogTrade(char side, char type, double price,
                              long size, double pnl) = 0;

    public:
        Base(Config &c);
        ~Base();

        // Initialisation
        virtual bool Initialise();

        // Learning
        virtual void getState(vector<float>& out) = 0;
        double getReward();
        double getPotential();

        double getEpisodeReward();
        double getMeanEpisodeReward();

        virtual bool performAction(int action);
        void ClearInventory();

        virtual bool isTerminal() = 0;
        virtual string getEpisodeId() = 0;

        // Logging
        void start_logging();
        void stop_logging();

        double getEpisodePnL();
        float getOrderRatio();
        int getTotalTransactions();

        // IO
        virtual void printInfo(const int action = -1);

        void resetStats();
        void writeStats(string path);
};

}

#endif
