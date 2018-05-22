# rl_markets

This repo contains the core code used to simulate limit order books and
evaluate reinforcement-learning-based strategies for the paper "Market making
via reinforcement learning" (https://arxiv.org/abs/1804.04216v1).

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

## References
Please use the following bibtex entry if citing this work.

```
@article{spooner2018market,
  title={Market Making via Reinforcement Learning},
  author={Spooner, Thomas and Fearnley, John and Savani, Rahul and Koukorinis, Andreas},
  journal={arXiv preprint arXiv:1804.04216},
  year={2018}
}
```
