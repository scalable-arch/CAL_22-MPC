#include "PredictorModule.h"
#include "ResidueModule.h"

namespace comp
{

ResidueModule::ResidueModule(PredictorModule *predModule)
  : m_RootIndex(predModule->m_RootIndex), mp_PredictorModule(predModule) {}

Symbol ResidueModule::ProcessLine(std::vector<uint8_t> &cacheLine)
{
  Symbol predictedLine;
  Symbol residueLine;
  uint8_t root, residue;

  predictedLine = mp_PredictorModule->PredictLine(cacheLine);

  residueLine.SetSize(predictedLine.GetLineSize());
  residueLine.SetRootIndex(m_RootIndex);

  // make residues
  //// root should be placed in index0
  root = cacheLine[m_RootIndex];
  residueLine[0] = root;
  int j = 1;
  for (int i = 0; i < predictedLine.GetLineSize(); i++)
  {
    if (i == m_RootIndex)
      continue;
    else
    {
      residue = cacheLine[i] - predictedLine[i];
      residueLine[j] = residue;
      j++;
    }
  }

  return residueLine;
}

}

