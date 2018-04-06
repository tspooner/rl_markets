# rl_markets

This repo contains the core code used to simulate limit order books and
evaluate reinforcement-learning-based strategies for the paper "Market making
via reinforcement learning."

Dependencies:
- cmake

To build code, run:

    ./configure
    cd build
    make

To cleanup, run:

    ./configure clean

or:

    rm -rf build


Note there may be certain issues with shared_ptr types across machines
(especially between Linux and Mac OS X boxes).
