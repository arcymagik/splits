#ifndef ALPHA_BETA_HH
#define ALPHA_BETA_HH

#include "game_algorithm.h"
#include "transposition_table.h"
#include "zobrist_hasher.h"

class AlphaBetaAlg : public Algorithm
{
public:
    AlphaBetaAlg();
    AlphaBetaAlg(Grader* grader, unsigned int height, unsigned int height_building);
    AlphaBetaAlg(TranspositionTable* transTable, Hasher* hash, Grader* grader, unsigned int height, unsigned int height_building);
    virtual ~AlphaBetaAlg();
    virtual Move* decideMove();
    virtual void decideMove(Move** move);
    virtual void makeMove(Move* move);

private:
    Grader* grader;
    TranspositionTable* transTable;
    Hasher* hasher;
    unsigned int height;
    unsigned int height_building;

    unsigned int visited_nodes;

    void updateWindow(int best, int cp , int* alpha, int* beta);
    bool outsideWindow(int best, int cp, int alpha, int beta);
    int alpha_beta(Move* move, int alpha, int beta, unsigned int height);
    int alpha_beta_opt(unsigned int mindex, int alpha, int beta, unsigned int height);
    int getHashedValue(unsigned int height);
    void setHashedValue(int grade, unsigned int height);
};

#endif
