#include "monte_carlo.h"

#include <random>
#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace std;

#define MS_DANGER_ZONE (100)
#define ONE_STEP_NO_SIMULATIONS (200)

MonteCarloMethod::MonteCarloMethod(int seed)
{
    generator.seed(seed);
    usingTrustLimit = false;
}

MonteCarloMethod::MonteCarloMethod(int seed, bool usingTrustLimit)
{
    generator.seed(seed);
    this->usingTrustLimit = usingTrustLimit;
}

MonteCarloMethod::~MonteCarloMethod() {}

void MonteCarloMethod::decideMove(Move** move)
{
    return decideMove(move, 1000);
}

void MonteCarloMethod::decideMove(Move** move, unsigned int timeToMove)
{
    unsigned int size;
    void* moves = game.getPossibleMoves(&size);
    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    unsigned int time_passed;
    SimulationResult* results = (SimulationResult*) malloc(sizeof(SimulationResult)*size);
    SimulationResult v;
    v.total = v.wins = 0;
    for (unsigned int i = 0; i < size; ++i) results[i].total = results[i].wins = 0;

    do
    {
        for (unsigned int i = 0; i < ONE_STEP_NO_SIMULATIONS; ++i)
        {
            oneSimulation(chooseSon(&v, results, size, game.curPlayerSign()), results, &v);
        }
        time_passed = (boost::posix_time::microsec_clock::local_time() - start_time).total_milliseconds();
    } while (time_passed + MS_DANGER_ZONE < timeToMove);

    unsigned int index = chooseBestSimResult(results, size, game.curPlayerSign());
    moves = game.getPossibleMoves(&size);
    free(results);
    *move = SplitsGame::rawPossibleMoveOfIndex(moves, index, game.gamePhase());
}

unsigned int MonteCarloMethod::chooseSon(SimulationResult* v, SimulationResult* sons, unsigned int size, int cps)
{
    if (usingTrustLimit)
    {
        return v->chooseSon(sons, size, cps);
    }
    else
    {
        uniform_int_distribution<> dis(0, size-1);
        return dis(generator);
    }
}

unsigned int MonteCarloMethod::chooseBestSimResult(SimulationResult* results, unsigned int size, unsigned int cps)
{
    unsigned int best = 0;
    for (unsigned int i = 1; i < size; ++i)
    {
        if (!results[i].isBetterThan(results + best, cps)) best = i;
    }
    return best;
}

void MonteCarloMethod::oneSimulation(int i, SimulationResult* results, SimulationResult* aggr)
{
    unsigned int no_moves = 0;
    unsigned int size;
    void* moves = game.getPossibleMoves(&size);
    int winner;
    Move* move = SplitsGame::rawPossibleMoveOfIndex(moves, i, game.gamePhase());
    game.makeMove(move);
    {
        while(!game.isFinished())
        {
            moves = game.getPossibleMoves(&size);
            uniform_int_distribution<> dis(0, size-1);
            move = SplitsGame::rawPossibleMoveOfIndex(moves, dis(generator), game.gamePhase());
            game.makeMove(move);
            ++no_moves;
        }
        winner = game.getWinner();
        for (unsigned int i = 0; i < no_moves; ++i) game.undoMove();
    }
    game.undoMove();

    results[i].total++; aggr->total++;
    results[i].wins += winner ^ 1; aggr->wins += winner ^ 1;
    //if (winner == game.curPlayer()) results[i].wins++;
}

