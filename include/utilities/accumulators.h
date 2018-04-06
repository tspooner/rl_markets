#ifndef ACCUMULATORS_H
#define ACCUMULATORS_H

#include <set>
#include <deque>
#include <cstddef>

template <typename T>
class Accumulator
{
    protected:
        const size_t window_size;

        std::deque<T> window;

        T _sum = T(0);

    public:
        Accumulator(size_t window_size);

        virtual void push(T val);

        T sum();

        T at(size_t i);
        T front();
        T back();

        void clear();

        bool full();
        size_t size();
};

template <typename T>
class RollingMean: public Accumulator<T>
{
    private:
        T _mean;
        T _s;

    public:
        RollingMean(size_t window_size);

        void push(T val);

        T mean();
        T var();
        T std();

        T zscore(T val);
        T last_zscore();
};

template <typename T>
class EWMA: public Accumulator<T>
{
    private:
        double _alpha;
        T _mean;

    public:
        EWMA(size_t window_size);

        void push(T val);

        T mean();
};

template <typename T>
class RollingMedian: public Accumulator<T>
{
    private:
        std::multiset<T> min_heap;
        std::multiset<T> max_heap;

    public:
        RollingMedian(size_t window_size);

        void push(T val);

        T median();

        T quartile(double quartile);
        T iqr();

        T min();
        T max();

        T zscore(T val);
        T last_zscore();
};

#endif
