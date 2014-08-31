#ifndef MONTE_CARLO_SIMPLE_HH
#define MONTE_CARLO_SIMPLE_HH

#include "game_algorithm.h"
#include "mc_utils.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>

class MonteCarloMethod : public Algorithm
{
public:
    MonteCarloMethod(int seed);
    MonteCarloMethod(int seed, bool usingTrustLimit);
    virtual ~MonteCarloMethod();
    virtual void decideMove(Move** move);
    virtual void decideMove(Move** move, unsigned int time);
private:
    std::mt19937 generator;
    bool usingTrustLimit;

    unsigned int chooseSon(SimulationResult* v, SimulationResult* sons, unsigned int size, int cps);
    void oneSimulation(int i, SimulationResult* results, SimulationResult* aggr);

    void printResults(SimulationResult* results, unsigned int size);
};

#endif
