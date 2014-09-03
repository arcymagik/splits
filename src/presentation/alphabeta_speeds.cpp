#include "splits.h"
#include "random_game_algorithm.h"
#include "minimax.h"
#include "simple_grader.h"
#include "zobrist_hasher.h"
#include "alpha_beta.h"
#include "monte_carlo.h"
#include "mcts.h"

#include <cstdio>

#include <vector>
#include <random>
#include <iostream>

#include <string>
#include <cstdlib>

#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace std;

#define DEPTH (3)

const string alg_names[] =
{
    "random",
    "minimax",
    "alphabeta",
    "alphabeta_with_transposition_table",
    "monte_carlo",
    "monte_carlo_with_confidentiality_bound",
    "mcts"
    ,"alphabeta_with_tt_and_cbf"
};

unsigned int algorithms_size = 7;
Algorithm* getAlgorithm(int i, int seed)
{
    switch(i)
    {
    case 0: return new RandomGameAlg(seed);
    case 1: return new MiniMaxAlg(seed, new SimpleGrader(), DEPTH, 0); // nie mierzy sobie czasu - moze nie powinien byc uzywany?
    case 2: return new AlphaBetaAlg(seed, new SimpleGrader(), DEPTH, 0);
    case 3: return new AlphaBetaAlg(seed, new TranspositionTable(), new ZobristHasher(42), new SimpleGrader(), DEPTH, 0);
    case 4: return new MonteCarloMethod(seed);
    case 5: return new MonteCarloMethod(seed, true);
    case 6: return new MCTS(seed);
    case 7: return new AlphaBetaAlg(seed, new TranspositionTable(), new ZobristHasher(42), new SimpleGrader(), DEPTH, 0, true);
    default: return NULL;
    }
}


void test(Algorithm* alg);
int main(int argc, char** argv)
{
    mt19937 generator;
    generator.seed(5674);
    uniform_int_distribution<> dis(0, 1000000);
    int ab_indices[4] = {1,2,3,7};

    int i;
    for (int j = 2; j < 4; ++j)
    {
        i = ab_indices[j];
        boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
        Algorithm* alg = getAlgorithm(i, dis(generator));
        test(alg);
        unsigned int passed = (boost::posix_time::microsec_clock::local_time() - start_time).total_milliseconds();
        printf("%s:\tfull search at depth %d took:\t%u\n", alg_names[i].c_str(), DEPTH+1, passed);
        printf("alg[%u]: %s\n\n", i, alg->stats().c_str());
        delete(alg);
        //depth +1, bo depth 0 oznacza zaglebienie sie na 1
    }

    return 0;
}

void test(Algorithm* alg)
{
    SplitsGame game;
    void* moves;
    unsigned int size;
    mt19937 generator;
    uniform_int_distribution<> dis(0, 100);
    generator.seed(5);
    Move* move;
    for (int i = 0; i < 10; ++i)
    {
        moves = game.getPossibleMoves(&size);
        move = SplitsGame::rawPossibleMoveOfIndex(moves, dis(generator)%size , game.gamePhase());
        move = game.copyMove(move);
        game.makeMove(move);
        alg->makeMove(move);
        delete(move); move = NULL;
    }

    alg->decideMove(&move);
}

