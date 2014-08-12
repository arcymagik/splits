#include "splits.h"

#include "random_game_algorithm.h"
#include "alpha_beta.h"
#include "minimax.h"
#include "monte_carlo.h"

#include "simple_grader.h"

#include <cstdio>

#include <vector>
#include <random>
#include <iostream>

#include <string>

#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace std;

int alpha_beta_sanity();
int ab_transTable_sanity();
int mc_sanity();
int alg_sanity_template(Algorithm* alg);

int run_test(int (*test)(), string name);

int main(int argc, char** argv)
{
    run_test(alpha_beta_sanity, "alpha_beta_sanity");
    run_test(ab_transTable_sanity, "ab_transTable_sanity");
    run_test(mc_sanity, "mc_sanity");
    return 0;
}

int run_test(int (*test)(), string name)
{
    printf("%s\n", name.c_str());
    if (test()) printf("%s test failed\n", name.c_str()); else printf("%s test passed\n", name.c_str());
    return 0;
}

int mc_sanity()
{
    MonteCarloMethod alg(32);
    return alg_sanity_template(&alg);
}

int alpha_beta_sanity()
{
    AlphaBetaAlg alg(new SimpleGrader(), 2, 0);
    return alg_sanity_template(&alg);
}

int ab_transTable_sanity()
{
    AlphaBetaAlg alg(new TranspositionTable(), new ZobristHasher(231), new SimpleGrader(), 2, 0);
    return alg_sanity_template(&alg);
}

int alg_sanity_template(Algorithm* alg)
{
    SplitsGame game;
    SimpleGrader grader;
    Move* move;
    boost::posix_time::ptime start_time;
    int time_passed;

    while (!game.isFinished())
    {
        //printf("staty algorytmu przed: %s\n", alg->stats().c_str());
        start_time = boost::posix_time::microsec_clock::local_time();
        alg->decideMove(&move);
        auto current_time = boost::posix_time::microsec_clock::local_time() - start_time;
        time_passed = current_time.total_milliseconds();

        if (game.canMove(move))
        {
            printf("move: %s\n", game.getPrettyDescMove(move).c_str());;
            printf("ocena ruchu: %d\n", grader.grade(&game));
            //printf("staty algorytmu: %s\n", alg->stats().c_str());
            //printf("opis gry: %s\n", game.getDesc().c_str());
            printf("krok algorytmu zajal: %d\n", time_passed);
            game.makeMove(move);
            alg->makeMove(move);
        }
        else
        {
            printf("Nastapil bardzo powazny problem. Ten ruch nie powinien sie tu znalezc!!!\n");
            printf("move: %s\n", game.getPrettyDescMove(move).c_str());
            printf("%s\n", game.getPrettyHistory().c_str());
            printf("plansza: %s\n", game.getDesc().c_str());
            printf("ostatnie staty algorytmu: %s\n", alg->stats().c_str());
            return -1;
        }
    }

    printf("Wygral gracz %d\n", ((unsigned int) game.curPlayer()) ^ 1);


    printf("%s\n", game.getPrettyHistory().c_str());

    printf("plansza: %s\n", game.getDesc().c_str());

    while(game.undoMove() == 0);

    return 0;
}

