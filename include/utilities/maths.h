#ifndef UTILITIES_MATHS_H
#define UTILITIES_MATHS_H

template<typename T>
T ulb(T val, T lb, T ub)
{
    return max(min(val, ub), lb);
}

#endif
