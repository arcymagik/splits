#include "alpha_beta.h"
#include "simple_grader.h"

#define INFINITY (1 << 20)
#define INVALID_GRADE (INFINITY+1)

#define GRADE_BIT_LENGTH 22
#define HEIGHT_BIT_LENGTH 4

using namespace std;

const unsigned long long grade_mask = ~((~0) << GRADE_BIT_LENGTH);
const unsigned long long height_mask = (~((~0) << (HEIGHT_BIT_LENGTH +GRADE_BIT_LENGTH))) ^ grade_mask;

AlphaBetaAlg::AlphaBetaAlg()
{
    grader = new SimpleGrader();
    height = 3;
    height_building = 0;
    transTable = NULL;
    hasher = NULL;
}

AlphaBetaAlg::AlphaBetaAlg(Grader* grader, unsigned int height, unsigned int height_building)
{
    this->grader = grader;
    this->height = height;
    this->height_building = height_building;
    transTable = NULL;
    hasher = NULL;
}

AlphaBetaAlg::AlphaBetaAlg(TranspositionTable* transTable, Hasher* hasher, Grader* grader, unsigned int height, unsigned int height_building)
{
    this->grader = grader;
    this->height = height;
    this->height_building = height_building;
    this->transTable = transTable;
    this->hasher = hasher;
}

AlphaBetaAlg::~AlphaBetaAlg()
{
    delete hasher;
    delete transTable;
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
    int best = -game.curPlayerSign()*INFINITY;
    int grade;

    for (unsigned int i = 1; i < size; ++i)
    {
        moves = game.getPossibleMoves(&an_size);
        grade = alpha_beta_opt(i, alpha, beta, h);
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
    if (hasher != NULL) hasher->makeMove(move, &game, game.gamePhase());
    game.makeMove(move);
    result = getHashedValue(h);
    if (result == INVALID_GRADE)
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
        setHashedValue(result, h);
    }
    game.undoMove();
    if (hasher != NULL) hasher->undoMove(move, &game, game.gamePhase());
    return result;
}

int AlphaBetaAlg::alpha_beta_opt(unsigned int mindex, int alpha, int beta, unsigned int h)
{
    int result;
    ++visited_nodes;
    unsigned int ssize;
    void* moves = game.getPossibleMoves(&ssize);
    Move* mmove = SplitsGame::rawPossibleMoveOfIndex(moves, mindex, game.gamePhase());
    if (hasher != NULL) hasher->makeMove(mmove, &game, game.gamePhase());
    game.makeMove(mmove);
    result = getHashedValue(h);
    if (result == INVALID_GRADE)
    {
        if (h == 0 || game.isFinished()) result = grader->grade(&game);
        else
        {
            unsigned int size;
            game.getPossibleMoves(&size);
            int cp = game.curPlayer();

            int alpha = -INFINITY;
            int beta = INFINITY;
            int best = -game.curPlayerSign()*INFINITY;
            updateWindow(best, cp, &alpha, &beta);
            int grade;

            for (unsigned int i = 0; i < size; ++i)
            {
                grade = alpha_beta_opt(i, alpha, beta, h-1);
                if (grader->better(&game, grade, best))
                {
                    best = grade;
                    if (outsideWindow(best, cp, alpha, beta)) break;
                    updateWindow(best, cp, &alpha, &beta);
                }
            }
            result = best;
        }
        setHashedValue(result, h);
    }
    game.undoMove();
    moves = game.getPossibleMoves(&ssize);
    mmove = SplitsGame::rawPossibleMoveOfIndex(moves, mindex, game.gamePhase());
    if (hasher != NULL) hasher->undoMove(mmove, &game, game.gamePhase());
    return result;
}

int AlphaBetaAlg::getHashedValue(unsigned int height)
{
    if (hasher == NULL) return INVALID_GRADE;

    unsigned long long hash = hasher->getHash();
    TTEntry entry = transTable->get(hasher->getHash());
    if (entry.hash != hash) return INVALID_GRADE;

    unsigned long long hheight = entry.data >> GRADE_BIT_LENGTH;
    unsigned long long hgrade = entry.data & grade_mask;

    if (height == hheight) return ((int) hgrade) - INFINITY;
    else return INVALID_GRADE;
}

void AlphaBetaAlg::setHashedValue(int grade, unsigned int height)
{
    if (hasher == NULL) return;

    unsigned long long hash = hasher->getHash();
    TTEntry entry;
    entry.hash = hash;
    entry.data =
        (((unsigned long long)height) << HEIGHT_BIT_LENGTH)
        | (unsigned long long) (grade + INFINITY);
    transTable->push(entry);
}

void AlphaBetaAlg::makeMove(Move* move)
{
    game.makeMove(move);
    if (hasher != NULL) hasher->makeMove(move, &game, game.gamePhase());
}

