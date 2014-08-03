#ifndef ALPHA_BETA_HH
#define ALPHA_BETA_HH

#include "game_algorithm.h"

class AlphaBetaAlg : public Algorithm
{
public:
    AlphaBetaAlg();
    AlphaBetaAlg(Grader* grader, unsigned int height, unsigned int height_building);
    virtual ~AlphaBetaAlg();
    virtual Move* decideMove();
    virtual void decideMove(Move** move);

private:
    Grader* grader;
    unsigned int height;
    unsigned int height_building;

    unsigned int visited_nodes;

    void updateWindow(int best, int cp , int* alpha, int* beta);
    bool outsideWindow(int best, int cp, int alpha, int beta);
    int alpha_beta(Move* move, int alpha, int beta, unsigned int height);
    int alpha_beta_opt(Move* move, int alpha, int beta, unsigned int height);
};

#endif
