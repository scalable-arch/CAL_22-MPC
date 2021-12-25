#ifndef __RESIDUE_MODULE_H__
#define __RESIDUE_MODULE_H__

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
  double GetMAE(std::vector<uint8_t> &dataLine);
  double GetMSE(std::vector<uint8_t> &dataLine);

private:
  int m_RootIndex;
  PredictorModule *mp_PredictorModule;
};

}

#endif
