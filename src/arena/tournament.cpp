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

#include <boost/date_time/posix_time/posix_time_types.hpp>

#define NUMBER_OF_PLAYS (10)
#define TIME_TO_MOVE (1000) // to chyba lepiej bylyby command line parametryv

using namespace std;

int play(Algorithm* alg0, Algorithm* alg1, unsigned int timeForMove);

const string alg_names[] =
{
    "random",
    "minimax",
    "alphabeta",
    "alphabeta_with_transposition_table",
    "monte_carlo",
    "monte_carlo_with_trust_limit",
    "mcts"
};

unsigned int algorithms_size = 7;
Algorithm* getAlgorithm(unsigned int i, int seed)
{
    switch(i)
    {
    case 0: return new RandomGameAlg(seed);
    case 1: return new MiniMaxAlg(new SimpleGrader(), 2, 0); // nie mierzy sobie czasu - moze nie powinien byc uzywany?
    case 2: return new AlphaBetaAlg(new SimpleGrader(), 2, 0);
    case 3: return new AlphaBetaAlg(new TranspositionTable(), new ZobristHasher(42), new SimpleGrader(), 2, 0);
    case 4: return new MonteCarloMethod(seed);
    case 5: return new MonteCarloMethod(seed, true);
    case 6: return new MCTS(seed);
    default: return NULL;
    }
}

int main(int argc, char** argv)
{
    Algorithm* alg1;
    Algorithm* alg2;
    mt19937 generator;
    generator.seed(432243);
    uniform_int_distribution<> dis(0, 1000000);
    unsigned int result;
    for (unsigned int i = 0; i < algorithms_size-1; ++i)
        for (unsigned int j = i+1; j < algorithms_size; ++j)
        {
            result = 0;
            for (unsigned int k = 0; k < NUMBER_OF_PLAYS; ++k)
            {
                alg1 = getAlgorithm(i, dis(generator));
                alg2 = getAlgorithm(j, dis(generator));
                result += play(alg1, alg2, TIME_TO_MOVE);
                delete(alg1);
                delete(alg2);
            }
            printf("%s\tvs\t%s:\t%u/%u\n", alg_names[i].c_str(), alg_names[j].c_str(), NUMBER_OF_PLAYS-result, NUMBER_OF_PLAYS);
        }
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
    }
    return game.getWinner();
}
