#include <cmath>

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

  residueLine.SetSize(predictedLine.GetCachelineSize());
  residueLine.SetRootIndex(m_RootIndex);

  // make residues
  //// root should be placed in index0
  root = cacheLine[m_RootIndex];
  residueLine[0] = root;
  int j = 1;
  for (int i = 0; i < predictedLine.GetCachelineSize(); i++)
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

double ResidueModule::GetMAE(std::vector<uint8_t> &dataLine)
{
  Symbol predictedLine;
  predictedLine = mp_PredictorModule->PredictLine(dataLine);
  const int lineSize = predictedLine.GetCachelineSize();

  double mae = 0;
  for (int i = 0; i < lineSize; i++)
  {
    uint8_t residue = dataLine[i] - predictedLine[i];
    mae += abs((double)residue);
  }
  mae /= (double)lineSize;
  return mae;
}

double ResidueModule::GetMSE(std::vector<uint8_t> &dataLine)
{
  Symbol predictedLine;
  predictedLine = mp_PredictorModule->PredictLine(dataLine);
  const int lineSize = predictedLine.GetCachelineSize();

  double mse = 0;
  for (int i = 0; i < lineSize; i++)
  {
    uint8_t residue = dataLine[i] - predictedLine[i];
    mse += pow((double)residue, 2);
  }
  mse /= (double)lineSize;
  return mse;
}

}

