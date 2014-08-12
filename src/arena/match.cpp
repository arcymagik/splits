#include "splits.h"
#include "random_game_algorithm.h"
#include "minimax.h"
#include "simple_grader.h"
#include "zobrist_hasher.h"
#include "alpha_beta.h"

#include <cstdio>

#include <vector>
#include <random>
#include <iostream>

#include <string>

#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace std;

int play(Algorithm* alg0, Algorithm* alg1);
int play(Algorithm* alg0, Algorithm* alg1, unsigned int timeForMove);

int main(int argc, char** argv)
{
    // TODO: dodac wybieranie algorytmu jako opcje command lineowa
    Algorithm* alg0 = new RandomGameAlg(57);
    Algorithm* alg1 = new AlphaBetaAlg(new TranspositionTable(), new ZobristHasher(231), new SimpleGrader(), 2, 0);

    play(alg0, alg1, 1000);

    delete alg1;
    delete alg0;

    return 0;
}

inline unsigned int nextPlayer(unsigned int player)
{
    return (player+1) & 1;
}

int play(Algorithm* alg0, Algorithm* alg1) // na razie bez mierzenia czasu
{
    SplitsGame game;
    Algorithm* algs[2] = {alg0, alg1};

    unsigned int curPl = 0;
    Move* move;
    boost::posix_time::ptime start_time;
    int time_passed;

    while (!game.isFinished())
    {
        start_time = boost::posix_time::microsec_clock::local_time();
        algs[curPl]->decideMove(&move);
        auto current_time = boost::posix_time::microsec_clock::local_time() - start_time;
        time_passed = current_time.total_milliseconds();
        printf("Algorytm %u myslal przez %d ms\n", curPl, time_passed);
        printf("move: %s\n", game.getPrettyDescMove(move).c_str());

        game.makeMove(move);
        for (int i = 0; i < 2; ++i) algs[i]->makeMove(move);
        curPl = nextPlayer(curPl);
    }

    printf("Wygral gracz %d\n", ((unsigned int) game.curPlayer()) ^ 1);

    printf("%s\n", game.getPrettyHistory().c_str());

    printf("plansza: %s\n", game.getDesc().c_str());
    return 0;
}

int play(Algorithm* alg0, Algorithm* alg1, unsigned int timeForMove)
{
    SplitsGame game;
    Algorithm* algs[2] = {alg0, alg1};

    unsigned int curPl = 0;
    Move* move;
    boost::posix_time::ptime start_time;
    int time_passed;

    while (!game.isFinished())
    {
        start_time = boost::posix_time::microsec_clock::local_time();
        algs[curPl]->decideMove(&move, timeForMove);
        auto current_time = boost::posix_time::microsec_clock::local_time() - start_time;
        time_passed = current_time.total_milliseconds();
        printf("Algorytm %u myslal przez %d ms\n", curPl, time_passed);
        if ((unsigned int) time_passed > timeForMove)
        {
            printf("Czas na ruch (%u ms) zostal przekroczony.\n", timeForMove);
            printf("Wygral gracz %d\n", ((unsigned int) game.curPlayer()) ^ 1);
            break;
        }
        printf("move: %s\n", game.getPrettyDescMove(move).c_str());

        game.makeMove(move);
        for (int i = 0; i < 2; ++i) algs[i]->makeMove(move);
        curPl = nextPlayer(curPl);
    }

    printf("Wygral gracz %d\n", ((unsigned int) game.curPlayer()) ^ 1);

    printf("%s\n", game.getPrettyHistory().c_str());

    printf("plansza: %s\n", game.getDesc().c_str());
    return 0;    
}
