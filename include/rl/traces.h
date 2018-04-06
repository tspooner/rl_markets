#ifndef TRACES_H
#define TRACES_H

#include "rl/state.h"

#include <iterator>

namespace rl {

#define MAX_NONZERO_TRACES 100000

class Traces
{
    private:
        const long MEMORY_SIZE;
        const int N_TILINGS;
        const int N_ACTIONS;

        float tolerance;

        int* nonzero_traces_inverse;

    public:
        float* eligibility;

        int n_nonzero_traces;
        int nonzero_traces[MAX_NONZERO_TRACES];

    public:
        Traces(long memory_size, int n_tilings, int n_actions);
        ~Traces();

        void decay(float rate);
        void update(State& state, int action);

        int* begin();
        int* end();

        float get(int feature);
        void set(int feature, float value);

        void clear(int feature);
        void clearExisting(int feature, int loc);

        void increaseTolerance();
};

}

#endif
