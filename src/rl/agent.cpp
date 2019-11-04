#include "rl/agent.h"

#include <cmath>
#include <memory>
#include <float.h>
#include <iostream>
#include <algorithm>
#include <spdlog/spdlog.h>

using namespace std;
using namespace rl;

Agent::Agent(std::unique_ptr<Policy> policy, Config &c):
    MEMORY_SIZE(c["learning"]["memory_size"].as<long>()),
    N_TILINGS(c["learning"]["n_tilings"].as<int>()),
    N_ACTIONS(c["learning"]["n_actions"].as<int>()),

    group_weights(make_tuple(1.0/3, 1.0/3, 1.0/3)),

    traces(MEMORY_SIZE, N_TILINGS, N_ACTIONS),

    alpha_start(c["learning"]["alpha_start"].as<double>(0.2)),
    alpha_floor(c["learning"]["alpha_floor"].as<double>(0.001)),
    omega(c["learning"]["omega"].as<double>(1.0)),
    alpha(alpha_start),

    gamma(c["learning"]["gamma"].as<double>()),
    lambda(c["learning"]["lambda"].as<double>()),

    gen(c["debug"]["random_seed"].as<unsigned>(random_device{}())),
    unif_dist(0.0, 1.0),

    policy(std::move(policy))
{
    theta = new double[MEMORY_SIZE];

    if (c["learning"]["random_init"].as<bool>(false))
        generate(&theta[0], &theta[MEMORY_SIZE],
                 [this]() { return 2.0*unif_dist(gen)-1.0; });
    else
        fill(&theta[0], &theta[MEMORY_SIZE], 0.0);

    if (c["learning"]["group_weights"]) {
        get<0>(group_weights) = c["learning"]["group_weights"][0].as<double>();
        get<1>(group_weights) = c["learning"]["group_weights"][1].as<double>();

        double alt = 1.0 - (get<0>(group_weights) + get<1>(group_weights));
        get<2>(group_weights) =
            c["learning"]["group_weights"][2].as<double>(alt);
    }

    // Register loggers for TD-error:
    if (c["logging"] and c["logging"]["log_learning"].as<bool>(true)) {
        try {
            auto log = spdlog::rotating_logger_mt("model_log",
                                                  c["output_dir"].as<string>() + "model_log.csv",
                                                  c["logging"]["max_size"].as<size_t>(), 1);
        } catch (spdlog::spdlog_ex& e) {}
    }
}

Agent::~Agent()
{
    delete [] theta;
}

unsigned int Agent::action(State& s)
{
    std::vector<double> qs(N_ACTIONS, 0.0);
    for (int a = 0; a < N_ACTIONS; a++)
        qs[a] = getQ(s, a);

    return policy->Sample(qs);
}

void Agent::GoGreedy()
{
    this->policy = std::unique_ptr<Policy>(new Greedy(N_ACTIONS));
}

void Agent::SetPolicy(std::unique_ptr<Policy> policy)
{
    this->policy = std::move(policy);
}

void Agent::HandleTransition(State& from_state, int action,
                             double reward, State& to_state)
{
    UpdateTraces(from_state, action);

    double delta = UpdateWeights(from_state, action, reward, to_state);

    _agg_delta += abs(delta);

    if (++_update_counter % 1000 == 0) {
        spdlog::get("model_log")->info(_agg_delta / 1000);

        _agg_delta = 0.0;
        _update_counter = 0;
    }
}

void Agent::HandleTerminal(int episode)
{
    traces.decay(0.0);
    alpha = max(alpha_floor, alpha_start * pow(omega, (double) episode));

    this->policy->HandleTerminal(episode);
}

void Agent::UpdateTraces(State& from_state, int action)
{
    traces.decay(gamma*lambda);
    traces.update(from_state, action);
}

double Agent::getQ(State& state, int action)
{
    auto features = state.getFeatures(action);
    double Q = 0.0;

    double w = get<0>(group_weights);
    for (int i = 0; i < N_TILINGS; i++)
        Q += w*theta[features[i]];

    w = get<1>(group_weights);
    for (int i = N_TILINGS; i < 2*N_TILINGS; i++)
        Q += w*theta[features[i]];

    w = get<2>(group_weights);
    for (int i = N_TILINGS; i < 3*N_TILINGS; i++)
        Q += w*theta[features[i]];

    return Q;
}

void Agent::updateQ(double update)
{
    double scaled_update = update / N_TILINGS;
    for (auto it = traces.begin(); it != traces.end(); it++)
        theta[*it] += scaled_update * traces.get(*it);
}

int Agent::argmaxQ(State& state)
{
    int index = 0;
    int n_ties = 1;
    double currMaxQ = getQ(state, index);

    for (int a = 1; a < N_ACTIONS; a++) {
        double val = getQ(state, a);

        if (val >= currMaxQ) {
            if (val > currMaxQ) {
                currMaxQ = val;
                index = a;
            } else {
                n_ties++;

                if (0 == rand() % n_ties) {
                    currMaxQ = val;
                    index = a;
                }
            }
        }
    }

    return index;
}

double Agent::maxQ(State& state)
{
    return getQ(state, argmaxQ(state));
}

void Agent::write_theta(string filename)
{
    ofstream file(filename.c_str(), ios::binary);
    file.write((char *) theta, MEMORY_SIZE * sizeof(double));
    file.close();
}

// ---------------

DoubleAgent::DoubleAgent(std::unique_ptr<Policy> policy, Config& c):
    Agent(std::move(policy), c)
{
    theta_b = new double[MEMORY_SIZE];

    if (c["learning"]["random_init"].as<bool>(false))
        generate(&theta_b[0], &theta_b[MEMORY_SIZE],
                 [this]() { return 2.0*unif_dist(gen)-1.0; });
    else
        fill(&theta_b[0], &theta_b[MEMORY_SIZE], 0.0);
}

DoubleAgent::~DoubleAgent()
{
    delete [] theta_b;
}

unsigned int DoubleAgent::action(State& s)
{
    std::vector<double> qs(N_ACTIONS, 0.0);
    for (int a = 0; a < N_ACTIONS; a++)
        qs[a] = (getQ(s, a) + getQb(s, a)) / 2.0f;

    return policy->Sample(qs);
}

double DoubleAgent::getQb(State& state, int action)
{
    auto features = state.getFeatures(action);

    double Q = 0.0;

    double w = get<0>(group_weights);
    for (int i = 0; i < N_TILINGS; i++)
        Q += w*theta_b[features[i]];

    w = get<1>(group_weights);
    for (int i = N_TILINGS; i < 2*N_TILINGS; i++)
        Q += w*theta_b[features[i]];

    w = get<2>(group_weights);
    for (int i = N_TILINGS; i < 3*N_TILINGS; i++)
        Q += w*theta_b[features[i]];

    return Q;
}

void DoubleAgent::updateQb(double update)
{
    double scaled_update = update / N_TILINGS;
    for (auto it = traces.begin(); it != traces.end(); it++)
        theta_b[*it] += scaled_update * traces.get(*it);
}

int DoubleAgent::argmaxQb(State& state)
{
    int index = 0;
    int n_ties = 1;
    double currMaxQ = getQb(state, index);

    for (int a = 1; a < N_ACTIONS; a++) {
        double val = getQb(state, a);

        if (val >= currMaxQ) {
            if (val > currMaxQ) {
                currMaxQ = val;
                index = a;
            } else {
                n_ties++;

                if (0 == rand() % n_ties) {
                    currMaxQ = val;
                    index = a;
                }
            }
        }
    }

    return index;
}

// ---------------

QLearn::QLearn(std::unique_ptr<Policy> policy, Config& c):
    Agent(std::move(policy), c)
{}

void QLearn::UpdateTraces(State& from_state, int action)
{
    int amax = argmaxQ(from_state);

    if (action != amax) traces.decay(0.0);
    else traces.decay(gamma*lambda);

    traces.update(from_state, action);
}

double QLearn::UpdateWeights(State& from_state, int action, double reward,
                             State& to_state)
{
    double Q = getQ(from_state, action),
           F_term = gamma*to_state.getPotential() - from_state.getPotential(),
           delta = reward + F_term + gamma*maxQ(to_state) - Q;

    updateQ(alpha * delta);

    return delta;
}

// ---------------

SARSA::SARSA(std::unique_ptr<Policy> policy, Config& c):
    Agent(std::move(policy), c)
{}

double SARSA::UpdateWeights(State& from_state, int action, double reward,
                            State& to_state)
{
    double Q1 = getQ(from_state, action),
          Q2 = getQ(to_state, this->action(to_state)),
          F = gamma*to_state.getPotential() - from_state.getPotential(),
          delta = reward + F + gamma*Q2 - Q1;

    updateQ(alpha * delta);

    return delta;
}

// ---------------

DoubleQLearn::DoubleQLearn(std::unique_ptr<Policy> policy, Config& c):
    DoubleAgent(std::move(policy), c)
{}

void DoubleQLearn::UpdateTraces(State& from_state, int action)
{
    int amax = argmaxQ(from_state);

    if (action != amax) traces.decay(0.0);
    else traces.decay(gamma*lambda);

    traces.update(from_state, action);
}

double DoubleQLearn::UpdateWeights(State& from_state, int action, double reward,
                                   State& to_state)
{
    double delta;
    double F_term = gamma*to_state.getPotential() - from_state.getPotential();
    if (unif_dist(gen) > 0.5) { // UPDATE(A)

        double Qa = getQ(from_state, action);
        delta = reward + F_term +
            gamma*getQb(to_state, argmaxQ(to_state)) - Qa;

        updateQ(alpha * delta);

    } else { // UPDATE(B)

        double Qb = getQb(from_state, action);
        delta = reward + F_term +
            gamma*getQ(to_state, argmaxQb(to_state)) - Qb;

        updateQb(alpha * delta);

    }

    return delta;
}

// ---------------

RLearn::RLearn(std::unique_ptr<Policy> policy, Config& c):
    Agent(std::move(policy), c),

    beta(c["learning"]["beta"].as<double>())
{}

void RLearn::UpdateTraces(State& from_state, int action)
{
    int amax = argmaxQ(from_state);

    if (action != amax) traces.decay(0.0);
    else traces.decay(gamma*lambda);

    traces.update(from_state, action);
}

double RLearn::UpdateWeights(State& from_state, int action, double reward,
                             State& to_state)
{
    double Q = getQ(from_state, action),
          mQ = maxQ(to_state),
          delta = reward - rho + mQ - Q,
          update = alpha*delta;

    updateQ(update);

    double nQ = Q + update;
    if (nQ - maxQ(from_state) < 1e-7)
        rho += beta * (reward - rho + mQ - nQ);

    return delta;
}

// ---------------

OnlineRLearn::OnlineRLearn(std::unique_ptr<Policy> policy, Config& c):
    Agent(std::move(policy), c),

    beta(c["learning"]["beta"].as<double>())
{}

double OnlineRLearn::UpdateWeights(State& from_state, int action, double reward,
                                   State& to_state)
{
    double Q = getQ(from_state, action),
          gQ = getQ(to_state, this->action(to_state)),
          delta = reward - rho + gQ - Q,
          update = alpha*delta;

    updateQ(update);

    double nQ = Q + update;
    if (nQ - maxQ(from_state) < 1e-7)
        rho += beta * (reward - rho + gQ - nQ);

    return delta;
}
// ---------------

DoubleRLearn::DoubleRLearn(std::unique_ptr<Policy> policy, Config& c):
    DoubleAgent(std::move(policy), c),

    beta(c["learning"]["beta"].as<double>())
{}

void DoubleRLearn::UpdateTraces(State& from_state, int action)
{
    int amax = argmaxQ(from_state);

    if (action != amax) traces.decay(0.0);
    else traces.decay(gamma*lambda);

    traces.update(from_state, action);
}

double DoubleRLearn::UpdateWeights(State& from_state, int action, double reward,
                                   State& to_state)
{
    double delta, Q, mQ;
    if (unif_dist(gen) > 0.5) { // UPDATE(A)

        Q = getQ(from_state, action);
        mQ = getQb(to_state, argmaxQ(to_state));
        delta = reward - rho + mQ - Q;

        updateQ(alpha*delta);

    } else { // UPDATE(B)

        Q = getQb(from_state, action);
        mQ = getQ(to_state, argmaxQb(to_state));
        delta = reward - rho + mQ - Q;

        updateQb(alpha*delta);

    }

    mQ = -DBL_MAX;
    for (int i = 0; i < N_ACTIONS; i++) {
        double val = (getQ(from_state, i) + getQb(from_state, i)) / 2.0;

        if (val > mQ)
            mQ = val;
    }

    double nQ = Q + alpha*delta;
    if (nQ - mQ < 1e-7)
        rho += beta * (reward - rho + mQ - nQ);

    return delta;
}
