#include "FPCModule.h"
#include "PredCompModule.h"
#include "ScanModule.h"
#include "PatternModule.h"

namespace comp
{

void FPCModule::AddModule(PatternModule *patternModule)
{
  m_PatternModules.push_back(patternModule);
}

void FPCModule::RemoveModule(int number)
{
  m_PatternModules.erase(m_PatternModules.begin() + number);
}

int FPCModule::ProcessLine(Binary &scanned)
{
  int zrle = 0;
  int runLength = 0;
  int compressedSize = 0;

  for (int numRow = 0; numRow < scanned.GetRowSize(); numRow++)
  {
    if (isRowZeros(scanned[numRow]))
    {
      if (zrle == 0)
        zrle = 1;
      runLength++;
    }
    else
    {
      if (zrle == 1)
      {
        if (runLength > 1)
        {
          compressedSize += m_EncodingBitsSize[ZRLE];
          zrle = 0;
          runLength = 0;
        }
        else
        {
          compressedSize += m_EncodingBitsSize[Zero];
          zrle = 0;
          runLength = 0;
        }
      }
      if (isRowSingleOne(scanned[numRow]))
      {
        compressedSize += m_EncodingBitsSize[SingleOne];
      }
      else if (isRowTwoConsecOnes(scanned[numRow]))
      {
        compressedSize += m_EncodingBitsSize[TwoConsecOnes];
      }
      else if (isRowFrontHalfZeros(scanned[numRow]))
      {
        compressedSize += m_EncodingBitsSize[FrontHalfZeros];
      }
      else if (isRowBackHalfZeros(scanned[numRow]))
      {
        compressedSize += m_EncodingBitsSize[BackHalfZeros];
      }
      else
      {
        compressedSize += m_EncodingBitsSize[Uncompressible];
      }
    }
  }

  if (zrle == 1)
  {
    if (runLength > 1)
    {
      compressedSize += m_EncodingBitsSize[ZRLE];
    }
    else
    {
      compressedSize += m_EncodingBitsSize[Zero];
    }
  }
  return compressedSize;
}

bool FPCModule::isRowZeros(std::vector<uint8_t> &row)
{
  for (int pos = 0; pos < SCANNED_SYMBOLSIZE; pos++)
    if (row[pos] != 0x00)
      return false;
  return true;
}

bool FPCModule::isRowSingleOne(std::vector<uint8_t> &row)
{
  int oneCounter = 0;
  for (int i = 0; i < SCANNED_SYMBOLSIZE; i++)
  {
    if (row[i] == 0x01)
      oneCounter++;

    // check the number of one
    if (oneCounter > 1)
      return false;
  }
  return true;
}

bool FPCModule::isRowTwoConsecOnes(std::vector<uint8_t> &row)
{
  int oneCounter = 0;
  int *onesPosition = new int[2];

  for (int i = 0; i < SCANNED_SYMBOLSIZE; i++)
  {
    if (row[i] == 0x01)
    {
      onesPosition[oneCounter] = i;
      oneCounter++;
    }

    // check the number of ones
    if (oneCounter > 2)
    {
      delete[] onesPosition;
      return false;
    }
  }

  // check consecutiveness
  if (onesPosition[1] - onesPosition[0] == 1)
  {
    delete[] onesPosition;
    return true;
  }
  else
  {
    delete[] onesPosition;
    return false;
  }
}

bool FPCModule::isRowFrontHalfZeros(std::vector<uint8_t> &row)
{
  for (int i = 0; i < SCANNED_SYMBOLSIZE/2; i++)
    if (row[i] != 0x00)
      return false;
  return true;
}

bool FPCModule::isRowBackHalfZeros(std::vector<uint8_t> &row)
{
  for (int i = SCANNED_SYMBOLSIZE/2; i < SCANNED_SYMBOLSIZE; i++)
    if (row[i] != 0x00)
      return false;
  return true;
}










}
