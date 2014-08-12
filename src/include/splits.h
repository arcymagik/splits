#ifndef SPLITS_HH
#define SPLITS_HH

#include <cstdlib>

#include <vector>
#include <string>

#define TOKEN_QUANTITY 16

enum GamePhase {Building, Initial, Normal};

class SplitsGame;
class Move;
class Field;

class Grader
{
public:
    Grader();
    virtual ~Grader();
    virtual int grade(SplitsGame* game) = 0;
    virtual int gradeMove(SplitsGame* game, Move* move);
    bool better(SplitsGame* game, int gradeA, int gradeB);
protected:
    std::vector<int>* getStacks(SplitsGame* game);
    Field* board(SplitsGame* game, int pos);
};

class Hasher
{
public:
    Hasher();
    virtual ~Hasher();
    virtual unsigned long long getHash();
    virtual unsigned long long getHashMove(Move* move, SplitsGame* game, GamePhase phase);
    virtual void makeMove(Move* move, SplitsGame* game, GamePhase phase) = 0;
    virtual void undoMove(Move* move, SplitsGame* game, GamePhase phase) = 0;
protected:
    unsigned long long hash;
};

class Move
{
public:
    Move();
    virtual ~Move();
    virtual Move* copy() = 0;
    virtual std::string prettyDesc() = 0;
    virtual void makeHere(SplitsGame* game) = 0;
    virtual bool canMove(SplitsGame* game) = 0;
    virtual void undo(SplitsGame* game) = 0;
};

class BuildingMove : public Move // 7 pierwszych ruchow w grze jest tego typu
{
public:
    BuildingMove();
    BuildingMove(int pos, int dir);
    virtual ~BuildingMove();
    virtual Move* copy();
    virtual std::string prettyDesc();
    virtual void makeHere(SplitsGame* game);
    virtual bool canMove(SplitsGame* game);
    virtual void undo(SplitsGame* game);

    static Move* copy(BuildingMove* move);

    int pos; // pozycja pierwszego konca

    int dir; // kierunek do drugiego konca (1-3 dla jednoznacznosci)
};

class NormalMove : public Move // 10+ ruchy sa tego typu
{
public:
    NormalMove();
    NormalMove(int source, int quantity, int target);
    virtual ~NormalMove();
    virtual Move* copy();
    virtual std::string prettyDesc();
    virtual void makeHere(SplitsGame* game);
    virtual bool canMove(SplitsGame* game);
    virtual void undo(SplitsGame* game);

    static Move* copy(NormalMove* move);

    int source;
    int quantity;
    int target;
};

class InitialMove : public Move // 8. i 9. ruch jest tego typu
{
public:
    InitialMove();
    InitialMove(int pos);
    virtual ~InitialMove();
    virtual Move* copy();
    virtual std::string prettyDesc();
    virtual void makeHere(SplitsGame* game);
    virtual bool canMove(SplitsGame* game);
    virtual void undo(SplitsGame* game);

    static Move* copy(InitialMove* move);
    
    int pos;
};

class Field
{
public:
    int stack;
    unsigned int stacksIndex; // index in SplitGame::stack[gracz, ktory postawil tu kamienie]; valid only for stack > 1; maybe change to unsigned short?
};

class SplitsGame
{
private:
    std::vector<Move*> history;
    Field* board;

    bool* outerBorder;
    bool structuresAfterBoardIsBuiltAreBuilt;

    std::vector<int> stacks[2];
    std::vector<int> fields;

    void* possibleMoves;
    unsigned int possibleMovesSize;
    bool possibleMovesUpToDate;

    friend class Grader;

public:
    SplitsGame();
    ~SplitsGame();

    void makeMove(Move* move);
    void makeIndexedMove(unsigned int index); // index z tablicy possibleMoves
    bool canMove(Move* move);
    int undoMove();

    std::vector<Move*> getPossibleMoves();
    void* getPossibleMoves(unsigned int* size);
    bool isFinished();
    unsigned int getWinner(); //oczywiscie to ma sens tylko gdy gra sie skonczyla
    GamePhase gamePhase();

    int curPlayer();
    int curPlayerSign();

    // metody do ogladania wnetrza
    Field* getField(int pos);

    std::vector<Move*> getHistory();
    std::string getPrettyHistory();
    std::string getDesc();
    std::string getPrettyDescMove(Move* move);
    bool stacksWrong();

    std::string getBoardDesc();

    // pomocnicze
    void makeNormal(NormalMove* move);
    void makeBuilding(BuildingMove* move);
    void makeInitial(InitialMove* move);
    bool canMoveNormal(NormalMove* move);
    bool canMoveInitial(InitialMove* move);
    bool canMoveBuilding(BuildingMove* move);
    void undoNormal(NormalMove* move);
    void undoBuilding(BuildingMove* move);
    void undoInitial(InitialMove* move);

    std::vector<Move*> getPossibleNormalMoves();
    std::vector<Move*> getPossibleBuildingMoves();
    std::vector<Move*> getPossibleInitialMoves();

    bool pathIsEmpty(int source, int dir, int dist);
    bool fieldBehindUnavailable(int field, int dir);
    bool tileTouchesOlderFields(int pos, int dir);
    bool fieldTouchesOuterBorder(int pos);
    int lastEmptyField(int pos, int dir);
    bool sourceInStacks(int source);

    void buildStructuresAfterBoardIsBuilt();

    bool stackBlocked(int pos);

    static int calcDir(int source, int target, int* dist);
    static bool fieldOutOfBoard(int pos, int dir);
    static Move* possibleMoveOfIndex(void* moves, unsigned int index, GamePhase phase);
    static Move* rawPossibleMoveOfIndex(void* moves, unsigned int index, GamePhase phase);

private:
    bool touchesInOneOf4(int pos, int dir1, int dir2);

    void addPossibleNormalMovesForStack(int stackPos, std::vector<Move*>* moves);
    void addPossibleNormalMovesForStackInDir(int stackPos, std::vector<Move*>* moves, int dir);
    void addPossibleBuildingMovesForField(int pos, std::vector<Move*>* moves);

    void updatePossibleMoves();
    void updateBuildingMoves();
    void updateInitialMoves();
    void updateNormalMoves();

    void addPossibleNormalMovesForStack(int stackPos);
    void addPossibleNormalMovesForStackInDir(int stackPos, int dir);
    void addPossibleBuildingMovesForField(int pos);    
};

#endif
