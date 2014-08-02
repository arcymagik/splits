#ifndef SIMPLE_GRADER_HH
#define SIMPLE_GRADER_HH

#include "splits.h"

class SimpleGrader : public Grader
{
public:
    SimpleGrader();
    ~SimpleGrader();
    virtual int grade(SplitsGame* game);
};

#endif
