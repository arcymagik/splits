#include "alpha_beta.h"
#include "simple_grader.h"

#include <boost/lexical_cast.hpp>

#define GRADE_INFINITY (1 << 10)
#define INVALID_GRADE (GRADE_INFINITY+1)

#define GRADE_BIT_LENGTH (12)
#define HEIGHT_BIT_LENGTH (6)
#define LAST_BEST_LENGTH (8)

#define MS_TO_ALERT (100)
#define VISITED_NODES_TO_CHECK_MASK (0xff)

using namespace std;

const unsigned long long grade_mask = ~((~0) << GRADE_BIT_LENGTH);
const unsigned long long height_mask = (~((~0) << (HEIGHT_BIT_LENGTH +GRADE_BIT_LENGTH))) ^ grade_mask;
const unsigned long long last_best_mask = (~((~0) << (HEIGHT_BIT_LENGTH +GRADE_BIT_LENGTH +LAST_BEST_LENGTH))) ^ (grade_mask | height_mask);

unsigned long long maximal_height = 40; // gra nie moze byc dluzsza niz 39, wiec przyjmuje to za max zaglebiania sie

AlphaBetaAlg::AlphaBetaAlg()
{
    grader = new SimpleGrader();
    height = 3;
    height_building = 0;
    transTable = NULL;
    hasher = NULL;
    timeToMove = 0;
    choosing_best_first_son = false;
    cbf_warming = false;
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
    choosing_best_first_son = false;
    cbf_warming = false;
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
    choosing_best_first_son = false;
    cbf_warming = false;
}

AlphaBetaAlg::AlphaBetaAlg(unsigned int seed, TranspositionTable* transTable, Hasher* hasher, Grader* grader, unsigned int height, unsigned int height_building, bool choosing_best_first_son)
{
    generator.seed(seed);
    this->grader = grader;
    this->height = height;
    this->height_building = height_building;
    this->transTable = transTable;
    this->hasher = hasher;
    timeToMove = 0;
    this->choosing_best_first_son = choosing_best_first_son;
    cbf_warming = false;
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
    GamePhase phase = game.gamePhase();
    unsigned int h = phase == Building ? height_building : height;

    if (cbf_warming)
        ++visited_nodes;
    else
        visited_nodes = 1;
    cuts = 0;

    if (choosing_best_first_son && !cbf_warming)
    {
        unsigned int old_h = h;
        h  = h == 0 ? 0 : h-1;
        if (phase == Building) height_building = h;
        else height = h;

        cbf_warming = true;
        decideMove(best_move); // zapelnienie tablicy transpozycji
        cbf_warming = false;
        h = old_h;
        if (phase == Building) height_building = h;
        else height = h;
        printf("alphabeta: visited node is already %u\n", visited_nodes);
    }
    unsigned int size, an_size;
    void* moves = game.getPossibleMoves(&size);
    int cp = game.curPlayer();

    unsigned int firstChosen;
    getHashedValue(h+1, &firstChosen);
    unsigned int bestIndex = firstChosen;
    int alpha = -GRADE_INFINITY;
    int beta = GRADE_INFINITY;
    int best = -game.curPlayerSign()*GRADE_INFINITY;
    int grade;
    int bests_size = 0;

    for (unsigned int i = 0; i < size; ++i)
        if (i != firstChosen)
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
    setHashedValue(best, h+1, bestIndex);
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
    cbf_warming = choosing_best_first_son;
    visited_nodes = 0;
    while (true && i < maximal_height)
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
    cbf_warming  = false;
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
    bool result;
    if (cp == 0) //max
    {
        result = best >= beta;
    }
    else //min
    {
        result = best <= alpha;
    }
    if (result) ++cuts;
    return result;
}

int AlphaBetaAlg::alpha_beta(Move* move, int alpha, int beta, unsigned int h)
{
    int result;
    ++visited_nodes;
    if (hasher != NULL) hasher->makeMove(move, &game, game.gamePhase());
    game.makeMove(move);
    unsigned int last_best;
    result = getHashedValue(h, &last_best);
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
        setHashedValue(result, h, last_best);
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
    unsigned int firstChosen;
    unsigned int bestIndex = 0;
    void* moves = game.getPossibleMoves(&ssize);
    Move* mmove = SplitsGame::rawPossibleMoveOfIndex(moves, mindex, game.gamePhase());
    if (hasher != NULL) hasher->makeMove(mmove, &game, game.gamePhase());
    game.makeMove(mmove);
    result = getHashedValue(h, &firstChosen);
    if (result == INVALID_GRADE)
    {
        if (h == 0 || game.isFinished()) result = grader->grade(&game);
        else
        {
            unsigned int size;
            game.getPossibleMoves(&size);
            int cp = game.curPlayer();

            //int alpha = -GRADE_INFINITY;
            //int beta = GRADE_INFINITY;
            int best = alpha_beta_opt(firstChosen, alpha, beta, h-1);
            bestIndex = firstChosen;
            updateWindow(best, cp, &alpha, &beta);

            int grade;
            for (unsigned int i = 0; i < size; ++i)
                if (i != firstChosen)
                {
                    grade = alpha_beta_opt(i, alpha, beta, h-1);
                    if (grader->better(&game, grade, best))
                    {
                        bestIndex = i;
                        best = grade;
                        if (outsideWindow(best, cp, alpha, beta)) break;
                        updateWindow(best, cp, &alpha, &beta);
                    }
                }
            result = best;
        }
        setHashedValue(result, h, bestIndex);
    }
    game.undoMove();
    moves = game.getPossibleMoves(&ssize);
    mmove = SplitsGame::rawPossibleMoveOfIndex(moves, mindex, game.gamePhase());
    if (hasher != NULL) hasher->undoMove(mmove, &game, game.gamePhase());
    return result;
}

int AlphaBetaAlg::getHashedValue(unsigned int height, unsigned int* best_son_index)
{
    *best_son_index = 0; // 0 will be default, because it's always correct
    if (hasher == NULL) return INVALID_GRADE;

    unsigned long long hash = hasher->getHash();
    TTEntry entry = transTable->get(hash);
    if (entry.hash != hash) return INVALID_GRADE;

    unsigned long long hbest_last = (entry.data & last_best_mask) >> (GRADE_BIT_LENGTH + HEIGHT_BIT_LENGTH);
    unsigned long long hheight = (entry.data & height_mask) >> GRADE_BIT_LENGTH;
    unsigned long long hgrade = entry.data & grade_mask;

    if (height == hheight) return ((int) hgrade) - GRADE_INFINITY;
    else
    {
        if (choosing_best_first_son)
            *best_son_index = (unsigned int) hbest_last; // otherise the default 0
        return INVALID_GRADE;
    }
}

void AlphaBetaAlg::setHashedValue(int grade, unsigned int height, unsigned int best_son_index)
{
    if (hasher == NULL) return;

    unsigned long long hash = hasher->getHash();
    TTEntry entry;
    entry.hash = hash;
    entry.data =
        (((unsigned long long)best_son_index) << (HEIGHT_BIT_LENGTH + GRADE_BIT_LENGTH))
        | (((unsigned long long)height) << GRADE_BIT_LENGTH)
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
    result += "\tcuts done: ";
    result += boost::lexical_cast<string>(cuts);
    return result;
}

bool AlphaBetaAlg::a_bet_is_won(int size)
{
    uniform_int_distribution<> dis(0, size-1);
    return (dis(generator) == 0); // 1/size probability to win a bet
}
