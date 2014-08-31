#include "mc_utils.h"
#include <cstdio>
#include <cmath>

using namespace std;

bool SimulationResult::isBetterThan(SimulationResult* another, int cps)
{
    int number =
        cps*
        (wins * another->total
         -
         another->wins * total);
    //printf("number: %d\n", number);
    return number > 0;
}

double SimulationResult::trustLimit(SimulationResult* son, int cps)
{
    if (son->total == 0) return ((double) cps)*2.0;

    double estimate = ((double) son->wins) / ((double) son->total);
    double sigma = sqrt( log((double) total) / son->total);
    return estimate + sigma*cps;
}

unsigned int SimulationResult::chooseSon(SimulationResult* sons, unsigned int size, int cps)
{
    unsigned int result = 0;
    double tlimit = trustLimit(sons, cps);
    //printf("tlimit: %f\n", tlimit);
    double temp;
    for (unsigned int i = 1; i < size; ++i)
    {
        temp = trustLimit(sons+i, cps);
        if (((double)cps)*(temp - tlimit) > 0)
        {
            tlimit = temp;
            result = i;
        }
    }
//    printf("chosen: %f\n", tlimit);
    return result;
}

unsigned int SimulationResult::chooseBestSimResult(SimulationResult* results, unsigned int size, int cps)
{
    unsigned int best = 0;
    for (unsigned int i = 1; i < size; ++i)
    {
        if (results[i].isBetterThan(results + best, cps))
            best = i;
    }
    return best;
}
