#include "splits.h"

#include <cstdio>

#include <vector>

using namespace std;

int basic_sanity();
//int random_sanity();
int choose_move_basic(vector<Move*>* moves);
//int choose_move_random(vector<Move*>* moves);

int main(int argc, char** argv)
{
    argc = argc; argv = argv;

    if (basic_sanity()) printf("basic_sanity test failed\n"); else printf("basic_sanity test passed\n");
    //if (random_sanity()) printf("random_sanity test failed\n"); else printf("random_sanity test passed\n");

    return 0;
}

int basic_sanity()
{
    SplitsGame game;
    vector<Move*> moves;
    Move* move;

    while(!game.isFinished())
    {
        moves = game.getPossibleMoves();
        move = moves[choose_move_basic(&moves)];
        if (game.canMove(move))
            game.makeMove(move);
        else
        {
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

int choose_move_basic(vector<Move*>* moves)
{
    return 0;
}

// int choose_move_random(vector<Move*> moves)
// {
//     std::uniform_int_distribution<> dis(0, moves.size());
    
// }
