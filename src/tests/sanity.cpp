#include "splits.h"

#include <cstdio>

#include <vector>

using namespace std;

int main(int argc, char** argv)
{
    argc = argc; argv = argv;

    SplitsGame game;

    while(game.isFinished() != 2)
    {
        game.makeMove(game.getPossibleMoves()[0]);
    }

    printf("Wygral gracz %d\n", game.isFinished());

    while(game.undoMove() == 0);
    
    return 0;
}
