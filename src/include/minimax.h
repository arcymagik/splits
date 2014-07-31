#ifndef MINIMAX_HH
#define MINIMAX_HH

#include "game_algorithm.h"

class MiniMaxAlg : public Algorithm
{
public:
    MiniMaxAlg();
    MiniMaxAlg(Grader* grader, unsigned int height);
    ~MiniMaxAlg();
    virtual Move* decideMove();

private:
    Grader* grader;
    unsigned int height;

    int minimax(Move* move, unsigned int h);
};

#endif
