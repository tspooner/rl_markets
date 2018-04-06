#include "rl/traces.h"

#include <iostream>
#include <iterator>

using namespace rl;

Traces::Traces(long memory_size, int n_tilings, int n_actions):
    MEMORY_SIZE(memory_size),
    N_TILINGS(n_tilings),
    N_ACTIONS(n_actions),

    tolerance(0.01),
    n_nonzero_traces(0)
{
    eligibility = new float[MEMORY_SIZE];
    nonzero_traces_inverse = new int[MEMORY_SIZE];
    for (long i = 0; i < MEMORY_SIZE; i++) {
        eligibility[i] = 0.0;
        nonzero_traces_inverse[i] = 0;
    }
}

Traces::~Traces()
{
    delete [] eligibility;
    delete [] nonzero_traces_inverse;
}

void Traces::decay(float rate)
{
    for (int loc = n_nonzero_traces-1; loc >= 0; loc--) {
        int f = nonzero_traces[loc];
        eligibility[f] *= rate;

        if (eligibility[f] < tolerance) clearExisting(f, loc);
    }
}

void Traces::update(State& state, int action)
{
    for (int a = 0; a < N_ACTIONS; a++) {
        auto features = state.getFeatures(a);

        if (a != action)
            for (int t = 0; t < N_TILINGS; t++) clear(features[t]);
        else
            for (int t = 0; t < N_TILINGS; t++) set(features[t], 1.0);
    }
}

int* Traces::begin()
{
    return nonzero_traces;
}

int* Traces::end()
{
    return &nonzero_traces[n_nonzero_traces];
}

float Traces::get(int feature)
{
    return eligibility[feature];
}

void Traces::set(int feature, float value)
{
    if (eligibility[feature] >= tolerance) eligibility[feature] = value;
    else {
        while (n_nonzero_traces >= MAX_NONZERO_TRACES) increaseTolerance();

        eligibility[feature] = value;
        nonzero_traces[n_nonzero_traces] = feature;
        nonzero_traces_inverse[feature] = n_nonzero_traces;
        n_nonzero_traces++;
    }
}

void Traces::clear(int feature)
{
    if (eligibility[feature] != 0.0)
        clearExisting(feature, nonzero_traces_inverse[feature]);
}

void Traces::clearExisting(int feature, int loc)
{
    eligibility[feature] = 0.0;
    n_nonzero_traces--;
    nonzero_traces[loc] = nonzero_traces[n_nonzero_traces];
    nonzero_traces_inverse[nonzero_traces[loc]] = loc;
}

void Traces::increaseTolerance()
{
    tolerance *= 1.1;
    for (int loc = n_nonzero_traces-1; loc >= 0; loc--) {
        int f = nonzero_traces[loc];

        if (eligibility[f] < tolerance) clearExisting(f, loc);
    }
}
