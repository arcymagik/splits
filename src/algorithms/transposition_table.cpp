#include "transposition_table.h"
#include <cstring>

using namespace std;

TranspositionTable::TranspositionTable()
    : TranspositionTable(20) // 16MB
{
}

TranspositionTable::TranspositionTable(unsigned int size_log)
{
    log = size_log;
    size = 1 << size_log;
    unsigned int ones = ~((unsigned int) 0);
    mask = ~((ones >> size_log) << size_log);
    table = (TTEntry*) malloc(size*sizeof(TTEntry));
    memset(table, 0, size*sizeof(TTEntry));
}

TranspositionTable::~TranspositionTable()
{
    free(table);
}

void TranspositionTable::push(TTEntry entry)
{
    unsigned int index = entry.hash & mask;
    table[index] = entry;
}

TTEntry TranspositionTable::get(unsigned long long hash)
{
    unsigned int index = hash & mask;
    return table[index];
}
