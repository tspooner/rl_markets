#ifndef RL_POLICY_H
#define RL_POLICY_H

#include <random>
#include <vector>


namespace rl {

class Policy
{
    protected:
        const unsigned int N_ACTIONS;

        std::mt19937_64 gen;
        std::uniform_int_distribution<unsigned int> unif_int;
        std::uniform_real_distribution<double> unif_real;

    public:
        Policy(unsigned int n_actions, unsigned seed = std::random_device{}());

        virtual unsigned int Sample(std::vector<double> &qs) = 0;

        virtual double descr();
        virtual void HandleTerminal(unsigned int episode);
};

class Random: public Policy
{
    public:
        Random(unsigned int n_actions, unsigned seed = std::random_device{}());

        unsigned int Sample(std::vector<double> &qs);
};

class Greedy: public Policy
{
    public:
        Greedy(unsigned int n_actions, unsigned seed = std::random_device{}());

        unsigned int Sample(std::vector<double> &qs);
};

class EpsilonGreedy: public Greedy
{
    protected:
        double eps;

        const long eps_T;
        const double eps_init,
                     eps_floor;

    public:
        EpsilonGreedy(unsigned int n_actions,
                      double eps, double floor, unsigned int T,
                      unsigned seed = std::random_device{}());

        unsigned int Sample(std::vector<double> &qs);

        double descr();
        void HandleTerminal(unsigned int episode);
};

class Boltzmann: public Policy
{
    protected:
        double tau;

        const long tau_T;
        const double tau_init,
                     tau_floor;

        std::vector<double> probabilities;

    public:
        Boltzmann(unsigned int n_actions,
                  double tau, double floor, unsigned int T,
                  unsigned seed = std::random_device{}());

        unsigned int Sample(std::vector<double> &qs);

        double descr();
        void HandleTerminal(unsigned int episode);
};

}

#endif
