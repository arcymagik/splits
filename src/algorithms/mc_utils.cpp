#include "mc_utils.h"

bool SimulationResult::isBetterThan(SimulationResult* another, int cps)
{
    return
        cps*
        (wins * another->total
         -
         another->wins * total)
        >
        0;
}

