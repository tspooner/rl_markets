#include "market/latency.h"

#include <math.h>
#include <stdexcept>

using namespace market;


Latency::Latency(const float floor):
    floor(floor)
{
    if (floor < 0.0f)
        throw std::invalid_argument("Latency must be zero or positive.");
}

float Latency::sample() { return floor; }


StochasticLatency::StochasticLatency(const float support, const unsigned seed):
    Latency(support),
    gen(seed)
{}

NormalLatency::NormalLatency(const float mu,
                             const float sigma,
                             const unsigned seed,
                             const float support):
    StochasticLatency(support, seed),
    dist(mu, sigma)
{}

float NormalLatency::sample()
{ return std::max(0.0f, Latency::sample() + dist(gen)); }

LognormalLatency::LognormalLatency(const float mu,
                                   const float beta,
                                   const unsigned seed,
                                   const float support):
    StochasticLatency(support, seed),
    dist(mu, beta)
{}

float LognormalLatency::sample()
{ return std::max(0.0f, Latency::sample() + dist(gen)); }
