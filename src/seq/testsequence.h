#ifndef S2W_TESTSEQUENCE_H
#define S2W_TESTSEQUENCE_H

#include "isequence.h"
#include <vector>

class TestSequence : public BaseSequence<> {
public:
  TestSequence(S2WContext* ctx);
};

#endif
