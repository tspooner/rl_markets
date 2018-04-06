#include "rl/policy.h"

#include <cmath>
#include <vector>
#include <iostream>

using namespace rl;


Policy::Policy(unsigned int n_actions, unsigned seed):
    N_ACTIONS(n_actions),

    gen(seed),
    unif_int(0, n_actions-1),
    unif_real(0.0, 1.0)
{}

double Policy::descr() { return 0.0f; }

void Policy::HandleTerminal(unsigned int) {}


Random::Random(unsigned int n_actions, unsigned seed):
    Policy(n_actions, seed)
{}

unsigned int Random::Sample(std::vector<double>&)
{
    return unif_int(gen);
}


Greedy::Greedy(unsigned int n_actions, unsigned seed):
    Policy(n_actions, seed)
{}

unsigned int Greedy::Sample(std::vector<double> &qs)
{
    int argmax = 0;
    int n_ties = 1;

    for (int a = 1; a < N_ACTIONS; a++) {
        if (qs[a] > qs[argmax]) {
            argmax = a;

        } else if (qs[a] >= qs[argmax]) {
            n_ties++;

            if (0 == rand() % n_ties)
                argmax = a;
        }
    }

    return argmax;
}


EpsilonGreedy::EpsilonGreedy(unsigned int n_actions,
                             double eps, double floor, unsigned int T,
                             unsigned seed):
    Greedy(n_actions, seed),

    eps(eps),
    eps_T(T),
    eps_init(eps),
    eps_floor(floor)
{}

unsigned int EpsilonGreedy::Sample(std::vector<double> &qs)
{
    if (unif_real(gen) < eps)
        return unif_int(gen);
    else
        return Greedy::Sample(qs);
}

double EpsilonGreedy::descr() { return eps; }

void EpsilonGreedy::HandleTerminal(unsigned int episode)
{
    eps = eps_init * pow(eps_floor / eps_init, (double) episode / eps_T);
}


Boltzmann::Boltzmann(unsigned int n_actions,
                     double tau, double floor, unsigned int T,
                     unsigned seed):
    Policy(n_actions, seed),

    tau(tau),
    tau_T(T),
    tau_init(tau),
    tau_floor(floor),

    probabilities(n_actions, 0.0)
{}

unsigned int Boltzmann::Sample(std::vector<double> &qs)
{
    double z = 0.0;
    for (int a = 0; a < N_ACTIONS; a++) {
        probabilities[a] = std::exp(qs[a] / tau);
        z += probabilities[a];
    }

    double acc = 0.0;
    double r = unif_real(gen);
    for (int a = 0; a < N_ACTIONS; a++) {
        acc += probabilities[a] / z;

        if (r < acc) return a;
    }

    return N_ACTIONS-1;
}

double Boltzmann::descr() { return tau; }

void Boltzmann::HandleTerminal(unsigned int episode)
{
    tau = tau_init * pow(tau_floor / tau_init, (double) episode / tau_T);
}
