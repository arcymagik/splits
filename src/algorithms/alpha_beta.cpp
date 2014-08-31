#include "alpha_beta.h"
#include "simple_grader.h"

#include <boost/lexical_cast.hpp>

#define GRADE_INFINITY (1 << 20)
#define INVALID_GRADE (GRADE_INFINITY+1)

#define GRADE_BIT_LENGTH (22)
#define HEIGHT_BIT_LENGTH (4)

#define MS_TO_ALERT (100)
#define VISITED_NODES_TO_CHECK_MASK (0xff)

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
    timeToMove = 0;
}

AlphaBetaAlg::AlphaBetaAlg(unsigned int seed, Grader* grader, unsigned int height, unsigned int height_building)
{
    generator.seed(seed);
    this->grader = grader;
    this->height = height;
    this->height_building = height_building;
    transTable = NULL;
    hasher = NULL;
    timeToMove = 0;
}

AlphaBetaAlg::AlphaBetaAlg(unsigned int seed, TranspositionTable* transTable, Hasher* hasher, Grader* grader, unsigned int height, unsigned int height_building)
{
    generator.seed(seed);
    this->grader = grader;
    this->height = height;
    this->height_building = height_building;
    this->transTable = transTable;
    this->hasher = hasher;
    timeToMove = 0;
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

    unsigned int bestIndex = GRADE_INFINITY;
    int alpha = -GRADE_INFINITY;
    int beta = GRADE_INFINITY;
    int best = -game.curPlayerSign()*GRADE_INFINITY;
    int grade;
    int bests_size = 0;

    for (unsigned int i = 0; i < size; ++i)
    {
        grade = alpha_beta(moves[i], alpha, beta, h);
        if (grader->better(&game, grade, best))
        {
            best = grade;
            bests_size = 1;
            bestIndex = i;
            updateWindow(best, cp, &alpha, &beta);
        }
        else if (grade == best)
        {
            ++bests_size;
            if (a_bet_is_won(bests_size))
            {
                bestIndex = i;
            }
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
    int alpha = -GRADE_INFINITY;
    int beta = GRADE_INFINITY;
    int best = -game.curPlayerSign()*GRADE_INFINITY;
    int grade;
    int bests_size = 0;

    for (unsigned int i = 1; i < size; ++i)
    {
        moves = game.getPossibleMoves(&an_size);
        grade = alpha_beta_opt(i, alpha, beta, h);
        if (grader->better(&game, grade, best))
        {
            best = grade;
            bestIndex = i;
            bests_size = 1;
            updateWindow(best, cp, &alpha, &beta);
        }
        else if (grade == best)
        {
            ++bests_size;
            if (a_bet_is_won(bests_size))
            {
                bestIndex = i;
            }
        }
    }
    moves = game.getPossibleMoves(&an_size);
    *best_move = SplitsGame::rawPossibleMoveOfIndex(moves, bestIndex, phase);
    //if (height < 6) printf("expected grade: %d\n", best);
}

void AlphaBetaAlg::decideMove(Move** move, unsigned int time)
{
    alert = false;
    start_time = boost::posix_time::microsec_clock::local_time();
    timeToMove = time;
    unsigned int time_passed;
    Move* result;
    unsigned int i = 0;
    *move = NULL;
    level_finished = 0;
    while (true)
    {
        height_building = height = i;
        decideMove(&result);
        auto current_time = boost::posix_time::microsec_clock::local_time() - start_time;
        time_passed = current_time.total_milliseconds();
        if (time_passed + MS_TO_ALERT > timeToMove) alert = true;
        if (alert) break;
        level_finished = height;
        *move = result;
        ++i;
    }
    if (*move == NULL) *move = result;
    alert = false;
    timeToMove = 0;
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

            int alpha = -GRADE_INFINITY;
            int beta = GRADE_INFINITY;
            int best = -game.curPlayerSign()*GRADE_INFINITY;
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
    unsigned int time_passed = 0;
    if (alert) return 0;
    if (timeToMove > 0 && ((visited_nodes & VISITED_NODES_TO_CHECK_MASK) == 0))
    {
        auto current_time = boost::posix_time::microsec_clock::local_time() - start_time;
        time_passed = current_time.total_milliseconds();
        if (time_passed + MS_TO_ALERT > timeToMove)
        {
            alert = true;
            return 0;
        }
    }
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

            int alpha = -GRADE_INFINITY;
            int beta = GRADE_INFINITY;
            int best = -game.curPlayerSign()*GRADE_INFINITY;
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

    if (height == hheight) return ((int) hgrade) - GRADE_INFINITY;
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
        | (unsigned long long) (grade + GRADE_INFINITY);
    transTable->push(entry);
}

void AlphaBetaAlg::makeMove(Move* move)
{
    if (hasher != NULL) hasher->makeMove(move, &game, game.gamePhase());
    game.makeMove(move);
}

string AlphaBetaAlg::stats()
{
    string result = "alphabeta stats:\n";
    result += "\tvisited nodes: ";
    result += boost::lexical_cast<string>(visited_nodes);
    result += "\n";
    result += "\tlevels_finished: ";
    result += boost::lexical_cast<string>(level_finished);
    return result;
}

bool AlphaBetaAlg::a_bet_is_won(int size)
{
    uniform_int_distribution<> dis(0, size-1);
    return (dis(generator) == 0); // 1/size probability to win a bet
}
