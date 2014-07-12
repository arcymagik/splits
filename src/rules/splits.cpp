#include "splits.h"

#define HALF_BOARD_SIZE 32
#define MAX_BOARD_SIZE (HALF_BOARD_SIZE*2)
#define WHOLE_MAX_BOARD_SIZE (MAX_BOARD_SIZE*MAX_BOARD_SIZE)
#define TOKEN_QUANTITY 20

#define single_coord(x, y) ((y)*MAX_BOARD_SIZE + (x))

#define quick_mod(num, m) ((num) >= (m) ? ((num)-(m)) : num)

using namespace std;

const int SHIFTS[6] = {
    -MAX_BOARD_SIZE+1,
    1,
    MAX_BOARD_SIZE,
    MAX_BOARD_SIZE-1,
    -1,
    -MAX_BOARD_SIZE
};

SplitsGame::SplitsGame()
{
    board = (Field*) malloc(WHOLE_MAX_BOARD_SIZE*sizeof(Field));

    for(int i = 0; i  < WHOLE_MAX_BOARD_SIZE; ++i)
        board[i].stack = -1;


    for(int i = 0; i < 2; ++i)
        for(int j = 0; j < 2; ++j)
            board[single_coord(HALF_BOARD_SIZE-i, HALF_BOARD_SIZE-j)].stack = 0;
}

SplitsGame::~SplitsGame()
{
    free(board);
}

void SplitsGame::makeMove(Move* move)
{
    move->makeHere(this);
    history.push_back((Move*) move);
}

int SplitsGame::undoMove()
{
    if(history.size() == 0) return -1;

    HistoryMove* unmove = history.back();
    history.pop_back();
    unmove->undo(this);
    return 0;
}

void SplitsGame::makeNormal(NormalMove* move)
{
    board[move->source].stack -= move->quantity;
    board[move->target].stack += move->quantity;

    //history.push_back((Move*) move); // TODO: sprawdzic czemu bez rzutowania wyskakuje blad
}

void SplitsGame::undoNormal(NormalMove* move)
{
    board[move->source].stack += move->quantity;
    board[move->target].stack -= move->quantity;
}

void SplitsGame::makeBuilding(BuildingMove* move)
{
    board[move->pos].stack = 0;
    board[move->pos + SHIFTS[ move->dir ]].stack = 0;
    board[move->pos + SHIFTS[ quick_mod(move->dir + 1, 6)]].stack = 0;
    board[move->pos + SHIFTS[ quick_mod(move->dir + 3, 6)]].stack = 0;
}

void SplitsGame::undoBuilding(BuildingMove* move)
{
    board[move->pos].stack = -1;
    board[move->pos + SHIFTS[ move->dir ]].stack = -1;
    board[move->pos + SHIFTS[ quick_mod(move->dir + 1, 6)]].stack = -1;
    board[move->pos + SHIFTS[ quick_mod(move->dir + 3, 6)]].stack = -1;
}

void SplitsGame::makeInitial(InitialMove* move)
{
    board[move->pos].stack = TOKEN_QUANTITY;
}

void SplitsGame::undoInitial(InitialMove* move)
{
    board[move->pos].stack = 0;
}

int SplitsGame::isFinished()
{
    if( getPossibleMoves().size() != 0) return 2;

    return ((unsigned int) history.size() & 1);
}

vector<Move*> SplitsGame::getPossibleMoves()
{
    //TODO;
    return vector<Move*>();
}

void NormalMove::makeHere(SplitsGame* game)
{
    game->makeNormal(this);
}

void BuildingMove::makeHere(SplitsGame* game)
{
    game->makeBuilding(this);
}

void InitialMove::makeHere(SplitsGame* game)
{
    game->makeInitial(this);
}

void NormalMove::undo(SplitsGame* game)
{
    game->undoNormal(this);
}

void BuildingMove::undo(SplitsGame* game)
{
    game->undoBuilding(this);
}

void InitialMove::undo(SplitsGame* game)
{
    game->undoInitial(this);
}
