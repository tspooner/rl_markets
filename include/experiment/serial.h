#ifndef EXPERIMENT_SERIAL_H
#define EXPERIMENT_SERIAL_H

#include <string>
#include <atomic>

#include "rl/agent.h"
#include "rl/state.h"
#include "environment/base.h"

namespace experiment {
namespace serial {

class Runner
{
    protected:
        environment::Base& environment;

        rl::State state1;
        rl::State state2;

        rl::State* state;
        rl::State* last_state;

        // Returns true if terminal
        virtual bool _step(rl::Agent *m) = 0;

    public:
        Runner(Config& c, environment::Base& env);

        virtual bool RunEpisode(rl::Agent *m);
};

class Learner: public Runner
{
    protected:
        double last_ask_quote = 0.0;
        double last_bid_quote = 0.0;

        unsigned long _step_counter = 0;
        static std::atomic_int _episode_counter;

        bool _step(rl::Agent *m);

    public:
        Learner(Config& c, environment::Base& env);

        bool RunEpisode(rl::Agent *m);
};

class Backtester: public Runner
{
    private:
        bool _step(rl::Agent *m);

    public:
        Backtester(Config& c, environment::Base& env);
};

}
}

#endif
