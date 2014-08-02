#ifndef MINIMAX_HH
#define MINIMAX_HH

#include "game_algorithm.h"

class MiniMaxAlg : public Algorithm
{
public:
    MiniMaxAlg();
    MiniMaxAlg(Grader* grader, unsigned int height);
    virtual ~MiniMaxAlg();
    virtual Move* decideMove();
    virtual void decideMove(Move** move);
    virtual std::string stats();
private:
    Grader* grader;
    unsigned int height;

    unsigned int visited_nodes;

    int minimax(Move* move, unsigned int h);
    int minimax_opt(Move* move, unsigned int h);
};

#endif
