#ifndef ALPHA_BETA_HH
#define ALPHA_BETA_HH

#include "game_algorithm.h"
#include "transposition_table.h"
#include "zobrist_hasher.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>

class AlphaBetaAlg : public Algorithm
{
public:
    AlphaBetaAlg();
    AlphaBetaAlg(unsigned int seed, Grader* grader, unsigned int height, unsigned int height_building);
    AlphaBetaAlg(unsigned int seed, TranspositionTable* transTable, Hasher* hash, Grader* grader, unsigned int height, unsigned int height_building);
    virtual ~AlphaBetaAlg();
    virtual Move* decideMove();
    virtual void decideMove(Move** move);
    virtual void decideMove(Move** move, unsigned int time);
    virtual void makeMove(Move* move);
    virtual std::string stats();

private:
    Grader* grader;
    TranspositionTable* transTable;
    Hasher* hasher;
    std::mt19937 generator; // do wybierania sposrod najlepiej punktowanych ruchow
    unsigned int height;
    unsigned int height_building;

    unsigned int visited_nodes;
    unsigned int level_finished;

    boost::posix_time::ptime start_time; // start time dla decideMove z czasem    
    unsigned int timeToMove;
    bool alert; // jesli true, nalezy szybko konczyc obliczenia, aby zdazyc przed czasem

    void updateWindow(int best, int cp , int* alpha, int* beta);
    bool outsideWindow(int best, int cp, int alpha, int beta);
    int alpha_beta(Move* move, int alpha, int beta, unsigned int height);
    int alpha_beta_opt(unsigned int mindex, int alpha, int beta, unsigned int height);
    int getHashedValue(unsigned int height);
    void setHashedValue(int grade, unsigned int height);

    bool a_bet_is_won(int size);
};

#endif
