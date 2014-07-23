#include "game_algorithm.h"

Algorithm::Algorithm() {}
Algorithm::~Algorithm() {}

void Algorithm::makeMove(Move* move)
{
    game.makeMove(move);
}
