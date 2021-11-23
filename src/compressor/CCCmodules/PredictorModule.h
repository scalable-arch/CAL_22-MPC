#pragma once

#include "ResidueModule.h"

namespace comp
{

/*** base class ***/
class PredictorModule
{
friend class ResidueModule;

public:
  PredictorModule(int rootIndex, int lineSize)
    : m_RootIndex(rootIndex), m_LineSize(lineSize) {}
  
  virtual Symbol PredictLine(std::vector<uint8_t> &cacheLine) = 0;

protected:
  int m_RootIndex;
  int m_LineSize;
};

/*** inherited classes ***/
// Weight Base Predictor
struct WeightBaseTable
{
  int RootIndex;
  int LineSize;

  std::vector<int> BaseIndexTable;
  std::vector<float> WeightTable;
};

class WeightBasePredictor : public PredictorModule
{
public:
  WeightBasePredictor(int rootIndex, int lineSize,
      std::vector<int> baseIndexTable, std::vector<float> weightTable);
  
  Symbol PredictLine(std::vector<uint8_t> &cacheLine);

private:
  WeightBaseTable m_Table;
  std::vector<int> m_ShiftDistanceTable;
};

// Differece Base Predictor
struct DiffBaseTable
{
  int RootIndex;
  int LineSize;

  std::vector<int> BaseIndexTable;
  std::vector<int> DiffTable;
};

class DiffBasePredictor : public PredictorModule
{
public:
  DiffBasePredictor(int rootIndex, int lineSize,
      std::vector<int> baseIndexTable, std::vector<int> diffTable);

  Symbol PredictLine(std::vector<uint8_t> &cacheLine);

private:
  DiffBaseTable m_Table;
};

// One Base Predictor
class OneBasePredictor : public PredictorModule
{
public:
  OneBasePredictor(int rootIndex, int lineSize)
    : PredictorModule(rootIndex, lineSize) {}

  Symbol PredictLine(std::vector<uint8_t> &cacheLine);
};

// Consecutive Base Predictor
class ConsecutiveBasePredictor : public PredictorModule
{
public:
  ConsecutiveBasePredictor(int rootIndex, int lineSize, bool byteplane=true)
    : PredictorModule(rootIndex, lineSize), mb_Byteplane(byteplane) {}

  Symbol PredictLine(std::vector<uint8_t> &cacheLine);

private:
  bool mb_Byteplane;
};

}
