#include "splits.h"
#include "random_game_algorithm.h"
#include "minimax.h"
#include "simple_grader.h"

#include <cstdio>

#include <vector>
#include <random>
#include <iostream>

#include <string>

#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace std;

#define SEED 19

int basic_sanity();
int random_sanity();
Move* choose_move_basic(vector<Move*>* moves);
Move* choose_move_random(vector<Move*>* moves);

int random_alg_test();
int minimax_alg_test();

mt19937 generator;

int run_test(int (*test)(), string name);

int main(int argc, char** argv)
{
    argc = argc; argv = argv;
    generator.seed(SEED);

    run_test(basic_sanity, "basic_sanity");
    run_test(random_sanity, "random_sanity");
    run_test(random_alg_test, "random_alg_test");
    run_test(minimax_alg_test, "minimax_alg_test");

    return 0;
}

int run_test(int (*test)(), string name)
{
    printf("%s\n", name.c_str());
    if (test()) printf("%s test failed\n", name.c_str()); else printf("%s test passed\n", name.c_str());
    return 0;
}

int sanity_template(Move* (*function)(vector<Move*>* moves))
{
    SplitsGame game;
    vector<Move*> moves;
    Move* move;
    //int mindex;
    unsigned int size;
    SimpleGrader grader;

    while(!game.isFinished())
    {
        // {
        //     //printf("%s\n", game.getPrettyHistory().c_str());

        //     printf("plansza: %s\n", game.getDesc().c_str());
        //     fflush(stdout);
        // }

        moves = game.getPossibleMoves();
        size = moves.size();
        // printf("chose %d of %u\n", mindex, size);
        //move = moves[mindex];
        move = function(&moves);
        if (game.canMove(move))
        {
            game.makeMove(move);
            printf("move: %s\n", move->prettyDesc().c_str());
            printf("ocena ruchu: %d\n", grader.grade(&game));
        }
        else
        {
            printf("Nastapil bardzo powazny problem. Ten ruch nie powinien sie tu znalezc!!!\n");
            printf("%s\n", game.getPrettyHistory().c_str());
            printf("plansza: %s\n", game.getDesc().c_str());
            return -1;
        }
        delete move;
        for (unsigned int i = 0; i < size; ++i)
            delete(moves[i]);
    }

    printf("Wygral gracz %d\n", ((unsigned int) game.curPlayer()) ^ 1);


    printf("%s\n", game.getPrettyHistory().c_str());

    printf("plansza: %s\n", game.getDesc().c_str());

    while(game.undoMove() == 0);
    
    return 0;
}

int basic_sanity()
{
    return sanity_template(choose_move_basic);
}

int random_sanity()
{
    return sanity_template(choose_move_random);
}

Move* choose_move_basic(vector<Move*>* moves)
{
    return (*moves)[0]->copy();
}

Move* choose_move_random(vector<Move*>* moves)
{
    uniform_int_distribution<> dis(0, moves->size()-1);
    int index = dis(generator);
    return (*moves)[index]->copy();
}

Move* random_alg_test_choose(vector<Move*>* moves)
{
    static RandomGameAlg alg(54);

    Move* move = alg.decideMove();
    alg.makeMove(move);
    return move;
}

Move* minimax_alg_test_choose(vector<Move*>* moves)
{
    static MiniMaxAlg alg;
    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    int time_passed;

    Move* move = alg.decideMove();

    auto current_time = boost::posix_time::microsec_clock::local_time() - start_time;
    time_passed = current_time.total_milliseconds();
    printf("staty algorytmu: %s\n", alg.stats().c_str());
    printf("krok algorytmu zajal: %d\n", time_passed);

    alg.makeMove(move);
    return move;
}

int random_alg_test()
{
    return sanity_template(random_alg_test_choose);
}


int minimax_alg_test()
{
    return sanity_template(minimax_alg_test_choose);
}
