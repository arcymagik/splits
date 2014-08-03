#ifndef ZOBRIST_HASHER_HH
#define ZOBRIST_HASHER_HH

#include "splits.h"

#define HALF_BOARD_SIZE 32
#define MAX_BOARD_SIZE (HALF_BOARD_SIZE*2)
#define WHOLE_MAX_BOARD_SIZE (MAX_BOARD_SIZE*MAX_BOARD_SIZE)

class ZobristHasher : public Hasher
{
public:
    ZobristHasher(int seed);
    virtual ~ZobristHasher();
    virtual void makeMove(Move* move, SplitsGame* game, GamePhase phase);
    virtual void undoMove(Move* move, SplitsGame* game, GamePhase phase);
private:
    unsigned long long magic[WHOLE_MAX_BOARD_SIZE][TOKEN_QUANTITY+2];
};

#endif
