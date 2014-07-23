#ifndef GAME_ALGORITHM_HH
#define GAME_ALGORITHM_HH

#include "splits.h"

class Algorithm
{
public:
    //void setGame(SplitsGame* game); // nie wiem, czy to dodawac - moze pozniej MAYBE
    Algorithm();
    ~Algorithm();
    void makeMove(Move* move);
    virtual Move* decideMove() = 0; // result tej metody nalezy do wolajacego

protected:
    SplitsGame game;
};

#endif
