#include "experiment/serial.h"

#include <iostream>
#include <algorithm>
#include <spdlog/spdlog.h>

using namespace experiment::serial;

Runner::Runner(Config& c, environment::Base& env):
    environment(env),

    state1(c), state2(c)
{
    state = &state1;
    last_state = &state2;
}

bool Runner::RunEpisode(rl::Agent *m)
{
    // Prepare environment for next episode
    if (not environment.Initialise())
        return false;

    // Initialise starting state
    last_state->newState(environment);

    bool is_terminal;
    do { is_terminal = _step(m); }
    while (not is_terminal);

    environment.ClearInventory();

    return true;
}

std::atomic_int Learner::_episode_counter{0};

Learner::Learner(Config &c, environment::Base& env):
    Runner(c, env)
{
    // Register loggers for training:
    if (c["logging"] and c["logging"]["log_learning"].as<bool>()) {
        try {
            auto log = spdlog::rotating_logger_mt("training_log",
                                                  c["output_dir"].as<string>() + "training_log.csv",
                                                  c["logging"]["max_size"].as<size_t>(), 1);

            log->info("episode,episode_id,reward,pnl,n_steps,epsilon");
        } catch (spdlog::spdlog_ex& e) {}
    }
}

bool Learner::_step(rl::Agent *m)
{
    swap(state, last_state);

    if (environment.isTerminal())
        return true; // Terminal condition

    int action = m->action(*last_state);
    if (not environment.performAction(action))
        return true;

    state->newState(environment);
    m->HandleTransition(*last_state, action, environment.getReward(), *state);

    _step_counter++;

    return false;
}

bool Learner::RunEpisode(rl::Agent *m)
{
    _step_counter = 0;
    environment.resetStats();

    if (Runner::RunEpisode(m))
    {
        m->HandleTerminal(_episode_counter++);

        spdlog::get("training_log")
            ->info("{},{},{},{},{},{}",
                   _episode_counter,
                   environment.getEpisodeId(),
                   environment.getEpisodeReward(),
                   environment.getEpisodePnL(),
                   _step_counter,
                   m->policy->descr());

        return true;
    }

    return false;
}


Backtester::Backtester(Config &c, environment::Base& env):
    Runner(c, env)
{
    // Register loggers for environment:
    if (c["logging"] and c["logging"]["log_backtest"].as<bool>()) {
        try {
            spdlog::rotating_logger_mt("profit_log",
                                       c["output_dir"].as<string>() + "profit_log.csv",
                                       c["logging"]["max_size"].as<size_t>(), 1);
            spdlog::get("profit_log")
                ->info("episode,step,action,position,midprice,spread,quoted_ask,quoted_bid,ask_level,bid_level,pnl_step,bandh_step");

        } catch(const std::exception& e) {}

        try {
            spdlog::rotating_logger_mt("trade_log",
                                       c["output_dir"].as<string>() + "order_log.csv",
                                       c["logging"]["max_size"].as<size_t>(), 1);
            spdlog::get("trade_log")->
                info("episode,step,position,side,action,price,size,pnl");

        } catch(const std::exception& e) {}

        env.start_logging();
    }
}

bool Backtester::_step(rl::Agent *m)
{
    if (environment.isTerminal())
        return true; // Terminal condition

    state->newState(environment);

    int action = m->action(*state);

    if (not environment.performAction(action))
        return true; // Action failed to execute (no data, etc)...

    return false;
}
