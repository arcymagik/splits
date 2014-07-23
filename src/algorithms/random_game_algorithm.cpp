#include "random_game_algorithm.h"

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
    int size = moves.size();
    uniform_int_distribution<> dis(0, size-1);
    int index = dis(generator);
    Move* move = moves[index]->copy();
    for (unsigned int i = 0; i < moves.size(); ++i) delete(moves[i]); // TODO - te ciagle alokacje/dealokacje musza sie skonczyc
    return move;
}
