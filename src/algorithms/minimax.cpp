#include "minimax.h"
#include "simple_grader.h"

#include <cstdio>

#include <boost/lexical_cast.hpp>

using namespace std;

MiniMaxAlg::MiniMaxAlg()
{
    generator.seed(422);
    grader = new SimpleGrader();
    height = 2;
    height_building = 0;
}

MiniMaxAlg::MiniMaxAlg(unsigned int seed, Grader* grader, unsigned int height, unsigned int height_building)
{
    generator.seed(seed);
    this->grader = grader;
    this->height = height;
    this->height_building = height_building;
}

MiniMaxAlg::~MiniMaxAlg()
{
    delete grader;
}

void MiniMaxAlg::decideMove(Move** best_move)
{
    debug_bool = true;
    visited_nodes = 1;
    unsigned int size, an_size;
    void* moves = game.getPossibleMoves(&size);
    GamePhase phase = game.gamePhase();
    unsigned int h = phase == Building ? height_building : height;
    unsigned int bestIndex = 0;
    Move* move = SplitsGame::rawPossibleMoveOfIndex(moves, bestIndex, phase);
    int best = minimax_opt(move, h);
    int grade;
    int bests_size = 1;

    for (unsigned int i = 1; i < size; ++i)
    {
        moves = game.getPossibleMoves(&an_size);
        move = SplitsGame::rawPossibleMoveOfIndex(moves, i, phase);
        grade = minimax_opt(move, h);
        if (grader->better(&game, grade, best))
        {
            best = grade;
            bestIndex = i;
            bests_size = 1;
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
}

Move* MiniMaxAlg::decideMove()
{
    debug_bool = true;
    visited_nodes = 1;
    vector<Move*> moves = game.getPossibleMoves();
    unsigned int size = moves.size();
    unsigned int h = game.gamePhase() == Building ? height_building : height;

    unsigned int bestIndex = 0;
    int best = minimax(moves[bestIndex], h);
    int grade;

    for (unsigned int i = 1; i < size; ++i)
    {
        grade = minimax(moves[i], h);
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
    //string shouldBe = game.getDesc();
    {
        if (h == 0 || game.isFinished())
        {
            result = grader->grade(&game);
        }
        else
        {
            unsigned int size, an_size;
            void* moves = game.getPossibleMoves(&size);
            GamePhase phase = game.gamePhase();
            Move* move = SplitsGame::rawPossibleMoveOfIndex(moves, 0, phase);
            int best = minimax_opt(move, h-1);
            int grade;
            for (unsigned int i = 1; i < size; ++i)
            {
                moves = game.getPossibleMoves(&an_size);
                // if (debug_bool && (an_size != size || game.stacksWrong())) {
                //     printf("mmark!!!!!! %u at %u:%u, and size is %u\n", an_size, h, i, size);
                //     printf("plansze::: %s\n%s\n====================================================\n", shouldBe.c_str(), game.getDesc().c_str());
                //     debug_bool = false;
                // }
                move = SplitsGame::rawPossibleMoveOfIndex(moves, i, phase);
                grade = minimax_opt(move, h-1);
                if (grader->better(&game, grade, best)) best = grade;
            }
            moves = game.getPossibleMoves(&an_size);

            // if (debug_bool && (an_size != size || game.stacksWrong())) {
            //     printf("mmarkk!!!!!! %u at %u:%u, and size is %u\n", an_size, h, size, size);
            //     printf("plansze::: %s\n%s\n====================================================\n", shouldBe.c_str(), game.getDesc().c_str());
            //     debug_bool = false;
            // }
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
    result += game.getDesc();
    return result;
}

bool MiniMaxAlg::a_bet_is_won(int size)
{
    uniform_int_distribution<> dis(0, size-1);
    return (dis(generator) == 0); // 1/size probability to win a bet
}
