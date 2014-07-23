#ifndef RANDOM_GAME_ALG_HH
#define RANDOM_GAME_ALG_HH

#include "game_algorithm.h"
#include <random>

class RandomGameAlg : public Algorithm
{
public:
    RandomGameAlg();
    RandomGameAlg(unsigned int seed);
    ~RandomGameAlg();
    virtual Move* decideMove();
private:
    std::mt19937 generator;
};

#endif

