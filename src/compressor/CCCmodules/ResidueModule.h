#pragma once

#include <vector>

#include "PredCompModule.h"

namespace comp
{

class PredictorModule;

class ResidueModule
{
friend class PredCompModule;

public:
  ResidueModule(PredictorModule *predModule);

  Symbol ProcessLine(std::vector<uint8_t> &cacheLine);

private:
  int m_RootIndex;
  PredictorModule *mp_PredictorModule;
};

}
