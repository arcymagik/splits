#include "random_game_algorithm.h"

#include <boost/lexical_cast.hpp>

using namespace std;

RandomGameAlg::RandomGameAlg()
{
    generator.seed(19);
}

RandomGameAlg::RandomGameAlg(unsigned int seed)
{
    generator.seed(seed);
}

RandomGameAlg::~RandomGameAlg() {}

Move* RandomGameAlg::decideMove()
{
    vector<Move*> moves = game.getPossibleMoves();
    unsigned int size = moves.size();
    moves_size = size;
    uniform_int_distribution<> dis(0, size-1);
    int index = dis(generator);
    Move* move = moves[index]->copy();
    for (unsigned int i = 0; i < moves.size(); ++i) delete(moves[i]); // TODO - te ciagle alokacje/dealokacje musza sie skonczyc
    return move;
}

void RandomGameAlg::decideMove(Move** best_move)
{
    unsigned int size;
    void* moves = game.getPossibleMoves(&size);
    moves_size = size;
    uniform_int_distribution<> dis(0, size-1);
    int index = dis(generator);
    *best_move = SplitsGame::rawPossibleMoveOfIndex(moves, index, game.gamePhase());
}

string RandomGameAlg::stats()
{
    string result = "moves sizes was: ";
    result += boost::lexical_cast<string>(moves_size);
    return result;
}
