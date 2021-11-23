#pragma once

#include "PredCompModule.h"

namespace comp
{

class XORModule
{
friend class PredCompModule;

public:
  // constructor
  XORModule(bool consecutiveXOR)
    : mb_ConsecutiveXOR(consecutiveXOR) {}

  Binary ProcessLine(Binary &bitplane);

private:
  bool mb_ConsecutiveXOR;
};

}

