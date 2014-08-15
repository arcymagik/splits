#include "splits.h"
#include <queue>

#include <boost/lexical_cast.hpp>

#define HALF_BOARD_SIZE 32
#define MAX_BOARD_SIZE (HALF_BOARD_SIZE*2)
#define WHOLE_MAX_BOARD_SIZE (MAX_BOARD_SIZE*MAX_BOARD_SIZE)

#define single_coord(x, y) ((y)*MAX_BOARD_SIZE + (x))
#define double_coord(p, x, y) {y = ((p)/MAX_BOARD_SIZE); x = ((p)%MAX_BOARD_SIZE);}

#define quick_mod(num, m) ((num) >= (m) ? ((num)-(m)) : num)
#define abs(a) ((a) < 0 ? (-(a)) : a)
#define shift_field(f, dir, dist) ((f) + (dist)*SHIFTS[(dir)])
#define shift_unit_field(f, dir) ((f) + SHIFTS[(dir)])

// jest initial move dla kazdego z graczy, ten jest pierwszy
#define INITIAL_MOVE_INDEX 7
#define NO_BUILDING_MOVES (INITIAL_MOVE_INDEX)
#define NO_NONNORMAL_MOVES (NO_BUILDING_MOVES+2)

#define INCORRECT_DIR (-1)

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

SplitsGame::SplitsGame()
{
    int pos;
    board = (Field*) malloc(WHOLE_MAX_BOARD_SIZE*sizeof(Field));
    outerBorder = (bool*) malloc(WHOLE_MAX_BOARD_SIZE*sizeof(bool));
    structuresAfterBoardIsBuiltAreBuilt = false;

    for(int i = 0; i  < WHOLE_MAX_BOARD_SIZE; ++i)
    {
        board[i].stack = -1;
        board[i].stacksIndex = 1000000000;
    }


    for(int i = 0; i < 2; ++i)
        for(int j = 0; j < 2; ++j)
        {
            pos = single_coord(HALF_BOARD_SIZE-i, HALF_BOARD_SIZE-j);
            board[pos].stack = 0;
            fields.push_back(pos);
        }
    possibleMovesUpToDate = false;
    possibleMoves = malloc(sizeof(NormalMove) * 500); // NormalMove jest najwiekszy; 500 > 120 to przeczucie;
}

SplitsGame::~SplitsGame()
{
    free(possibleMoves);
    free(outerBorder);
    //printf("board: %s\n", getBoardDesc().c_str());
    free(board);
}

// void SplitsGame::makeMove(Move* move)
// {
//     possibleMovesUpToDate = false;
//     move->makeHere(this);
//     history.push_back(move->copy());
// }

void SplitsGame::makeMove(Move* move)
{
    switch (gamePhase())
    {
    case Building:
    {
        BuildingMove bmove = *((BuildingMove*) move);
        bmove.makeHere(this);
        history.push_back(bmove.copy());
        break;
    }
    case Initial:
    {
        InitialMove imove = *((InitialMove*) move);
        imove.makeHere(this);
        history.push_back(imove.copy());
        break;
    }
    case Normal:
    {
        NormalMove nmove = *((NormalMove*) move);
        nmove.makeHere(this);
        history.push_back(nmove.copy());
        break;
    }
    };
    possibleMovesUpToDate = false;
}

void SplitsGame::makeIndexedMove(unsigned int index)
{
    GamePhase phase = gamePhase();
    if (!possibleMovesUpToDate) updatePossibleMoves();
    Move* move = possibleMoveOfIndex(possibleMoves, index, phase);
    switch (phase)
    {
    case Building:
    {
        BuildingMove* bmove = (BuildingMove*) move;
        bmove->makeHere(this);
        history.push_back(bmove);
        break;
    }
    case Initial:
    {
        InitialMove* imove = (InitialMove*) move;
        imove->makeHere(this);
        history.push_back(imove);
        break;
    }
    case Normal:
    {
        NormalMove* nmove = (NormalMove*) move;
        nmove->makeHere(this);
        history.push_back(nmove);
        break;
    }
    };
    possibleMovesUpToDate = false;
}

bool SplitsGame::canMove(Move* move)
{
    //return move->canMove(this);
    switch (gamePhase())
    {
    case Building: return canMoveBuilding((BuildingMove*) move);
    case Initial: return canMoveInitial((InitialMove*) move);
    case Normal: return canMoveNormal((NormalMove*) move);
    };
    return false;
}

int SplitsGame::undoMove()
{
    if(history.size() == 0) return -1;

    Move* unmove = history.back();
    history.pop_back();
    unmove->undo(this);
    delete(unmove);
    possibleMovesUpToDate = false;
    return 0;
}

//------------------------- static methods
int SplitsGame::calcDir(int source, int target, int* dist)
{
    int diff = target - source;
    int result;
    int sx, sy, tx, ty;
    //int mod, div;

    if (diff == 0) return INCORRECT_DIR;
    double_coord(source, sx, sy);
    double_coord(target, tx, ty);

    if (sy == ty)
    {
        if (diff > 0) result = 1;
        else result = 4;
        if (dist != NULL) *dist = diff;
        return result;
    }
    else if (sx == tx)
    {
        if (diff > 0) result = 2;
        else result = 5;
        if (dist != NULL) *dist = abs(diff) / MAX_BOARD_SIZE;
        return result;
    }
    else if (sx+sy == tx + ty)
    {
        if (diff > 0) result = 3;
        else result = 0;
        if (dist != NULL) *dist = abs(diff) / (MAX_BOARD_SIZE-1);
        return result;
    }
    

    // for (int i = 1; i < 4; ++i)
    // {
    //     mod = abs(diff) % SHIFTS[i];
    //     div = abs(diff) / SHIFTS[i];
    //     if (mod == 0 && div < MAX_BOARD_SIZE)
    //     {
    //         if (diff > 0) result =  i;
    //         else result =  quick_mod(i+3, 6);
            
    //         if (dist != NULL) *dist = div;

    //         return result;
    //     }
    // }
    return INCORRECT_DIR;
}

bool SplitsGame::fieldOutOfBoard(int pos, int dir)
{
    if (dir == 0 || dir == 5) return shift_unit_field(pos, dir) < 0;
    
    if (dir == 2 || dir == 3) return shift_unit_field(pos, dir) >= WHOLE_MAX_BOARD_SIZE;

    if (dir == 1) return pos % MAX_BOARD_SIZE == 0;

    //dir == 4
    return (pos-1) % MAX_BOARD_SIZE == 0;
}

bool SplitsGame::dangerousMove(Move* move)
{
    if (history.size() > 9)
    {
        NormalMove nmove = *(NormalMove*) move;
        NormalMove another = *(NormalMove*) history[history.size()-1];
        if (nmove.source == another.source || nmove.source == another.target) return true;
    }
    return false;
}

Move* SplitsGame::rawPossibleMoveOfIndex(void* voidMoves, unsigned int index, GamePhase phase)
{
    switch (phase)
    {
    case Building:
    {
        BuildingMove* moves = (BuildingMove*) voidMoves;
        return moves+index;
    }
    case Initial:
    {
        InitialMove* moves = (InitialMove*) voidMoves;
        return moves+index;
    }
    case Normal:
    {
        NormalMove* moves = (NormalMove*) voidMoves;
        return moves+index;
    }
    };
    return NULL;
}

Move* SplitsGame::possibleMoveOfIndex(void* voidMoves, unsigned int index, GamePhase phase)
{
    switch (phase)
    {
    case Building:
    {
        BuildingMove* moves = (BuildingMove*) voidMoves;
        return BuildingMove::copy(moves+index);
    }
    case Initial:
    {
        InitialMove* moves = (InitialMove*) voidMoves;
        return InitialMove::copy(moves+index);
    }
    case Normal:
    {
        NormalMove* moves = (NormalMove*) voidMoves;
        return NormalMove::copy(moves+index);
    }
    };
    return NULL;
}

//-----------------------------------------------------

bool SplitsGame::pathIsEmpty(int source, int dir, int dist)
{
    int field = source;
    for (int i = 0; i < dist; ++i)
    {
        field = shift_unit_field(field, dir);
        if (board[field].stack != 0) return false;
    }
    return true;
}

bool SplitsGame::fieldBehindUnavailable(int field, int dir)
{
    return board[shift_unit_field(field, dir)].stack != 0;
}

bool SplitsGame::sourceInStacks(int source)
{
    int cp = curPlayer();
    for (unsigned int i = 0; i < stacks[cp].size(); ++i)
        if (stacks[cp][i] == source) return true;
    return false;
}

// byc moze to wywalic i sprawdzac razem z polami na dir1 i dir2?
bool SplitsGame::touchesInOneOf4(int pos, int dir1, int dir2)
{
    for (int i = 0; i < 6; ++i) {
        if (i != dir1 && i != dir2)
        {
            if (board[shift_unit_field(pos, i)].stack > 0)
                return true;
        }
    }
    return false;
}

bool SplitsGame::stackBlocked(int pos)
{
    bool result = false;
    int next;
    for (unsigned int i = 0; i < 6; ++i)
    {
        next = shift_unit_field(pos, i);
        if (board[next].stack == 0) return true;
    }
    return result;
}

bool SplitsGame::tileTouchesOlderFields(int pos, int dir)
{
    return
        board[ shift_unit_field(pos, quick_mod(dir+3, 6)) ].stack >= 0
        || board[ shift_field(pos, dir, 2) ].stack >= 0
        || touchesInOneOf4( shift_unit_field(pos, quick_mod(dir + BUILDING_TILE_SHIFTS[1], 6)),
                            quick_mod(quick_mod(dir+BUILDING_TILE_SHIFTS[1], 6) + 3, 6),
                            quick_mod(dir + BUILDING_TILE_SHIFTS[2], 6)) //ten parametr to drobny hack, bo nie korzystam bezposrednio ze swojego pola tablicy BUILDING_TILE_SHIFTS
        || touchesInOneOf4( shift_unit_field(pos, quick_mod(dir + BUILDING_TILE_SHIFTS[2], 6)),
                            quick_mod(quick_mod(dir+BUILDING_TILE_SHIFTS[2], 6) + 3, 6),
                            quick_mod(dir + BUILDING_TILE_SHIFTS[1], 6));
}

void SplitsGame::buildStructuresAfterBoardIsBuilt()
{
    for (int i = 0; i < WHOLE_MAX_BOARD_SIZE; ++i) outerBorder[i] = false;
    for (int i = 0; i < MAX_BOARD_SIZE; ++i)
    {
	outerBorder[single_coord(i, 0)] = true;
	outerBorder[single_coord(i, (MAX_BOARD_SIZE-1))] = true;
	outerBorder[single_coord(0, i)] = true;
	outerBorder[single_coord((MAX_BOARD_SIZE-1), i)] = true;
    }

    queue<int> q;
    outerBorder[single_coord(1,1)] = true; // to pole na pewno jest poza plansza
    q.push(single_coord(1,1));

    int cur, another;
    while (!q.empty())
    {
        cur = q.front();
        q.pop();
        for (int i = 0; i < 6; ++i)
        {
            if (!fieldOutOfBoard(cur, i))
            {
                another = shift_unit_field(cur, i);
                if (!outerBorder[another] && board[another].stack == -1)
                {
                    outerBorder[another] = true;
                    q.push(another);
                }
            }
        }
    }
    structuresAfterBoardIsBuiltAreBuilt = true;
}

bool SplitsGame::fieldTouchesOuterBorder(int pos)
{
    if (!structuresAfterBoardIsBuiltAreBuilt) buildStructuresAfterBoardIsBuilt();
    for (int i = 0; i < 6; ++i)
        if (outerBorder[shift_unit_field(pos, i)])
            return true;
    return false;
}

bool SplitsGame::stacksWrong()
{
    return board[2205].stack != -1;
}

void SplitsGame::makeNormal(NormalMove* move)
{
    int source = move->source;
    int quantity = move->quantity;
    int target = move->target;
    unsigned int cp = curPlayer();
    //string debug = getDesc() + "\n";// + getBoardDesc()
    board[source].stack -= quantity;
    // if(board[target].stack != 0)
    //     printf("ccccccccccccc\n");
    board[target].stack = quantity; // zamiast +=, bo przeciez tu musialo byc 0

    if (board[source].stack == 1) // update stacks[cp] vector
    {
        int index = board[source].stacksIndex;
        if (quantity > 1)
        {
            board[target].stacksIndex = index;
            stacks[cp][index] = target;
        }
        else
        {
            stacks[cp][index] = stacks[cp][stacks[cp].size()-1];
            board[stacks[cp][index]].stacksIndex = index;
            stacks[cp].pop_back();
        }
    }
    else if (quantity > 1)
    {
        board[target].stacksIndex = stacks[cp].size();
        stacks[cp].push_back(target);
    }
    // if (checkStacks())
    // {
    //     printf ("%s", debug.c_str());
    // }
}

bool SplitsGame::checkStacks()
{
    for (unsigned int i = 0; i < stacks[0].size(); ++i)
        for (unsigned int j = 0; j < stacks[1].size(); ++j)
            if (stacks[0][i] == stacks[1][j])
            {
                // printf("plansza: %s\n", getDesc().c_str());
                // printf("board: %s\n", getBoardDesc().c_str());
                // printf("GGGGGGGGGGGGGGGGGGG");
                return true;
            }

    for (unsigned int pl = 0; pl < 2; ++pl)
        for (unsigned int i = 0; i < stacks[pl].size(); ++i)
            if (board[stacks[pl][i]].stacksIndex != i)
            {
                // printf("plansza: %s\n", getDesc().c_str());
                // printf("board: %s\n", getBoardDesc().c_str());
                // printf("SSSSSSSSSSSSSSSSSSS\n");
                return true;
            }
    if (board[2205].stack == 0)
        return false;
    return false;
}

unsigned int SplitsGame::getIndexOfMove(Move* move)
{
    if (!possibleMovesUpToDate) updatePossibleMoves();
    Move* another;
    unsigned int size;
    for (unsigned int i = 0; i < possibleMovesSize; ++i)
    {
        another = rawPossibleMoveOfIndex(getPossibleMoves(&size), i, gamePhase());
        if (movesEqual(move, another)) return i;
    }
    printf("this should not happen: move not found!\n");
    printf("%s\n", getDesc().c_str());
    printf("%s\n", getBoardDesc().c_str());
    printf("%d\n", (int) canMove(move));
    return 1000000000;
}

bool SplitsGame::movesEqual(Move* st, Move* nd)
{
    switch (gamePhase())
    {
    case Normal:
    {
        return
            ((NormalMove*) st)->source == ((NormalMove*) nd)->source
            && ((NormalMove*) st)->quantity == ((NormalMove*) nd)->quantity
            && ((NormalMove*) st)->target == ((NormalMove*) nd)->target;
    }
    case Building:
        return
            ((BuildingMove*) st)->pos == ((BuildingMove*) nd)->pos
            && ((BuildingMove*) st)->dir == ((BuildingMove*) nd)->dir;
    case Initial:
        return
            ((InitialMove*) st)->pos == ((InitialMove*) nd)->pos;
    };
    return false;
}

bool SplitsGame::canMoveNormal(NormalMove* move)
{
    int source = move->source;
    int target = move->target;
    int quantity = move->quantity;
    int dist;
    int dir = calcDir(source, target, &dist);

    return
        history.size() >= (NO_NONNORMAL_MOVES)
        && dir != INCORRECT_DIR
        && board[source].stack > quantity
        && pathIsEmpty(source, dir, dist)
        && fieldBehindUnavailable(target, dir)
        && sourceInStacks(source);
}

void SplitsGame::undoNormal(NormalMove* move)
{
    int source = move->source;
    int quantity = move->quantity;
    int target = move->target;
    unsigned int cp = curPlayer();
    //string debug = getDesc() + "\n";// + getBoardDesc()
    if (board[source].stack == 1) // update stacks[cp] vector
    {
        unsigned int index = board[source].stacksIndex;
        if (quantity > 1)
        {
            stacks[cp][index] = source;
        }
        else
        {
            stacks[cp].push_back(-1);
            if (index != stacks[cp].size()-1)
            {
                stacks[cp][stacks[cp].size()-1] = stacks[cp][index];
                board[stacks[cp][index]].stacksIndex = stacks[cp].size()-1;
            }
            stacks[cp][index] = source;
        }
    }
    else if (quantity > 1)
    {
        stacks[cp].pop_back();
    }

    board[source].stack += quantity;
    board[target].stack = 0; // bo przeciez musialo byc 0

    // if (checkStacks())
    // {
    //     printf("%s", debug.c_str());
    // }
}

// building ma zawsze kierunki dir + (0, 1, 5), co byc moze moznaby zapisac
// w jakiejs const tablicy, ale to chyba obnizy wydajnosc
void SplitsGame::makeBuilding(BuildingMove* move)
{
    int pos = move->pos;
    int dir = move->dir;
    int tmp_pos = pos;
    board[pos].stack = 0;
    fields.push_back(tmp_pos);
    for (int i = 0; i < 3; ++i)
    {
        tmp_pos = shift_unit_field(pos, quick_mod(dir + BUILDING_TILE_SHIFTS[i], 6));
        board[tmp_pos].stack = 0;
        fields.push_back(tmp_pos);
    }

}

bool SplitsGame::canMoveBuilding(BuildingMove* move)
{
    int pos = move->pos;
    int dir = move->dir;
    return
        history.size() < NO_BUILDING_MOVES
        && board[pos].stack == -1 //budowane pola musza byc puste
        && board[shift_unit_field(pos, dir)].stack == -1
        && board[shift_unit_field(pos, quick_mod(move->dir + 1, 6))].stack == -1
        && board[shift_unit_field(pos, quick_mod(move->dir + 5, 6))].stack == -1
        && tileTouchesOlderFields(pos, dir);
}

void SplitsGame::undoBuilding(BuildingMove* move)
{
    int pos = move->pos;
    int dir = move->dir;

    for (int i = 0; i < 4; ++i) fields.pop_back();

    board[move->pos].stack = -1;
    for (int i = 0; i < 3; ++i)
        board[shift_unit_field(pos, quick_mod(dir + BUILDING_TILE_SHIFTS[i], 6))].stack = -1;
    // board[move->pos + SHIFTS[ move->dir ]].stack = -1;
    // board[move->pos + SHIFTS[ quick_mod(move->dir + 1, 6)]].stack = -1;
    // board[move->pos + SHIFTS[ quick_mod(move->dir + 5, 6)]].stack = -1;
    structuresAfterBoardIsBuiltAreBuilt = false;
}

void SplitsGame::makeInitial(InitialMove* move)
{
    unsigned int cp = curPlayer();
    stacks[cp].push_back(move->pos);
    board[move->pos].stack = TOKEN_QUANTITY;
    board[move->pos].stacksIndex = stacks[cp].size()-1;
}

bool SplitsGame::canMoveInitial(InitialMove* move)
{
    int pos = move->pos;
    unsigned int historySize = history.size();
    return
        (historySize == INITIAL_MOVE_INDEX || historySize == INITIAL_MOVE_INDEX+1)
        && board[pos].stack == 0
        && fieldTouchesOuterBorder(pos);
}

void SplitsGame::undoInitial(InitialMove* move)
{
    board[move->pos].stack = 0;
    stacks[curPlayer()].pop_back();
}

// bool SplitsGame::isFinished()
// {
//     vector<Move*> moves = getPossibleMoves();
//     unsigned int size = moves.size();

//     for(unsigned int i = 0; i < size; ++i)
//         delete(moves[i]);

//     return size == 0;
// }

bool SplitsGame::isFinished()
{
    unsigned int size;
    getPossibleMoves(&size); // discarding the result; only size important
    return size == 0;
}

unsigned int SplitsGame::getWinner()
{
    return
        curPlayer() ^ 1;
}

GamePhase SplitsGame::gamePhase()
{
    unsigned int hsize = history.size();
    if (hsize >= NO_NONNORMAL_MOVES) return Normal;
    else if (hsize < NO_BUILDING_MOVES) return Building;
    else return Initial;
}

int SplitsGame::curPlayer()
{
    return (history.size() & 1);
}

int SplitsGame::curPlayerSign()
{
    int cp = curPlayer();
    return cp*2-1;
}

int SplitsGame::lastEmptyField(int pos, int dir)
{
    int result = -1;
    do
    {
        pos = shift_unit_field(pos, dir);
        ++result;
    } while (board[pos].stack == 0);
    return result;
}

void* SplitsGame::getPossibleMoves(unsigned int* size)
{
    if (!possibleMovesUpToDate) updatePossibleMoves();
    *size = possibleMovesSize;
    return possibleMoves;
}

vector<Move*> SplitsGame::getPossibleMoves()
{    
    int historySize = history.size();
    if (historySize >= NO_NONNORMAL_MOVES) return getPossibleNormalMoves();
    else if (historySize < NO_BUILDING_MOVES) return getPossibleBuildingMoves();
    else // if (historySize == NO_BUILDING_MOVES(lub +1))
        return getPossibleInitialMoves();
}

void SplitsGame::updatePossibleMoves()
{
    possibleMovesSize = 0;
    possibleMovesUpToDate = true;
    switch (gamePhase())
    {
    case Building: updateBuildingMoves(); break;
    case Initial: updateInitialMoves(); break;
    case Normal: updateNormalMoves(); break;
    };
    if (possibleMovesSize >= 200) printf("------------------------------something terrible happened\n");
}

void SplitsGame::updateBuildingMoves()
{
    for (unsigned int i = 0; i < fields.size(); ++i)
        addPossibleBuildingMovesForField(fields[i]);
}

void SplitsGame::addPossibleNormalMovesForStackInDir(int stackPos, int dir)
{
    NormalMove* moves = (NormalMove*) possibleMoves;
    int dist = lastEmptyField(stackPos, dir);

    if (dist == 0) return;

    for(int i = 1; i <= board[stackPos].stack-1; ++i)
    {
        moves[possibleMovesSize] = NormalMove(stackPos, i, shift_field(stackPos, dir, dist));
        ++possibleMovesSize;
    }
}

void SplitsGame::addPossibleNormalMovesForStackInDir(int stackPos, vector<Move*>* moves, int dir)
{
    int dist = lastEmptyField(stackPos, dir);

    if (dist == 0) return;

    for(int i = 1; i <= board[stackPos].stack-1; ++i)
    {
        moves->push_back((Move*) new NormalMove(stackPos, i, shift_field(stackPos, dir, dist))); // 'new' ok here
    }
}

void SplitsGame::addPossibleNormalMovesForStack(int stackPos)
{
    for (int i = 0; i < 6; ++i)
    {
        addPossibleNormalMovesForStackInDir(stackPos, i);
    }
}

void SplitsGame::addPossibleNormalMovesForStack(int stackPos, vector<Move*>* moves)
{
    for (int i = 0; i < 6; ++i)
    {
        addPossibleNormalMovesForStackInDir(stackPos, moves, i);
    }
}

vector<Move*> SplitsGame::getPossibleNormalMoves()
{
    vector<Move*> result;
    int cp = curPlayer();
    for (unsigned int i = 0; i < stacks[cp].size(); ++i)
    {
        addPossibleNormalMovesForStack(stacks[cp][i], &result);
    }
    return result;
}


void SplitsGame::updateNormalMoves()
{
    int cp = curPlayer();
    for (unsigned int i = 0; i < stacks[cp].size(); ++i)
    {
        addPossibleNormalMovesForStack(stacks[cp][i]);
    }
}

void SplitsGame::addPossibleBuildingMovesForField(int pos)
{
    int dr, mdir;
    int touching_pos, mpos;

    BuildingMove move;
    BuildingMove* moves = (BuildingMove*) possibleMoves;

    for (int i = 0; i < 6; ++i)
    {
        touching_pos = shift_unit_field(pos, i);
        for (int j = 0; j < 6; ++j) // przypadek, gdy sasiad jest jednym z srodkowych heksow plytki
        {
            move = BuildingMove(touching_pos, j);
            if (canMoveBuilding(&move))
            {
                //moves->push_back(move);
                moves[possibleMovesSize] = move;
                ++possibleMovesSize;
            }
            //else delete(move);
        }

        for (int j = 0; j < 4; ++j)
        {
            dr = quick_mod( quick_mod(i+4, 6)+j, 6);
            mpos = shift_unit_field(touching_pos, dr);
            mdir = quick_mod(i+j, 6);
            move = BuildingMove(mpos, mdir);
            if (canMoveBuilding(&move))
            {
                //moves->push_back(move);
                moves[possibleMovesSize] = move;
                ++possibleMovesSize;
            }
            //else delete(move);
        }
    }   
}

void SplitsGame::addPossibleBuildingMovesForField(int pos, std::vector<Move*>* moves)
{
    BuildingMove* move;
    int dr, mdir;
    int touching_pos, mpos;

    for (int i = 0; i < 6; ++i)
    {
        touching_pos = shift_unit_field(pos, i);
        for (int j = 0; j < 6; ++j) // przypadek, gdy sasiad jest jednym z srodkowych heksow plytki
        {
            move = new BuildingMove(touching_pos, j); // 'new' ok here
            if (canMoveBuilding(move))
            {
                moves->push_back(move);
            }
            else delete(move);
        }

        for (int j = 0; j < 4; ++j)
        {
            dr = quick_mod( quick_mod(i+4, 6)+j, 6);
            mpos = shift_unit_field(touching_pos, dr);
            mdir = quick_mod(i+j, 6);
            move = new BuildingMove(mpos, mdir); // 'new' ok here
            if (canMoveBuilding(move))
            {
                moves->push_back(move);
            }
            else delete(move);
        }
    }
}

vector<Move*> SplitsGame::getPossibleBuildingMoves()
{
    vector<Move*> result;
    for (unsigned int i = 0; i < fields.size(); ++i)
        addPossibleBuildingMovesForField(fields[i], &result);
    return result;
}

void SplitsGame::updateInitialMoves()
{
    InitialMove* moves = (InitialMove*) possibleMoves;
    for (unsigned int i = 0; i < fields.size(); ++i)
    {
        if (board[fields[i]].stack == 0 && fieldTouchesOuterBorder(fields[i]))
        {
            moves[possibleMovesSize] = InitialMove(fields[i]);
            ++possibleMovesSize;
        }
    }
}

vector<Move*> SplitsGame::getPossibleInitialMoves()
{
    vector<Move*> result;
    for (unsigned int i = 0; i < fields.size(); ++i)
    {
        if (board[fields[i]].stack == 0 && fieldTouchesOuterBorder(fields[i]))
            result.push_back(new InitialMove(fields[i])); //'new' ok here
    }
    return result;
}

BuildingMove::BuildingMove(int pos, int dir)
{
    if (dir > 0 && dir < 4)
    {
        this->pos = pos;
        this->dir = dir;
    }
    else // trzeba znormalizowac - dir \in {1,2,3}
    {
        this->pos = shift_unit_field(pos, dir);
        this->dir = quick_mod(dir+3, 6);
    }
}

InitialMove::InitialMove(int pos)
{
    this->pos = pos;
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

bool BuildingMove::canMove(SplitsGame* game)
{
    return game->canMoveBuilding(this);
}

bool NormalMove::canMove(SplitsGame* game)
{
    return game->canMoveNormal(this);
}

bool InitialMove::canMove(SplitsGame* game)
{
    return game->canMoveInitial(this);
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

Move::Move() {}

NormalMove::NormalMove(int source, int quantity, int target)
{
    this->source = source;
    this->quantity = quantity;
    this->target = target;
}

string InitialMove::prettyDesc()
{
    string result = "initial\t(";
    int x,y;
    double_coord(pos, x, y);

    result += boost::lexical_cast<string>(x);
    result += ", ";
    result += boost::lexical_cast<string>(y);
    result += ")";

    return result;
}

string BuildingMove::prettyDesc()
{
    string result = "building\t(";
    int x,y;
    double_coord(pos, x, y);

    result += boost::lexical_cast<string>(x);
    result += ", ";
    result += boost::lexical_cast<string>(y);

    result += ")-->";
    result += boost::lexical_cast<string>(dir);

    return result;
}

string NormalMove::prettyDesc()
{
    string result = "normal\t(";
    int sx, sy, tx, ty;
    double_coord(source, sx, sy);
    double_coord(target, tx, ty);

    result += boost::lexical_cast<string>(sx);
    result += ", ";
    result += boost::lexical_cast<string>(sy);

    result += ")--";
    result += boost::lexical_cast<string>(quantity);
    result += "-->(";

    result += boost::lexical_cast<string>(tx);
    result += ", ";
    result += boost::lexical_cast<string>(ty);

    result += ") dir: ";
    result += boost::lexical_cast<string>(SplitsGame::calcDir(source, target, NULL));
    result += "|";

    return result;
}

Move* BuildingMove::copy(BuildingMove* move)
{
    return new BuildingMove(move->pos, move->dir); // 'new' ok here
}

Move* InitialMove::copy(InitialMove* move)
{
    return new InitialMove(move->pos); // 'new' ok here
}

Move* NormalMove::copy(NormalMove* move)
{
    return new NormalMove(move->source, move->quantity, move->target); // 'new' ok here
}

Move* BuildingMove::copy()
{
    return new BuildingMove(pos, dir); // 'new' ok here
}

Move* InitialMove::copy()
{
    return new InitialMove(pos); // 'new' ok here
}

Move* NormalMove::copy()
{
    return new NormalMove(source, quantity, target); // 'new' ok here
}

vector<Move*> SplitsGame::getHistory()
{
    return history;
}

string SplitsGame::getStacksDesc()
{
    string result = "";
    int cp = curPlayer();
    result += "cp = ";
    result += boost::lexical_cast<string>(cp);
    result += "\n";
    for (cp = 0; cp < 2; ++cp)
    {
        result += "stacks[cp]: ";
        for (unsigned int i = 0; i < stacks[cp].size(); ++i)
        {
            int pos = stacks[cp][i];
            int x,y;
            double_coord(pos, x, y);
            result += "(";
            result += boost::lexical_cast<string>(x);
            result += ", ";
            result += boost::lexical_cast<string>(y);
            result += ") ";
        }
        result += "\n";
    }
    return result;
}

string SplitsGame::getDesc()
{
    string result = "possible moves:\n";
    if (!possibleMovesUpToDate) updatePossibleMoves();

    for (unsigned int i = 0; i < possibleMovesSize; ++i)
    {
        result += boost::lexical_cast<string>(i);
        result += ":\t";
        result += getPrettyDescMove(rawPossibleMoveOfIndex(possibleMoves, i, gamePhase()));
        result += "\n";
    }

    result += getPrettyHistory();
    result += "\n";

    result += getStacksDesc();
    //string result = "fields: ";
    //result += boost::lexical_cast<string>(fields.size());
    // for (unsigned int i = 0; i < fields.size(); ++i)
    // {
        
    // }

    // result += "outerBorder:\n";
    // for (int i = 0; i < MAX_BOARD_SIZE; ++i)
    // {
    //     result += boost::lexical_cast<string>(i);
    //     result += ":\t";
    //     for (int j = 0; j < MAX_BOARD_SIZE; ++j)
    //     {
    //         result += "(";
    //         result += boost::lexical_cast<string>(j);
    //         result += ":";
    //         result += boost::lexical_cast<string>(outerBorder[i*MAX_BOARD_SIZE+j]);
    //         result += ") ";
    //     }
    //     result += "\n";
    // }

    return result;
}

string SplitsGame::getBoardDesc()
{
    string result;
    result += "board:\n";
    for (int i = 23; i < 38; ++i)
    {
        result += boost::lexical_cast<string>(i);
        result += ":\t";
        for (int j = 23; j < 38; ++j)
        {
            result += "(";
            result += boost::lexical_cast<string>(j);
            result += ":";
            result += boost::lexical_cast<string>(board[i*MAX_BOARD_SIZE+j].stack);
            result += ", ";
            result += boost::lexical_cast<string>(board[i*MAX_BOARD_SIZE+j].stacksIndex);
            result += ") ";
        }
        result += "\n";
    }
    return result;
}

string SplitsGame::getPrettyDescMove(Move* move)
{
    string result;
    switch (gamePhase())
    {
    case Building:
    {
        int pos = ((BuildingMove*) move)->pos;
        int dir = ((BuildingMove*) move)->dir;
        result = "building\t(";
        int x,y;
        double_coord(pos, x, y);

        result += boost::lexical_cast<string>(x);
        result += ", ";
        result += boost::lexical_cast<string>(y);

        result += ")-->";
        result += boost::lexical_cast<string>(dir);
        break;
    }
    case Initial:
    {
        int pos = ((InitialMove*) move)->pos;
        result = "initial\t(";
        int x,y;
        double_coord(pos, x, y);

        result += boost::lexical_cast<string>(x);
        result += ", ";
        result += boost::lexical_cast<string>(y);
        result += ")";
        break;
    }
    case Normal:
    {
        int source = ((NormalMove*) move)->source;
        int target = ((NormalMove*) move)->target;
        int quantity = ((NormalMove*) move)->quantity;
        result = "normal\t(";
        int sx, sy, tx, ty;
        double_coord(source, sx, sy);
        double_coord(target, tx, ty);

        result += boost::lexical_cast<string>(sx);
        result += ", ";
        result += boost::lexical_cast<string>(sy);

        result += ")--";
        result += boost::lexical_cast<string>(quantity);
        result += "-->(";

        result += boost::lexical_cast<string>(tx);
        result += ", ";
        result += boost::lexical_cast<string>(ty);

        result += ") dir: ";
        result += boost::lexical_cast<string>(SplitsGame::calcDir(source, target, NULL));
        result += "|";
        break;
    }
    };
    return result;
}

Field* SplitsGame::getField(int pos)
{
    return board+pos;
}

string SplitsGame::getPrettyHistory()
{
    string result = "history:\n";
    for (unsigned int i = 0; i < history.size(); ++i)
    {
        Move* move = history[i];
        result += boost::lexical_cast<string>(i);
        result += ": ";
        result += move->prettyDesc();
        result += "\n";
    }
    return result;
}

Move::~Move() {}
NormalMove::NormalMove() {}
NormalMove::~NormalMove() {}
BuildingMove::BuildingMove() {}
BuildingMove::~BuildingMove() {}
InitialMove::InitialMove() {}
InitialMove::~InitialMove() {}

Grader::Grader() {}
Grader::~Grader() {}


vector<int>* Grader::getStacks(SplitsGame* game)
{
    return game->stacks;
}

Field* Grader::board(SplitsGame* game, int pos)
{
    return game->board + pos;
}

int Grader::gradeMove(SplitsGame* game, Move* move)
{
    game->makeMove(move);
    int result = grade(game);
    game->undoMove();
    return result;
}

bool Grader::better(SplitsGame* game, int gradeA, int gradeB)
{
    int cps = game->curPlayerSign();
    return cps*(gradeA-gradeB) > 0;
}

Hasher::Hasher() {}
Hasher::~Hasher() {}

unsigned long long Hasher::getHash()
{
    return hash;
}

unsigned long long Hasher::getHashMove(Move* move, SplitsGame* game, GamePhase phase)
{
    unsigned long oldHash = hash;
    makeMove(move, game, phase);
    unsigned long result = hash;
    hash = oldHash;
    return result;
}
