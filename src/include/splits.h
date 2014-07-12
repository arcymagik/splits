#ifndef SPLITS_HH
#define SPLITS_HH

#include <cstdlib>

#include <vector>

class SplitsGame;
class Move;

typedef Move HistoryMove;

class Move
{
public:
    virtual void makeHere(SplitsGame* game) = 0;
    virtual void undo(SplitsGame* game) = 0;
};

class BuildingMove : Move
{
public:
    virtual void makeHere(SplitsGame* game);
    virtual void undo(SplitsGame* game);

    int pos; // pozycja pierwszego konca

    int dir; // kierunek do drugiego konca (0 - 5)
};

class NormalMove : Move
{
public:
    virtual void makeHere(SplitsGame* game);
    virtual void undo(SplitsGame* game);

    int source;
    int quantity;
    int target;
};

class InitialMove : Move
{
public:
    virtual void makeHere(SplitsGame* game);
    virtual void undo(SplitsGame* game);
    
    int pos;
};

class Field
{
public:
    int stack;
};

class SplitsGame
{
public:
    SplitsGame();
    ~SplitsGame();

    void makeMove(Move* move);
    int undoMove();

    std::vector<Move*> getPossibleMoves();
    int isFinished(); //0,1 - strony, 2 - trwa rozgrywka

    // pomocnicze
    void makeNormal(NormalMove* move);
    void makeBuilding(BuildingMove* move);
    void makeInitial(InitialMove* move);
    void undoNormal(NormalMove* move);
    void undoBuilding(BuildingMove* move);
    void undoInitial(InitialMove* move);
private:
    std::vector<HistoryMove*> history;
    Field* board;
};

#endif
