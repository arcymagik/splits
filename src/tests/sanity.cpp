#include "splits.h"
#include "random_game_algorithm.h"
#include "minimax.h"
#include "simple_grader.h"

#include <cstdio>

#include <vector>
#include <random>
#include <iostream>

#include <string>

using namespace std;

#define SEED 19

int basic_sanity();
int random_sanity();
unsigned int choose_move_basic(void* moves, unsigned int size, GamePhase phase);
unsigned int choose_move_random(void* moves, unsigned int size, GamePhase phase);

int random_alg_test();

mt19937 generator;

int run_test(int (*test)(), string name);

int main(int argc, char** argv)
{
    argc = argc; argv = argv;
    generator.seed(SEED);

    run_test(basic_sanity, "basic_sanity");
    run_test(random_sanity, "random_sanity");

    return 0;
}

int run_test(int (*test)(), string name)
{
    printf("%s\n", name.c_str());
    if (test()) printf("%s test failed\n", name.c_str()); else printf("%s test passed\n", name.c_str());
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

    while(!game.isFinished())
    {
        moves = game.getPossibleMoves(&size);
        index = function(moves, size, game.gamePhase());
        move = SplitsGame::possibleMoveOfIndex(moves, index, game.gamePhase());
        if (game.canMove(move))
        {
            game.makeMove(index);
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
