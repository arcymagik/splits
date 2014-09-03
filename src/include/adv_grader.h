#ifndef ADVANCED_GRADER_HH
#define ADVANCED_GRADER_HH

#include "splits.h"

class AdvancedGrader : public Grader
{
public:
    AdvancedGrader();
    ~AdvancedGrader();
    virtual int grade(SplitsGame* game);
};

#endif
