#include "minimax.h"
#include "simple_grader.h"

using namespace std;

MiniMaxAlg::MiniMaxAlg()
{
    grader = new SimpleGrader();
    height = 2;
}

MiniMaxAlg::MiniMaxAlg(Grader* grader, unsigned int height)
{
    this->grader = grader;
    this->height = height;
}

MiniMaxAlg::~MiniMaxAlg()
{
    delete grader;
}

Move* MiniMaxAlg::decideMove()
{
    vector<Move*> moves = game.getPossibleMoves();
    unsigned int size = moves.size();

    unsigned int bestIndex = 0;
    int best = minimax(moves[bestIndex], height);
    int grade;

    for (unsigned int i = 1; i < size; ++i)
    {
        grade = minimax(moves[i], height);
        if (grader->better(&game, grade, best)) // TODO: lepiej wybierac losowy z najlepszych
        {
            best = grade;
            bestIndex = i;
        }
    }
    Move* move = moves[bestIndex]->copy();
    for (unsigned int i = 0; i < moves.size(); ++i) delete(moves[i]); // TODO - te ciagle alokacje/dealokacje musza sie skonczyc
    return move;
}

int MiniMaxAlg::minimax(Move* move, unsigned int h)
{
    int result;
    game.makeMove(move);
    {
        if (h == 0 || game.isFinished())
        {
            result = grader->grade(&game);
        }
        else
        {
            vector<Move*> moves = game.getPossibleMoves();
            unsigned int size = moves.size();
            int best = minimax(moves[0], h-1);
            int grade;
            for (unsigned int i = 1; i < size; ++i)
            {
                grade = minimax(moves[i], h-1);
                if (grader->better(&game, grade, best)) best = grade;
            }
            result = best;
            for (unsigned int i = 0; i < moves.size(); ++i) delete(moves[i]); // TODO - te ciagle alokacje/dealokacje musza sie skonczyc
        }
    }
    game.undoMove();
    return result;
}
