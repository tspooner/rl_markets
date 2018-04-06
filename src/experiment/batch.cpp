#include "experiment/batch.h"
#include "rl/agent.h"
#include "rl/state.h"

#include <random>
#include <utility>
#include <iostream>

using namespace experiment::batch;
using namespace std;

ExperienceReplay::ExperienceReplay(Config &c, environment::Base& env,
                                   unsigned seed):
    environment(env),

    k(0),
    l(1),

    database_size(c["experiment"]["database_size"].as<size_t>()),
    batch_size(c["experiment"]["batch_size"].as<size_t>()),
    trajectory_size(c["experiment"]["trajectory_size"].as<size_t>()),

    rng(seed),

    db_id(database_size, std::make_tuple(0, 0, 0)),
    db_state(database_size, rl::State(c)),
    db_action(database_size, 0),
    db_reward(database_size, 0.0)
{
    if (trajectory_size == 1)
        throw std::runtime_error("[ExperienceReplay] Trajectory size must be "
                                 "greater than 1.");

    if (database_size % trajectory_size != 0)
        throw std::runtime_error("[ExperienceReplay] Database size must be a "
                                 "multiple of the trajectory size.");
}

size_t ExperienceReplay::rand_idx()
{
    return std::uniform_int_distribution<size_t>{0, min(k, database_size)-2}(rng);
}

size_t ExperienceReplay::n_collected()
{
    return k;
}

bool ExperienceReplay::CollectTrajectory(rl::Agent *m)
{
    size_t new_k = k;

    for (int t = 0; t < trajectory_size; t++) {
        if (environment.isTerminal())
            return false;

        size_t idx = new_k % database_size;
        size_t tau = new_k - (l - 1)*trajectory_size;

        std::get<0>(db_id[idx]) = new_k;
        std::get<1>(db_id[idx]) = l;
        std::get<2>(db_id[idx]) = tau;

        db_state[idx].newState(environment);

        int action = m->action(db_state[idx]);
        if (not environment.performAction(action))
            return false;

        db_action[idx] = action;
        db_reward[idx] = environment.getReward();

        new_k++;
    }

    l++;
    k = new_k;

    return true;
}

bool ExperienceReplay::LearnFromSamples(rl::Agent *m)
{
    if (not environment.Initialise())
        return false;

    while (true) {
        // Collect a new sample trajectory:
        if (not CollectTrajectory(m)) break;

        // Learn from `batch_size` independent samples:
        for (int i = 0; i < batch_size; i++) {
            size_t idx = rand_idx();

            m->HandleTransition(db_state[idx],
                                db_action[idx],
                                db_reward[idx],
                                db_state[idx+1]);
        }
    }

    environment.ClearInventory();

    return true;
}

bool ExperienceReplay::LearnFromTrajectories(rl::Agent *m)
{
    if (not environment.Initialise())
        return false;

    while (true) {
        // Collect a new sample trajectory:
        if (not CollectTrajectory(m)) break;

        // Learn from `batch_size` sample trajectories:
        for (int i = 0; i < batch_size; i++) {
            size_t idx = rand_idx();
            idx -= idx % batch_size;

            for (int j = idx; j < idx+batch_size; j++)
                m->HandleTransition(db_state[idx],
                                    db_action[idx],
                                    db_reward[idx],
                                    db_state[idx+1]);
        }
    }

    environment.ClearInventory();

    return true;
}
