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
    virtual std::string stats();

protected:
    SplitsGame game;
};

#endif
