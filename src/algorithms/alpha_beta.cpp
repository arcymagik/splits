#include "alpha_beta.h"
#include "simple_grader.h"

#define INFINITY (1 << 20)

using namespace std;

AlphaBetaAlg::AlphaBetaAlg()
{
    grader = new SimpleGrader();
    height = 3;
    height_building = 0;
}

AlphaBetaAlg::AlphaBetaAlg(Grader* grader, unsigned int height, unsigned int height_building)
{
    this->grader = grader;
    this->height = height;
    this->height_building = height_building;
}

AlphaBetaAlg::~AlphaBetaAlg()
{
    delete grader;
}

Move* AlphaBetaAlg::decideMove()
{
    visited_nodes = 1;
    vector<Move*> moves = game.getPossibleMoves();
    unsigned int size = moves.size();
    GamePhase phase = game.gamePhase();
    int cp = game.curPlayer();
    unsigned int h = phase == Building ? height_building : height;

    unsigned int bestIndex = INFINITY;
    int alpha = -INFINITY;
    int beta = INFINITY;
    int best = -game.curPlayerSign()*INFINITY;
    int grade;

    for (unsigned int i = 0; i < size; ++i)
    {
        grade = alpha_beta(moves[i], alpha, beta, h);
        if (grader->better(&game, grade, best))
        {
            best = grade;
            bestIndex = i;
            updateWindow(best, cp, &alpha, &beta);
        }
    }
    Move* move = moves[bestIndex]->copy();
    for (unsigned int i = 0; i < moves.size(); ++i) delete(moves[i]);
    return move;
}

void AlphaBetaAlg::decideMove(Move** best_move)
{
    visited_nodes = 1;
    unsigned int size, an_size;
    void* moves = game.getPossibleMoves(&size);
    GamePhase phase = game.gamePhase();
    int cp = game.curPlayer();
    unsigned int h = phase == Building ? height_building : height;

    unsigned int bestIndex = 0;
    int alpha = -INFINITY;
    int beta = INFINITY;
    Move* move;
    int best = -game.curPlayerSign()*INFINITY;
    int grade;

    for (unsigned int i = 1; i < size; ++i)
    {
        moves = game.getPossibleMoves(&an_size);
        move = SplitsGame::rawPossibleMoveOfIndex(moves, i, phase);
        grade = alpha_beta_opt(move, alpha, beta, h);
        if (grader->better(&game, grade, best))
        {
            best = grade;
            bestIndex = i;
            updateWindow(best, cp, &alpha, &beta);
        }
    }
    moves = game.getPossibleMoves(&an_size);
    *best_move = SplitsGame::rawPossibleMoveOfIndex(moves, bestIndex, phase);
}

void AlphaBetaAlg::updateWindow(int best, int cp , int* alpha, int* beta)
{
    if (cp == 0) // dla wierzcholka max
    {
        if (best > *alpha) *alpha = best; // MAYBE moze lepiej tu uzywac better() gradera?
    }
    else // min
    {
        if (best < *beta) *beta = best;
    }
}

bool AlphaBetaAlg::outsideWindow(int best, int cp, int alpha, int beta)
{
    if (cp == 0) //max
    {
        return best >= beta;
    }
    else //min
    {
        return best <= alpha;
    }
}

int AlphaBetaAlg::alpha_beta(Move* move, int alpha, int beta, unsigned int h)
{
    int result;
    ++visited_nodes;
    game.makeMove(move);
    {
        if (h == 0 || game.isFinished()) result = grader->grade(&game);
        else
        {
            vector<Move*> moves = game.getPossibleMoves();
            unsigned int size = moves.size();
            int cp = game.curPlayer();

            int alpha = -INFINITY;
            int beta = INFINITY;
            int best = -game.curPlayerSign()*INFINITY;
            updateWindow(best, cp, &alpha, &beta);
            int grade;

            for (unsigned int i = 0; i < size; ++i)
            {
                grade = alpha_beta(moves[i], alpha, beta, h-1);
                if (grader->better(&game, grade, best))
                {
                    best = grade;
                    if (outsideWindow(best, cp, alpha, beta)) break;
                    updateWindow(best, cp, &alpha, &beta);
                }
            }
            result = best;
            for (unsigned int i = 0; i < size; ++i) delete(moves[i]);
        }
    }
    game.undoMove();
    return result;
}

int AlphaBetaAlg::alpha_beta_opt(Move* move, int alpha, int beta, unsigned int h)
{
    int result;
    ++visited_nodes;
    game.makeMove(move);
    {
        if (h == 0 || game.isFinished()) result = grader->grade(&game);
        else
        {
            unsigned int size, an_size;
            void* moves = game.getPossibleMoves(&size);
            GamePhase phase = game.gamePhase();
            int cp = game.curPlayer();

            int alpha = -INFINITY;
            int beta = INFINITY;
            int best = -game.curPlayerSign()*INFINITY;
            updateWindow(best, cp, &alpha, &beta);
            int grade;
            Move* move;

            for (unsigned int i = 0; i < size; ++i)
            {
                moves = game.getPossibleMoves(&an_size);
                move = SplitsGame::rawPossibleMoveOfIndex(moves, i, phase);
                grade = alpha_beta_opt(move, alpha, beta, h-1);
                if (grader->better(&game, grade, best))
                {
                    best = grade;
                    if (outsideWindow(best, cp, alpha, beta)) break;
                    updateWindow(best, cp, &alpha, &beta);
                }
            }
            result = best;
        }
    }
    game.undoMove();
    return result;
}

