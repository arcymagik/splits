#include "zobrist_hasher.h"

#include <random>

#define EMPTY_FIELD_NUMBER (TOKEN_QUANTITY+1)
#define quick_mod(num, m) ((num) >= (m) ? ((num)-(m)) : num)
#define shift_unit_field(f, dir) ((f) + SHIFTS[(dir)])

using namespace std;

const int SHIFTS[6] = {
    -MAX_BOARD_SIZE+1,
    1,
    MAX_BOARD_SIZE,
    MAX_BOARD_SIZE-1,
    -1,
    -MAX_BOARD_SIZE
};

const int BUILDING_TILE_SHIFTS[3] = {0, 1, 5};

ZobristHasher::ZobristHasher(int seed)
{
    std::mt19937 generator;
    generator.seed(seed);
    hash = 0;

    uniform_int_distribution<unsigned long long> dis;
    for (unsigned int i = 0; i < WHOLE_MAX_BOARD_SIZE; ++i)
    {
        for (unsigned int j = 0; j < TOKEN_QUANTITY+2; ++j)
            magic[i][j] = dis(generator);
        hash += magic[i][EMPTY_FIELD_NUMBER];
    }

    for (unsigned int i = 31; i <= 32; ++i)
        for (unsigned int j = 31; j <= 32; ++j)
            hash += magic[i*MAX_BOARD_SIZE+j][0] - magic[i*MAX_BOARD_SIZE+j][EMPTY_FIELD_NUMBER];
}

ZobristHasher::~ZobristHasher()
{
}

void ZobristHasher::undoMove(Move* move, SplitsGame* game, GamePhase phase) // rozni sie od makeMove tylko odejmowaniem od hasha zamiast dodawaniem
{
    switch (phase)
    {
    case Building:
    {
        int pos = ((BuildingMove*) move)->pos;
        int dir = ((BuildingMove*) move)->dir;
        int tmp_pos;
        hash -= magic[pos][0] - magic[pos][EMPTY_FIELD_NUMBER];
        for (unsigned int i = 0; i < 3; ++i)
        {
            tmp_pos = shift_unit_field(pos, quick_mod(dir + BUILDING_TILE_SHIFTS[i], 6));
            hash -= magic[tmp_pos][0] - magic[pos][EMPTY_FIELD_NUMBER];
        }
        break;
    }
    case Initial:
    {
        int pos = ((InitialMove*) move)->pos;
        hash -= magic[pos][TOKEN_QUANTITY] - magic[pos][0];
        break;
    }
    case Normal:
    {
        int source = ((NormalMove*) move)->source;
        int target = ((NormalMove*) move)->target;
        int quantity = ((NormalMove*) move)->quantity;
        unsigned int stack = game->getField(source)->stack;

        hash -=
            magic[source][stack-quantity] - magic[source][stack]
            + magic[target][quantity] - magic[target][0];
        break;
    }
    };
}

void ZobristHasher::makeMove(Move* move, SplitsGame* game, GamePhase phase)
{
    switch (phase)
    {
    case Building:
    {
        int pos = ((BuildingMove*) move)->pos;
        int dir = ((BuildingMove*) move)->dir;
        int tmp_pos;
        hash += magic[pos][0] - magic[pos][EMPTY_FIELD_NUMBER];
        for (unsigned int i = 0; i < 3; ++i)
        {
            tmp_pos = shift_unit_field(pos, quick_mod(dir + BUILDING_TILE_SHIFTS[i], 6));
            hash += magic[tmp_pos][0] - magic[pos][EMPTY_FIELD_NUMBER];
        }
        break;
    }
    case Initial:
    {
        int pos = ((InitialMove*) move)->pos;
        hash += magic[pos][TOKEN_QUANTITY] - magic[pos][0];
        break;
    }
    case Normal:
    {
        int source = ((NormalMove*) move)->source;
        int target = ((NormalMove*) move)->target;
        int quantity = ((NormalMove*) move)->quantity;
        unsigned int stack = game->getField(source)->stack;

        hash +=
            magic[source][stack-quantity] - magic[source][stack]
            + magic[target][quantity] - magic[target][0];
        break;
    }
    };
}
