#ifndef SAMPLER_H
#define SAMPLER_H

#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <algorithm>

template<typename T> class Sampler
{
    protected:
        size_t idx_;
        std::vector<T> source_;

    public:
        Sampler(std::vector<T> source):
            idx_(0),
            source_(source)
        {};

        T sample()
        {
            return this->source_[this->idx_++ % this->source_.size()];
        }
};

template<typename T> class RandomSampler: public Sampler<T>
{
    private:
        std::default_random_engine rng;

    public:
        RandomSampler(std::vector<T> source,
                      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count()):
            Sampler<T>(source),
            rng(seed)
        {}

        T sample()
        {
            auto idx = std::uniform_int_distribution<size_t>{0, this->source_.size()-1}(rng);

            return this->source_[idx];
        }
};

#endif
