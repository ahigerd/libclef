#ifndef CLEF_TESTSEQUENCE_H
#define CLEF_TESTSEQUENCE_H

#include "isequence.h"
#include <vector>

class TestSequence : public BaseSequence<> {
public:
  TestSequence(ClefContext* ctx);
};

#endif
