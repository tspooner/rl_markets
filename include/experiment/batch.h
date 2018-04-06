#ifndef EXPERIMENT_BATCH_H
#define EXPERIMENT_BATCH_H

#include <tuple>
#include <chrono>
#include <random>
#include <vector>

#include "rl/agent.h"
#include "rl/state.h"
#include "environment/base.h"

namespace experiment {
namespace batch {

class ExperienceReplay
{
    private:
        environment::Base& environment;

        size_t k, l;
        const size_t database_size, batch_size, trajectory_size;

        std::default_random_engine rng;

        std::vector<std::tuple<size_t, size_t, size_t>> db_id;
        std::vector<rl::State> db_state;
        std::vector<unsigned> db_action;
        std::vector<double> db_reward;

        size_t rand_idx();

    public:
        ExperienceReplay(Config& c, environment::Base& env, unsigned seed = 0);

        size_t n_collected();

        bool CollectTrajectory(rl::Agent *m);

        bool LearnFromSamples(rl::Agent *m);
        bool LearnFromTrajectories(rl::Agent *m);
};

}
}

#endif
