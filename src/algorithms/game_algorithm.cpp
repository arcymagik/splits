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
