#ifndef TRANSPOSITION_TABLE_HH
#define TRANSPOSITION_TABLE_HH

#include "splits.h"

class TTEntry
{
// razem mamy tu 16 bajtow
public:
    unsigned long long hash;
    unsigned long long data;
};

class TranspositionTable
{
public:
    TTEntry* table;
    unsigned int size;
    unsigned int log;
    unsigned int mask;

    TranspositionTable();
    TranspositionTable(unsigned int size_log);
    ~TranspositionTable();
    void push(TTEntry entry);
    TTEntry get(unsigned long long hash);
};

#endif
