#include "game_algorithm.h"

using namespace std;

Algorithm::Algorithm() {}
Algorithm::~Algorithm() {}

void Algorithm::makeMove(Move* move)
{
    game.makeMove(move);
}

string Algorithm::stats()
{
    string result = "";
    return result;
}

void Algorithm::decideMove(Move** move, unsigned int time)
{
    decideMove(move);
}

Move* Algorithm::decideMove()
{
    return NULL;
}

string Algorithm::gameBoardDesc()
{
    return game.getBoardDesc();
}
