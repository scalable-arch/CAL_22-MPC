#include <cmath>
#include <vector>
#include <iostream>

#include "PredictorModule.h"

namespace comp
{
/*** WeightBasePredictor ***/
WeightBasePredictor::WeightBasePredictor(int rootIndex, int lineSize,
    std::vector<int> baseIndexTable, std::vector<float> weightTable)
  : PredictorModule(rootIndex, lineSize)
{
  m_Table.RootIndex = rootIndex;
  m_Table.LineSize = lineSize;

  m_Table.BaseIndexTable = baseIndexTable;
  m_Table.WeightTable = weightTable;

  m_ShiftDistanceTable.resize(m_LineSize);
  for (int i = 0; i < m_LineSize; i++)
  {
    if (i == m_RootIndex)
    {
      m_ShiftDistanceTable[i] = 0;
    }
    else
    {
      float weight = m_Table.WeightTable[i];
      int shiftDistance = (int)log2f(weight);

      m_ShiftDistanceTable[i] = shiftDistance;
    }
  }
}

Symbol WeightBasePredictor::PredictLine(std::vector<uint8_t> &cacheLine)
{
  Symbol predictedLine;
  uint8_t root, predicted;

  predictedLine.SetSize(m_LineSize);
  predictedLine.SetRootIndex(m_RootIndex);

  for (int i = 0; i < m_LineSize; i++)
  {
    if (i == m_RootIndex)
    {
      root = cacheLine[i];

      predictedLine[i] = root;
    }
    else
    {
      int baseIndex = m_Table.BaseIndexTable[i];

      uint8_t base = cacheLine[baseIndex];
      int shiftDistance = m_ShiftDistanceTable[i];
      if (shiftDistance < 0)
        predicted = base >> abs(shiftDistance);
      else
        predicted = base << shiftDistance;

      predictedLine[i] = predicted;
    }
  }
  return predictedLine;
}

/*** DiffBasePredictor ***/
DiffBasePredictor::DiffBasePredictor(int rootIndex, int lineSize,
    std::vector<int> baseIndexTable, std::vector<int> diffTable)
  : PredictorModule(rootIndex, lineSize)
{
  m_Table.RootIndex = rootIndex;
  m_Table.LineSize = lineSize;

  m_Table.BaseIndexTable = baseIndexTable;
  m_Table.DiffTable = diffTable;
}

Symbol DiffBasePredictor::PredictLine(std::vector<uint8_t> &cacheLine)
{
  Symbol predictedLine;
  uint8_t root, predicted;

  predictedLine.SetSize(m_LineSize);
  predictedLine.SetRootIndex(m_RootIndex);

  for (int i = 0; i < m_LineSize; i++)
  {
    if (i == m_RootIndex)
    {
      root = cacheLine[i];

      predictedLine[i] = root;
    }
    else
    {
      int diff = m_Table.DiffTable[i];
      int baseIndex = m_Table.BaseIndexTable[i];

      uint8_t base = cacheLine[baseIndex];
      predicted = (uint8_t)diff + base;

      predictedLine[i] = predicted;
    }
  }
  return predictedLine;
}

/*** OneBasePredictor ***/
Symbol OneBasePredictor::PredictLine(std::vector<uint8_t> &cacheLine)
{
  m_LineSize = cacheLine.size();

  Symbol predictedLine;
  uint8_t root, predicted;

  predictedLine.SetSize(m_LineSize);
  predictedLine.SetRootIndex(m_RootIndex);

  for (int i = 0; i < m_LineSize; i++)
  {
    root = cacheLine[m_RootIndex];
  
    predictedLine[i] = root;
  }
  return predictedLine;
}

/*** ConsecutiveBasePredictor ***/
Symbol ConsecutiveBasePredictor::PredictLine(std::vector<uint8_t> &cacheLine)
{
  m_LineSize = cacheLine.size();

  Symbol predictedLine;
  uint8_t root, predicted;

  predictedLine.SetSize(m_LineSize);
  predictedLine.SetRootIndex(m_RootIndex);

  std::vector<uint8_t> inputLine = cacheLine;
  if (mb_Byteplane)
  {
    int idx = 0;
    for (int plane = 3; plane >= 0; plane--)
    {
      for (int i = plane; i < m_LineSize; i += 4)
      {
        inputLine[idx] = cacheLine[i];
        idx++;
      }
    }
  }

  for (int i = 0; i < m_LineSize; i++)
  {
    if (i == m_RootIndex)
    {
      root = inputLine[i];

      predictedLine[i] = root;
    }
    else
    {
      predicted = inputLine[i - 1];

      predictedLine[i] = predicted;
    }
  }
  return predictedLine;
}


}

