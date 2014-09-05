#include "simple_grader.h"

using namespace std;

/*
  ilosc zetonow w niezablokowanych stackach, ktore potencjalnie mozna jescze ruszyc
  minus to samo u przeciwnika
 */

#define GRADE_INFINITY (1 << 10)
#define INVALID_GRADE (GRADE_INFINITY+1)

int SimpleGrader::grade(SplitsGame* game)
{
    //vector<Move*> moves = game->getPossibleMoves();
    // int cp = game->curPlayer();
    // int cps = -(cp*2-1); // gracz 0 ma znak +, gracz 1 -
    int cps = game->curPlayerSign();
    vector<int>* stacks = getStacks(game);

    if (game->isFinished()) return - cps * GRADE_INFINITY;

    int result = 0;

    for (int pl = 0; pl < 2; ++pl)
    {
        int pls = -(pl*2-1);
        for (unsigned int i = 0; i < stacks[pl].size(); ++i)
        {
            if (game->freeContacts(stacks[pl][i]) > 0)
                result += pls*(board(game, stacks[pl][i])->stack - 1);
        }
    }

    return result;
}

SimpleGrader::SimpleGrader() {}
SimpleGrader::~SimpleGrader() {}

