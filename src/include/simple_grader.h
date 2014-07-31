#ifndef SIMPLE_GRADER_HH
#define SIMPLE_GRADER_HH

#include "splits.h"

class SimpleGrader : public Grader
{
public:
    virtual int grade(SplitsGame* game);
};

#endif
