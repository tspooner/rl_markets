#ifndef RL_STATE_H
#define RL_STATE_H

#include <vector>

#include "environment/base.h"

namespace rl {

class State
{
    private:
        // Pseudo-constant:
        long MEMORY_SIZE;
        int N_TILINGS;
        int N_ACTIONS;
        // ---

        std::vector<float> state_vars;
        std::vector<std::vector<int>> features;

        double potential;

    public:
        State(long n_states, int n_actions, int n_tilings);
        State(Config &c);

        void initialise();

        void newState(environment::Base& env);
        void newState(std::vector<float>& vars, double potential = 0.0);

        void populateFeatures();
        std::vector<int>& getFeatures(int action = 0);

        double getPotential();

        std::vector<float>& toVector();
        void printState();
};

}

#endif
