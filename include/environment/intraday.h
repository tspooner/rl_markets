#ifndef ENVIRONMENT_INTRADAY_H
#define ENVIRONMENT_INTRADAY_H

#include "data/basic.h"
#include "market/market.h"
#include "environment/base.h"
#include "utilities/comparison.h"

#include <map>
#include <tuple>
#include <vector>
#include <functional>


namespace environment {

enum class Variable {
    pos, spd, mpm,
    imb, svl, vol,
    rsi, vwap,
    a_dist, a_queue,
    b_dist, b_queue,
    last_action
};

template<class T1 = data::basic::MarketDepth,
         class T2 = data::basic::TimeAndSales>
class Intraday: public Base
{
    protected:
        T1 market_depth;
        T2 time_and_sales;

        // Real market parameters
        market::Market* market = nullptr;

        list<Variable> state_vars;
        const static map<string, Variable> v_to_i;


        int last_date = 0;
        int init_date = 0;      // Used for logging...

        long ref_time;

        int ask_level = 0;
        int bid_level = 0;

        void DoAction(int action);
        bool NextState();
        bool UpdateBookProfiles(
            const std::map<double, long, FloatComparator<>>& transactions = {});

        std::function<std::tuple<double, double>(int, int)> l2p_;
        void _place_orders(int sp, int sk, bool replace=true);

        void LogProfit(int action, double pnl, double bandh);
        void LogTrade(char side, char type, double price,
                      long size, double pnl);

    public:
        Intraday(Config& c);
        Intraday(Config& c, string symbol, string md_path, string tas_path);
        ~Intraday();

        bool Initialise();
        void LoadData(string symbol, string md_path, string tas_path);

        double getVariable(Variable v);
        void getState(vector<float>& out);

        bool isTerminal();
        string getEpisodeId();

        void printInfo(const int action = -1);
};

}

#endif
