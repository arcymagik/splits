#ifndef MONTE_CARLO_TREE_SEARCH_HH
#define MONTE_CARLO_TREE_SEARCH_HH

#include "game_algorithm.h"
#include "mc_utils.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>

class MCT_Node
{
public:
    SimulationResult simResult;
    MCT_Node* sons;
    unsigned int sons_size;

    MCT_Node();
    MCT_Node(const MCT_Node& another);
    virtual ~MCT_Node();
    void destroy_sons();
    unsigned int chooseSon(unsigned int cps);
    unsigned int chooseBestSimResult(unsigned int cps);
    int simulate(SplitsGame* game, std::mt19937* generator); // zwraca wynik rozgrywki
    void expand(SplitsGame* game);
    int randomGame(SplitsGame* game, std::mt19937* generator);
};

class MCTS : public Algorithm
{
public:
    MCTS(int seed);
    virtual ~MCTS();
    virtual void decideMove(Move** move);
    virtual void decideMove(Move** move, unsigned int time);
    virtual void makeMove(Move* move);

    void doComputing(unsigned int time);    
    void performSingleComputation();
private:
    MCT_Node tree;
    std::mt19937 generator;
};

#endif
