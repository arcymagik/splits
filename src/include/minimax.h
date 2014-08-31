#ifndef MINIMAX_HH
#define MINIMAX_HH

#include "game_algorithm.h"
#include <random>

class MiniMaxAlg : public Algorithm
{
public:
    MiniMaxAlg();
    MiniMaxAlg(unsigned int seed, Grader* grader, unsigned int height, unsigned int height_building);
    virtual ~MiniMaxAlg();
    virtual Move* decideMove();
    virtual void decideMove(Move** move);
    virtual std::string stats();
private:
    Grader* grader;
    unsigned int height;
    unsigned int height_building;

    unsigned int visited_nodes;
    bool debug_bool;

    std::mt19937 generator; // do wybierania sposrod najlepiej punktowanych ruchow

    int minimax(Move* move, unsigned int h);
    int minimax_opt(Move* move, unsigned int h);

    bool a_bet_is_won(int size);
};

#endif
