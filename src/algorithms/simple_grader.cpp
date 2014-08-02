#include "simple_grader.h"

using namespace std;

/*
  ilosc zetonow w niezablokowanych stackach, ktore potencjalnie mozna jescze ruszyc
  minus to samo u przeciwnika
 */
int SimpleGrader::grade(SplitsGame* game)
{
    //vector<Move*> moves = game->getPossibleMoves();
    int cp = game->curPlayer();
    int cps = -(cp*2-1); // gracz 0 ma znak +, gracz 1 -
    vector<int>* stacks = getStacks(game);

    if (game->isFinished()) return cps * 100000;

    int result = 0;

    for (unsigned int pl = 0; pl < 2; ++pl)
    {
        int pls = -(pl*2-1);
        for (unsigned int i = 0; i < stacks[pl].size(); ++i)
        {
            if (!game->stackBlocked(stacks[pl][i])) result += pls*(board(game, stacks[pl][i])->stack - 1);
        }
    }

    return result;
}

SimpleGrader::SimpleGrader() {}
SimpleGrader::~SimpleGrader() {}

