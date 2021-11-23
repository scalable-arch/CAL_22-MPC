#pragma once

#include "PredCompModule.h"

namespace comp
{

class BitplaneModule
{
friend class PredCompModule;

public:
  Binary ProcessLine(Symbol &residueLine);

private:
  std::vector<uint8_t> convertToBitVector(uint8_t symbol);
  Binary transposeBinaryArray(Binary &binary);

private:
  const uint8_t m_Mask = 0x01;
};

}
