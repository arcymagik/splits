#include "splits.h"
#include "random_game_algorithm.h"
#include "minimax.h"
#include "simple_grader.h"
#include "adv_grader.h"
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
#define TIME_TO_MOVE (2000) // to chyba lepiej bylyby command line parametry

using namespace std;

int play(Algorithm* alg0, Algorithm* alg1, unsigned int timeForMove);

const string alg_names[] =
{
    "mcts_20"
    ,"mcts_30"
    ,"mcts_50"
    ,"mcts_80"
    ,"mcts_100"
    ,"mcts_150"
    ,"mcts_200"
    ,"mcts_4"
    ,"mcts_4000"
};

unsigned int algorithms_size = 9;
Algorithm* getAlgorithm(unsigned int i, int seed)
{
    switch(i)
    {
        //case 0: return new MCTS(seed, 20);
        //case 1: return new MCTS(seed, 30);
        //case 2: return new MCTS(seed, 50);
        //case 3: return new MCTS(seed, 80);
    case 4: return new MCTS(seed, 100);
        //case 5: return new MCTS(seed, 150);
        //case 6: return new MCTS(seed, 200);
    case 7: return new MCTS(seed, 5);
    case 8: return new MCTS(seed, 4000);
    default: return NULL;
    }
}

bool both_active(unsigned int i, unsigned int j)
{
    Algorithm* alg1 = getAlgorithm(i, 1);
    Algorithm* alg2 = getAlgorithm(j, 1);
    bool result = (alg1 != NULL) && (alg2 != NULL);
    delete alg1;
    delete alg2;
    return result;
}

int main(int argc, char** argv)
{
    Algorithm* alg1;
    Algorithm* alg2;
    mt19937 generator;
    generator.seed(43223);
    uniform_int_distribution<> dis(0, 1000000);
    unsigned int result;
    for (unsigned int i = 0; i < algorithms_size; ++i)
        for (unsigned int j = 0; j < algorithms_size; ++j)
            if (i != j && // MAYBE nawet bez tego, zeby zobaczyc jaki wplyw na wygrywanie ma pierwszenstwo
                both_active(i, j))
            {
                boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
                result = 0;
                for (unsigned int k = 0; k < NUMBER_OF_PLAYS; ++k)
                {
                    alg1 = getAlgorithm(i, dis(generator));
                    alg2 = getAlgorithm(j, dis(generator));
                    result += play(alg1, alg2, TIME_TO_MOVE); //ilosc zwyciestw gracza 1 (tzn. alg2)
                    delete(alg1);
                    delete(alg2);
                    printf(".");
                    fflush(stdout);
                }
                unsigned int passed = (boost::posix_time::microsec_clock::local_time() - start_time).total_milliseconds();
                printf("%s\tvs\t%s:\t%u/%u\ttook %u ms\n", alg_names[i].c_str(), alg_names[j].c_str(), NUMBER_OF_PLAYS-result, NUMBER_OF_PLAYS, passed);
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
