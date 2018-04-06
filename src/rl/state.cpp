#include "rl/state.h"

#include "rl/tiles.h"

#include <iostream>
#include <algorithm>

using namespace rl;

State::State(long memory_size, int n_actions, int n_tilings):
    MEMORY_SIZE(memory_size),
    N_TILINGS(n_tilings),
    N_ACTIONS(n_actions),

    state_vars(),
    features(n_actions, std::vector<int>(3*n_tilings, 0)),

    potential(0.0)
{}

State::State(Config &c):
    State(c["learning"]["memory_size"].as<long>(),
          c["learning"]["n_actions"].as<int>(),
          c["learning"]["n_tilings"].as<int>())
{}

void State::initialise()
{
    state_vars.clear();

    for (int a = 0; a < N_ACTIONS; a++)
        features[a].clear();
}

void State::newState(environment::Base& env)
{
    state_vars.clear();
    env.getState(state_vars);

    populateFeatures();

    potential = env.getPotential();
}

void State::newState(vector<float>& _vars, double _potential)
{
    state_vars = _vars;
    populateFeatures();

    potential = _potential;
}

void State::populateFeatures()
{
    for (int a = 0; a < N_ACTIONS; a++) {
        ::tiles(&features[a][0], N_TILINGS, MEMORY_SIZE,
                &state_vars[0], 3, a);

        ::tiles(&features[a][N_TILINGS], N_TILINGS, MEMORY_SIZE,
                &state_vars[3], state_vars.size()-3, N_ACTIONS + a);

        ::tiles(&features[a][2*N_TILINGS], N_TILINGS, MEMORY_SIZE,
                &state_vars[0], state_vars.size(), (2*N_ACTIONS) + a);
    }
}

std::vector<int>& State::getFeatures(int action)
{
    return features[action];
}

double State::getPotential()
{
    return potential;
}

vector<float>& State::toVector()
{
    return state_vars;
}

void State::printState()
{
    for (size_t i = 0; i < state_vars.size(); i++)
        cout << "[" << i << "] " << state_vars[i] << " \t\t-> " <<
            floor(state_vars[i] * N_TILINGS) << endl;

    cout << "\n--Press any key to continue--\n" << flush;
    cin.sync();
    cin.get();
}
