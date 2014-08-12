#ifndef GAME_ALGORITHM_HH
#define GAME_ALGORITHM_HH

#include "splits.h"
#include <string>

class Algorithm
{
public:
    //void setGame(SplitsGame* game); // nie wiem, czy to dodawac - moze pozniej MAYBE
    Algorithm();
    virtual ~Algorithm();
    virtual void makeMove(Move* move);
    virtual Move* decideMove() = 0; // result tej metody nalezy do wolajacego
    virtual void decideMove(Move** move) = 0;
    virtual void decideMove(Move** move, unsigned int time); // ta metoda jest zdefiniowana glownie dla istniejacych algorytmow przykladowych. raczej powinna byc overrideowana
    virtual std::string stats();

protected:
    SplitsGame game;
};

#endif
