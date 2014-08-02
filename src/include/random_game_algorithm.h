#ifndef RANDOM_GAME_ALG_HH
#define RANDOM_GAME_ALG_HH

#include "game_algorithm.h"
#include <random>

class RandomGameAlg : public Algorithm
{
public:
    RandomGameAlg();
    RandomGameAlg(unsigned int seed);
    virtual ~RandomGameAlg();
    virtual Move* decideMove();
    virtual void decideMove(Move** move);
    virtual std::string stats();
private:
    std::mt19937 generator;
    unsigned int moves_size;
};

#endif

