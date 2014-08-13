#include "mcts.h"

#include <random>
#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace std;

#define MS_DANGER_ZONE (100)
#define ONE_STEP_NO_SIMULATIONS (200)
#define EXPAND_TRESHOLD (30)

MCTS::MCTS(int seed)
{
    generator.seed(seed);
    tree.expand(&game);
}

MCTS::~MCTS() {}

void MCTS::decideMove(Move** move)
{
    decideMove(move, 1000);
}

void MCTS::decideMove(Move** move, unsigned int time)
{
    doComputing(time);
    unsigned int mindex = tree.chooseBestSimResult(game.curPlayerSign());
    unsigned int size;
    void* moves = game.getPossibleMoves(&size);
    *move = SplitsGame::rawPossibleMoveOfIndex(moves, mindex, game.gamePhase());
}

MCT_Node::MCT_Node()
{
    sons = NULL;
    simResult.wins = simResult.total = 0;
    sons_size = 0;
}

MCT_Node::~MCT_Node()
{
    destroy_sons();
}

void MCT_Node::destroy_sons()
{
    for (unsigned int i = 0; i < sons_size; ++i)
        sons[i].destroy_sons();
    free(sons);
    sons = NULL;
    sons_size = 0;
}

void MCTS::makeMove(Move* move)
{
    unsigned int mindex = game.getIndexOfMove(move);
    for (unsigned int i = 0; i < tree.sons_size; ++i)
        if (i != mindex) tree.sons[i].destroy_sons();
    MCT_Node tmp = tree.sons[mindex];
    tree.sons[mindex].sons_size = 0;
    tree.sons[mindex].sons = NULL;
    tree.destroy_sons();
    tree = tmp;
    tmp.sons = NULL;
    tmp.sons_size = 0;
    if (tree.sons == NULL)
        tree.expand(&game);
    game.makeMove(move);
}

void MCTS::doComputing(unsigned int time)
{
    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    unsigned int time_passed;

    do
    {
        for (unsigned int i = 0; i < ONE_STEP_NO_SIMULATIONS; ++i)
        {
            performSingleComputation();
        }
        time_passed = (boost::posix_time::microsec_clock::local_time() - start_time).total_milliseconds();
    } while (time_passed + MS_DANGER_ZONE < time);
}

void MCTS::performSingleComputation()
{
    tree.simulate(&game, &generator);
}

int MCT_Node::simulate(SplitsGame* game, mt19937* generator)
{
    if (game->isFinished()) return game->getWinner();

    int result;
    if (sons == NULL)
    {
        if (simResult.total >= EXPAND_TRESHOLD)
        {
            expand(game);
            unsigned int mindex = chooseSon(game->curPlayerSign());
            game->makeIndexedMove(mindex);
            result = sons[mindex].simulate(game, generator);
            game->undoMove();
        }
        else result = randomGame(game, generator);
    }
    else
    {
        unsigned int mindex = chooseSon(game->curPlayerSign());
        game->makeIndexedMove(mindex);
        result = sons[mindex].simulate(game, generator);
        game->undoMove();
    }
    simResult.wins += (result ^ 1);
    simResult.total++;
    return result;
}

int MCT_Node::randomGame(SplitsGame* game, mt19937* generator)
{
    void* moves;
    unsigned int size;
    Move* move;
    unsigned int no_moves = 0;
    while (!game->isFinished())
    {
        moves = game->getPossibleMoves(&size);
        uniform_int_distribution<> dis(0, size-1);
        move = SplitsGame::rawPossibleMoveOfIndex(moves, dis(*generator), game->gamePhase());
        game->makeMove(move);
        ++no_moves;
    }
    int result = game->getWinner();
    for (unsigned int i = 0; i < no_moves; ++i) game->undoMove();
    return result;
}

unsigned int MCT_Node::chooseBestSimResult(unsigned int cps)
{
    unsigned int best = 0;
    for (unsigned int i = 1; i < sons_size; ++i)
    {
        if (sons[i].simResult.isBetterThan(&(sons[best].simResult), cps))
            best = i;
    }
    return best;
}

void MCT_Node::expand(SplitsGame* game)
{
    unsigned int size;
    game->getPossibleMoves(&size);
    sons = (MCT_Node*) malloc(sizeof(MCT_Node)*size);
    for (unsigned int i = 0; i < size; ++i)
        sons[i] = MCT_Node();
    sons_size = size;
}

unsigned int MCT_Node::chooseSon(unsigned int cps)
{
    unsigned int result = 0;
    double tlimit = simResult.trustLimit(&(sons[0].simResult), cps);
    double temp;
    for (unsigned int i = 1; i < sons_size; ++i)
    {
        temp = simResult.trustLimit(&(sons[i].simResult), cps);
        if (((double) cps)*(temp-tlimit) > 0)
        {
            tlimit = temp;
            result = i;
        }
    }
    return result;
}
