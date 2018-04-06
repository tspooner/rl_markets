#ifndef RL_AGENT_H
#define RL_AGENT_H

#include <tuple>
#include <memory>
#include <random>
#include <fstream>

#include "rl/state.h"
#include "rl/policy.h"
#include "rl/traces.h"

namespace rl {

class Agent
{
    protected:
        const long MEMORY_SIZE;
        const int N_TILINGS;
        const int N_ACTIONS;

        double* theta;
        std::tuple<double, double, double> group_weights;

        Traces traces;

        // Learning parameters {
        const double alpha_start,
                    alpha_floor,
                    omega;
        double alpha;

        double gamma;
        double lambda;
        // }

        std::mt19937_64 gen;
        std::uniform_real_distribution<double> unif_dist;

        double _agg_delta = 0.0;
        int _update_counter = 0;

        virtual void UpdateTraces(State& from_state, int action);

        int epsilonGreedy(State& state);

    public:
        Agent(std::unique_ptr<Policy> policy, Config &c);
        virtual ~Agent();

        std::unique_ptr<Policy> policy;

        // Policies
        virtual unsigned int action(State& s);

        void GoGreedy();
        void SetPolicy(std::unique_ptr<Policy> policy);

        // Updating weights:
        void HandleTransition(State& from_state, int action, double reward,
                              State& to_state);
        void HandleTerminal(int episode);

        virtual double UpdateWeights(State& from_state,
                                     int action,
                                     double reward,
                                     State& to_state) = 0;

        // State value iteration
        void updateQ(double update);

        double getQ(State& state, int action);
        int argmaxQ(State& state);
        double maxQ(State& state);

        // IO
        void write_theta(string filename);
};

class DoubleAgent: public Agent {
    private:
        double* theta_b;

    public:
        DoubleAgent(std::unique_ptr<Policy> policy, Config& c);
        ~DoubleAgent();

        unsigned int action(State& s);

        void updateQb(double update);

        double getQb(State& state, int action);
        int argmaxQb(State& state);
};


class QLearn: public Agent {
    private:
        void UpdateTraces(State& from_state, int action);

    public:
        QLearn(std::unique_ptr<Policy> policy, Config& c);

        double UpdateWeights(State& from_state, int action, double reward,
                             State& to_state);
};

class SARSA: public Agent {
    public:
        SARSA(std::unique_ptr<Policy> policy, Config& c);

        double UpdateWeights(State& from_state, int action, double reward,
                             State& to_state);
};

class DoubleQLearn: public DoubleAgent {
    // H. V Hasselt, “Double Q-learning,”
    private:
        void UpdateTraces(State& from_state, int action);

    public:
        DoubleQLearn(std::unique_ptr<Policy> policy, Config& c);

        double UpdateWeights(State& from_state, int action, double reward,
                             State& to_state);
};

class RLearn: public Agent {
    private:
        double beta;
        double rho = 0;

        void UpdateTraces(State& from_state, int action);

    public:
        RLearn(std::unique_ptr<Policy> policy, Config& c);

        double UpdateWeights(State& from_state, int action, double reward,
                             State& to_state);
};

class OnlineRLearn: public Agent {
    private:
        double beta;
        double rho = 0;

    public:
        OnlineRLearn(std::unique_ptr<Policy> policy, Config& c);

        double UpdateWeights(State& from_state, int action, double reward,
                             State& to_state);
};

class DoubleRLearn: public DoubleAgent {
    private:
        double beta;
        double rho = 0;

        void UpdateTraces(State& from_state, int action);

    public:
        DoubleRLearn(std::unique_ptr<Policy> policy, Config& c);

        double UpdateWeights(State& from_state, int action, double reward,
                             State& to_state);
};

}

#endif
