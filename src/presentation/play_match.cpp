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

#define NUMBER_OF_PLAYS (5)
#define TIME_TO_MOVE (1000)

using namespace std;

int play(Algorithm* alg0, Algorithm* alg1, unsigned int timeForMove);

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

unsigned int algorithms_size = 8;
Algorithm* getAlgorithm(int i, int seed)
{
    switch(i)
    {
    case 0: return new RandomGameAlg(seed);
    case 1: return new MiniMaxAlg(seed, new SimpleGrader(), 2, 0); // nie mierzy sobie czasu - moze nie powinien byc uzywany?
    case 2: return new AlphaBetaAlg(seed, new SimpleGrader(), 2, 0);
    case 3: return new AlphaBetaAlg(seed, new TranspositionTable(), new ZobristHasher(42), new SimpleGrader(), 2, 0);
    case 4: return new MonteCarloMethod(seed);
    case 5: return new MonteCarloMethod(seed, true);
    case 6: return new MCTS(seed);
    case 7: return new AlphaBetaAlg(seed, new TranspositionTable(), new ZobristHasher(42), new SimpleGrader(), 2, 0, true);
    default: return NULL;
    }
}

int main(int argc, char** argv)
{
    int zth, st;
    zth = atoi(argv[1]);
    st = atoi(argv[2]);

    mt19937 generator;
    generator.seed(43423);
    unsigned int result = 0;
    uniform_int_distribution<> dis(0, 1000000);
    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    for (unsigned int i = 0; i < NUMBER_OF_PLAYS; ++i)
    {
        Algorithm* alg0 = getAlgorithm(zth, dis(generator));
        Algorithm* alg1 = getAlgorithm(st, dis(generator));
        result += play(alg0, alg1, TIME_TO_MOVE);
        delete(alg1);
        delete(alg0);
    }

    unsigned int passed = (boost::posix_time::microsec_clock::local_time() - start_time).total_milliseconds();
    printf("%s\tvs\t%s:\t%u/%u\ttook %u ms\n",
           alg_names[zth].c_str(), alg_names[st].c_str(), NUMBER_OF_PLAYS-result, NUMBER_OF_PLAYS, passed);

    return 0;
}

int play(Algorithm* alg0, Algorithm* alg1, unsigned int timeForMove)
{
    SplitsGame game;
    Algorithm* algs[2] = {alg0, alg1};
    Move* move;
    boost::posix_time::ptime start_time;
    int time_passed;
    unsigned int cp;
    while (!game.isFinished())
    {
        cp = game.curPlayer();
        start_time = boost::posix_time::microsec_clock::local_time();
        algs[cp]->decideMove(&move, TIME_TO_MOVE);
        move = game.copyMove(move);
        auto current_time = boost::posix_time::microsec_clock::local_time() - start_time;
        time_passed = current_time.total_milliseconds();
        if (time_passed > TIME_TO_MOVE)
        {
            printf("alg %u przekroczyl czas (passed: %d)!\n", cp, time_passed);
            return cp ^ 1;
        }
        game.makeMove(move);
        for (unsigned int i = 0; i < 2; ++i)
            algs[i]->makeMove(move);
        delete(move); move = NULL;
    }
    return game.getWinner();
}
