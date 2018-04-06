#include "utilities/accumulators.h"

#include <math.h>
#include <numeric>
#include <iostream>
#include <stdexcept>

using namespace std;

// ------------ Accumulator ------------------
template <typename T>
Accumulator<T>::Accumulator(size_t window_size):
    window_size(window_size),
    window()
{}

template <typename T>
void Accumulator<T>::push(T val)
{
    _sum += val;
    window.push_front(val);

    if (size() > window_size) {
        _sum -= window.back();
        window.pop_back();
    }
}

template <typename T>
T Accumulator<T>::sum()
{
    return _sum;
}

template <typename T>
T Accumulator<T>::at(size_t i)
{
    if (size() > i) return window[i];
    else return -1;
}

template <typename T>
T Accumulator<T>::front()
{
    if (size() == 0)
        throw runtime_error("Attempting to access `front` of an empty accumulator...");

    return window.front();
}

template <typename T>
T Accumulator<T>::back()
{
    if (size() == 0)
        throw runtime_error("Attempting to access `back` of an empty accumulator...");

    return window.back();
}

template <typename T>
void Accumulator<T>::clear()
{
    window.clear();
}

template <typename T>
bool Accumulator<T>::full()
{
    return (window.size() == window_size);
}

template <typename T>
size_t Accumulator<T>::size()
{
    return window.size();
}

// ------------ Rolling Mean --------------------

template <typename T>
RollingMean<T>::RollingMean(size_t window_size):
    Accumulator<T>(window_size),
    _mean(T(0)), _s(T(0))
{}

template <typename T>
void RollingMean<T>::push(T val)
{
    this->_sum += val;
    this->window.push_front(val);

    T n = this->size();
    T _old_mean = _mean;

    _mean += (val - _mean) / n;
    _s += (val - _mean) * (val - _old_mean);

    if (this->size() > this->window_size) {
        T old = this->window.back();
        this->window.pop_back();
        this->_sum -= old;

        T n = this->size();
        T _old_mean = _mean;

        _mean -= (old - _mean) / n;
        _s -= (old - _mean) * (old - _old_mean);
    }
}

template <typename T>
T RollingMean<T>::mean()
{
    return _mean;
}

template <typename T>
T RollingMean<T>::var()
{
    return _s / (T) (this->size() - 1);
}

template <typename T>
T RollingMean<T>::std()
{
    T v = var();
    if (v > 0)
        return sqrt(v);
    else
        return T(0);
}

template <typename T>
T RollingMean<T>::zscore(T val)
{
    T _std = std();
    return _std > 0 ? (val - mean()) / _std : 0;
}

template <typename T>
T RollingMean<T>::last_zscore()
{
    return zscore(this->window.front());
}

// ------------ EWMA --------------------

template <typename T>
EWMA<T>::EWMA(size_t window_size):
    Accumulator<T>(window_size),

    _alpha(2.0 / (window_size + 1.0)),
    _mean(T(0))
{}

template <typename T>
void EWMA<T>::push(T val)
{
    this->window.push_front(val);
    this->window.pop_back();

    this->_mean = (this->_alpha * val) + ((1 - this->_alpha) * this->_mean);
}

template <typename T>
T EWMA<T>::mean()
{
    return _mean;
}

// ----------- Rolling Median -------------------

template <typename T>
RollingMedian<T>::RollingMedian(size_t window_size):
    Accumulator<T>(window_size)
{}

template <typename T>
void RollingMedian<T>::push(T val)
{
    if (this->size() == this->window_size) {
        T old = this->window.back();
        this->window.pop_back();

        this->_sum -= old;

        if (min_heap.size() and old <= *(--min_heap.end()))
            min_heap.erase(min_heap.find(old));
        else if (max_heap.size() and old >= *max_heap.begin())
            max_heap.erase(max_heap.find(old));
    }

    this->_sum += val;
    this->window.push_front(val);

    if (max_heap.size() and val >= *max_heap.begin())
        max_heap.insert(val);
    else
        min_heap.insert(val);

    while (min_heap.size() > max_heap.size()) {
        max_heap.insert(*(--min_heap.end()));
        min_heap.erase(--min_heap.end());
    }

    while (max_heap.size() > min_heap.size()) {
        min_heap.insert(*max_heap.begin());
        max_heap.erase(max_heap.begin());
    }
}

template <typename T>
T RollingMedian<T>::median()
{
    T median;

    if (min_heap.size() == max_heap.size())
        median = ((*(--min_heap.end()) + *max_heap.begin()) / 2.0f);
    else
        median = *(--min_heap.end());

    return median;
}

template <typename T>
T RollingMedian<T>::quartile(double q)
{
    if (this->size() < 3) return median();

    size_t loc;
    typename multiset<T>::iterator it;

    if (q <= 0.50) {
        loc = q*min_heap.size();
        it = min_heap.cbegin();
    } else {
        loc = q*max_heap.size()/2;
        it = max_heap.cbegin();
    }

    advance(it, loc);

    if (loc && loc % 2 == 0)
        return T(*it + *(--it)) / 2.0f;
    else
        return *it;
}

template <typename T>
T RollingMedian<T>::iqr()
{
    return (quartile(0.75) - quartile(0.25));
}

template <typename T>
T RollingMedian<T>::min()
{
    return *min_heap.begin();
}

template <typename T>
T RollingMedian<T>::max()
{
    return *(--max_heap.end());
}

template <typename T>
T RollingMedian<T>::zscore(T val)
{
    T _iqr = iqr();
    return _iqr > 0 ? (val - median()) / _iqr : 0;
}

template <typename T>
T RollingMedian<T>::last_zscore()
{
    return zscore(this->window.front());
}

// Explicit implementations
template class Accumulator<int>;
template class Accumulator<float>;
template class Accumulator<double>;

template class RollingMean<int>;
template class RollingMean<float>;
template class RollingMean<double>;

template class EWMA<int>;
template class EWMA<float>;
template class EWMA<double>;

template class RollingMedian<int>;
template class RollingMedian<float>;
template class RollingMedian<double>;
