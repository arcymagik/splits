#ifndef MONTE_CARLO_UTILITIES_HH
#define MONTE_CARLO_UTILITIES_HH

class SimulationResult
{
public:
    unsigned int wins; // ilosc wygranych gracza 0 (a NIE aktualnego)
    unsigned int total;

    bool isBetterThan(SimulationResult* another, int cps); // lepszy z punktu widzenia cp
    double trustLimit(SimulationResult* son, int cps);
    unsigned int chooseSon(SimulationResult* sons, unsigned int size, int cps);
};

#endif
