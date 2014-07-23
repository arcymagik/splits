#include "splits.h"

#include <cstdio>

#include <vector>
#include <random>
#include <iostream>

using namespace std;

#define SEED 19

int basic_sanity();
int random_sanity();
int choose_move_basic(vector<Move*>* moves);
int choose_move_random(vector<Move*>* moves);

mt19937 generator;

int main(int argc, char** argv)
{
    argc = argc; argv = argv;
    generator.seed(SEED);

    printf("basic\n");
    if (basic_sanity()) printf("basic_sanity test failed\n"); else printf("basic_sanity test passed\n");
    printf("random\n");
    if (random_sanity()) printf("random_sanity test failed\n"); else printf("random_sanity test passed\n");

    return 0;
}

int sanity_template(int (*function)(vector<Move*>* moves))
{
    SplitsGame game;
    vector<Move*> moves;
    Move* move;
    int mindex;

    while(!game.isFinished())
    {
        {
            printf("%s\n", game.getPrettyHistory().c_str());

            printf("plansza: %s\n", game.getDesc().c_str());
            fflush(stdout);
        }

        moves = game.getPossibleMoves();
        mindex = function(&moves);
        move = moves[mindex];
        if (game.canMove(move))
            game.makeMove(move);
        else
        {
            printf("Nastapil bardzo powazny problem. Ten ruch nie powinien sie tu znalezc!!!\n");
            printf("%s\n", game.getPrettyHistory().c_str());
            printf("plansza: %s\n", game.getDesc().c_str());
            return -1;
        }
        for (unsigned int i = 0; i < moves.size(); ++i) delete(moves[i]);
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

int choose_move_basic(vector<Move*>* moves)
{
    return 0;
}

int choose_move_random(vector<Move*>* moves)
{
    std::uniform_int_distribution<> dis(0, moves->size()-1);
    return dis(generator);
}
