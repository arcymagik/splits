#include "adv_grader.h"

using namespace std;

/*
  ilosc zetonow w niezablokowanych stackach, ktore potencjalnie mozna jescze ruszyc
  minus to samo u przeciwnika
 */

#define GRADE_INFINITY (1 << 10)
#define INVALID_GRADE (GRADE_INFINITY+1)

int AdvancedGrader::grade(SplitsGame* game)
{
    int cps = game->curPlayerSign();
    vector<int>* stacks = getStacks(game);

    int result = 0;
    if (game->isFinished()) return - cps * GRADE_INFINITY;
    unsigned int fConts;

    for (int pl = 0; pl < 2; ++pl)
    {
        int pls = -(pl*2-1);
        for (unsigned int i = 0; i < stacks[pl].size(); ++i)
        {
            fConts = game->freeContacts(stacks[pl][i]);
            if (fConts > 0)
                result += (pls*(board(game, stacks[pl][i])->stack - 1))*(50 + fConts);
        }
    }

    return result;
}

AdvancedGrader::AdvancedGrader() {}
AdvancedGrader::~AdvancedGrader() {}

