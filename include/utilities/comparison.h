#ifndef UTILITIES_COMPARISON_H
#define UTILITIES_COMPARISON_H

#define _TOL_ 10000

#include <math.h>

template<unsigned int TOL = _TOL_>
struct FloatComparator
{
    FloatComparator() = default;

    bool operator()(double a, double b) const
    {
        return rint(a * TOL) < rint(b * TOL);
    }
};

template<unsigned int TOL = _TOL_>
struct ReverseFloatComparator
{
    ReverseFloatComparator() = default;

    bool operator()(double a, double b) const
    {
        return rint(b * TOL) < rint(a * TOL);
    }
};

template<unsigned int TOL = _TOL_>
bool approx_equal(double a, double b)
{
    return rint(a * TOL) == rint(b * TOL);
}

#endif
