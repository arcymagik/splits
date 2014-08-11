#include "splits.h"
#include "random_game_algorithm.h"
#include "minimax.h"
#include "simple_grader.h"
#include "zobrist_hasher.h"

#include <cstdio>

#include <vector>
#include <random>
#include <iostream>

#include <string>

#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace std;

#define SEED 19

int random_alg_sanity();
int minimax_alg_sanity();
int alg_sanity_template(Algorithm* alg);

int undo_sanity();
int basic_sanity();
int random_sanity();
unsigned int choose_move_basic(void* moves, unsigned int size, GamePhase phase);
unsigned int choose_move_random(void* moves, unsigned int size, GamePhase phase);

mt19937 generator;

int run_test(int (*test)(), string name);

int main(int argc, char** argv)
{
    argc = argc; argv = argv;
    generator.seed(SEED);

    run_test(basic_sanity, "basic_sanity");
    run_test(random_sanity, "random_sanity");
    run_test(undo_sanity, "undo_sanity");
    run_test(random_alg_sanity, "random_alg_sanity");
    run_test(minimax_alg_sanity, "minimax_alg_sanity");

    return 0;
}

int run_test(int (*test)(), string name)
{
    printf("%s\n", name.c_str());
    if (test()) printf("%s test failed\n", name.c_str()); else printf("%s test passed\n", name.c_str());
    return 0;
}

int undo_sanity()
{
    SplitsGame game;

    for (int i = 0; i < 9; ++i)
    {
        game.makeIndexedMove(0);
    }
    printf("first: %s\n", game.getDesc().c_str());
    game.makeIndexedMove(0);
    game.undoMove();
    printf("second: %s\n", game.getDesc().c_str());

    return 0;
}

int sanity_template(unsigned int (*function)(void* moves, unsigned int size, GamePhase phase))
{
    SplitsGame game;
    void* moves;
    unsigned int index;
    Move* move;
    unsigned int size;
    SimpleGrader grader;
    ZobristHasher hasher(19);

    while(!game.isFinished())
    {
        moves = game.getPossibleMoves(&size);
        index = function(moves, size, game.gamePhase());
        move = SplitsGame::possibleMoveOfIndex(moves, index, game.gamePhase());
        if (game.canMove(move))
        {
            hasher.makeMove(move, &game, game.gamePhase());
            game.makeIndexedMove(index);
            printf("move: %s\n", move->prettyDesc().c_str());
            printf("ocena ruchu: %d\n", grader.grade(&game));
            printf("hash: %llu\n", hasher.getHash());
        }
        else
        {
            printf("Nastapil bardzo powazny problem. Ten ruch nie powinien sie tu znalezc!!!\n");
            printf("%s\n", game.getPrettyHistory().c_str());
            printf("plansza: %s\n", game.getDesc().c_str());
            return -1;
        }
        delete move;
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

unsigned int choose_move_basic(void* moves, unsigned int size, GamePhase phase)
{
    return 0;
}

unsigned int choose_move_random(void* moves, unsigned int size, GamePhase phase)
{
    uniform_int_distribution<> dis(0, size-1);
    int index = dis(generator);
    return index;
}

int random_alg_sanity()
{
    RandomGameAlg alg(19);
    return alg_sanity_template(&alg);
}

int minimax_alg_sanity()
{
    MiniMaxAlg alg(new SimpleGrader(), 2, 0);
    return alg_sanity_template(&alg);
}

int alg_sanity_template(Algorithm* alg)
{
    SplitsGame game;
    SimpleGrader grader;
    Move* move;
    boost::posix_time::ptime start_time;
    boost::posix_time::ptime end_time;
    int time_passed;

    while (!game.isFinished())
    {
        //printf("staty algorytmu przed: %s\n", alg->stats().c_str());
        start_time = boost::posix_time::microsec_clock::local_time();
        alg->decideMove(&move);
        end_time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration duration = end_time - start_time;
        time_passed = duration.total_milliseconds();
        
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
