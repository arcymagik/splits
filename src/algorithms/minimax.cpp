#include "minimax.h"
#include "simple_grader.h"

#include <boost/lexical_cast.hpp>

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

void MiniMaxAlg::decideMove(Move** best_move)
{
    visited_nodes = 1;
    unsigned int size;
    void* moves = game.getPossibleMoves(&size);
    GamePhase phase = game.gamePhase();
    unsigned int bestIndex = 0;
    Move* move = SplitsGame::rawPossibleMoveOfIndex(moves, bestIndex, phase);
    int best = minimax_opt(move, height);
    int grade;

    for (unsigned int i = 1; i < size; ++i)
    {
        move = SplitsGame::rawPossibleMoveOfIndex(moves, i, phase);
        grade = minimax_opt(move, height);
        if (grader->better(&game, grade, best)) // TODO: lepiej wybierac losowy z najlepszych
        {
            best = grade;
            bestIndex = i;
        }
    }
    *best_move = SplitsGame::rawPossibleMoveOfIndex(moves, bestIndex, phase);
}

Move* MiniMaxAlg::decideMove()
{
    visited_nodes = 1;
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
    ++visited_nodes;
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

int MiniMaxAlg::minimax_opt(Move* move, unsigned int h)
{
    int result;
    ++visited_nodes;
    game.makeMove(move);
    {
        if (h == 0 || game.isFinished())
        {
            result = grader->grade(&game);
        }
        else
        {
            unsigned int size;
            void* moves = game.getPossibleMoves(&size);
            GamePhase phase = game.gamePhase();
            Move* move = SplitsGame::rawPossibleMoveOfIndex(moves, 0, phase);
            int best = minimax_opt(move, h-1);
            int grade;
            for (unsigned int i = 1; i < size; ++i)
            {
                move = SplitsGame::rawPossibleMoveOfIndex(moves, i, phase);
                grade = minimax_opt(move, h-1);
                if (grader->better(&game, grade, best)) best = grade;
            }
            result = best;
        }
    }
    game.undoMove();
    return result;
}

string MiniMaxAlg::stats()
{
    string result = "";
    result += "visited_nodes: ";
    result += boost::lexical_cast<string>(visited_nodes);
    return result;
}
