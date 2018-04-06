#ifndef MARKET_LATENCY_H
#define MARKET_LATENCY_H

#include <random>

namespace market {

class Latency
{
    protected:
        const float floor;

    public:
        Latency(const float floor = 0);
        virtual ~Latency() = default;

        virtual float sample();
};


class StochasticLatency: public Latency
{
    protected:
        std::mt19937_64 gen;

        StochasticLatency(const float support, unsigned seed);
};

class NormalLatency: public StochasticLatency
{
    private:
        std::normal_distribution<float> dist;

    public:
        NormalLatency(const float mu, const float sigma,
                      unsigned seed, const float support = 0.0f);

        float sample();
};

class LognormalLatency: public StochasticLatency
{
    private:
        std::lognormal_distribution<float> dist;

    public:
        LognormalLatency(const float mu, const float beta,
                         unsigned seed, const float support = 0.0f);

        float sample();
};

}

#endif
