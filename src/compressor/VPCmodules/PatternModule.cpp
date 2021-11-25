#include "PatternModule.h"
#include "PredCompModule.h"
#include "ScanModule.h"
#include <bits/stdint-uintn.h>

namespace comp
{
// Uncompressed pattern check module
compSizeList& UncompressedPatternModule::Compress(Binary &scanned, compSizeList &sizeList)
{
  for (int i = 0; i < sizeList.size(); i++)
  {
    // if the row is not compressed,
    // attach encoding prefix bits for uncompressed row
    if (sizeList[i] == SCANNED_SYMBOLSIZE)
      sizeList[i] += m_EncodingBits;
  }

  return sizeList;
}

// ZRLE & Zero pattern check module
compSizeList& ZerosPatternModule::Compress(Binary &scanned, compSizeList &sizeList)
{
  int &m_EncodingBitsZRLE = m_EncodingBits;

  bool zeroRuns = false;
  int runLength = 0;

  int lengthBits = (int)ceil(log2f((float)scanned.GetRowSize()));

  for (int i = 0; i < scanned.GetRowSize(); i++)
  {
    // if the row is already compressed,
    // do not compress further
    if (sizeList[i] != SCANNED_SYMBOLSIZE)
      continue;

    // compress rows
    if (is_rowAllZeros(scanned[i]))
    {
      zeroRuns = true;
      runLength++;

      sizeList[i] = 0;
    }
    else
    {
      if (zeroRuns)
      {
        // Zero runs
        if (runLength > 1)
          sizeList[i - 1] = (m_EncodingBitsZRLE + lengthBits);
        // One zero
        else
          sizeList[i - 1] = (m_EncodingBitsZero);
        
      }

      zeroRuns = false;
      runLength = 0;
    }
  }

  // check the last row
  if (zeroRuns)
  {
    // Zero runs
    if (runLength > 1)
      sizeList[sizeList.size() - 1] = (m_EncodingBitsZRLE + lengthBits);
    // One zero
    else
      sizeList[sizeList.size() - 1] = (m_EncodingBitsZero);
  }

  return sizeList;
}

bool ZerosPatternModule::is_rowAllZeros(std::vector<uint8_t> &row)
{
  for (int i = 0; i < row.size(); i++)
  {
    if (row[i] != 0x00)
      return false;
  }
  return true;
}

// Single One pattern check module
compSizeList& SingleOnePatternModule::Compress(Binary &scanned, compSizeList &sizeList)
{
  int positionBits = (int)ceil(log2f((float)scanned.GetColSize()));

  for (int i = 0; i < scanned.GetRowSize(); i++)
  {
    // if the row is already compressed,
    // do not compress further
    if (sizeList[i] != SCANNED_SYMBOLSIZE)
      continue;

    // compress row
    if (is_rowSingleOne(scanned[i]))
      sizeList[i] = (m_EncodingBits + positionBits);
  }

  return sizeList;
}

bool SingleOnePatternModule::is_rowSingleOne(std::vector<uint8_t> &row)
{
  int oneCounter = 0;

  for (int i = 0; i < row.size(); i++)
  {
    if (row[i] == 0x01)
      oneCounter++;

    // check the number of one
    if (oneCounter > 1)
      return false;
  }
  return true;
}

// Two Consecutive Ones pattern check module
compSizeList& TwoConsecutiveOnesPatternModule::Compress(Binary &scanned, compSizeList &sizeList)
{
  int positionBits = (int)ceil(log2f((float)scanned.GetColSize()));

  for (int i = 0; i < scanned.GetRowSize(); i++)
  {
    // if the row is already compressed,
    // do not compress further
    if (sizeList[i] != SCANNED_SYMBOLSIZE)
      continue;

    // compress row
    if (is_rowTwoConsecutiveOnes(scanned[i]))
      sizeList[i] = (m_EncodingBits + positionBits);
  }

  return sizeList;
}

bool TwoConsecutiveOnesPatternModule::is_rowTwoConsecutiveOnes(std::vector<uint8_t> &row)
{
  int oneCounter = 0;
  int *onesPosition = new int[2];

  for (int i = 0; i < row.size(); i++)
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

// Zeros Front pattern check module
compSizeList& ZerosFrontPatternModule::Compress(Binary &scanned, compSizeList &sizeList)
{
  int residualBits = scanned.GetColSize() - m_NumZeros;

  for (int i = 0; i < scanned.GetRowSize(); i++)
  {
    // if the row is already compressed,
    // do not compress further
    if (sizeList[i] != SCANNED_SYMBOLSIZE)
      continue;

    // compress row
    if (is_rowZerosFront(scanned[i]))
      sizeList[i] = (m_EncodingBits + residualBits);
  }

  return sizeList;
}

bool ZerosFrontPatternModule::is_rowZerosFront(std::vector<uint8_t> &row)
{
  for (int i = 0; i < m_NumZeros; i++)
    if (row[i] != 0x00)
      return false;
  return true;
}

// Zeros Front-Half pattern check module
compSizeList& ZerosFrontHalfPatternModule::Compress(Binary &scanned, compSizeList &sizeList)
{
  int residualBits = scanned.GetColSize() / 2;

  for (int i = 0; i < scanned.GetRowSize(); i++)
  {
    // if the row is already compressed,
    // do not compress further
    if (sizeList[i] != SCANNED_SYMBOLSIZE)
      continue;

    // compress row
    if (is_frontHalfZeros(scanned[i]))
      sizeList[i] = (m_EncodingBits + residualBits);
  }

  return sizeList;
}

bool ZerosFrontHalfPatternModule::is_frontHalfZeros(std::vector<uint8_t> &row)
{
  for (int i = 0; i < row.size() / 2; i++)
  {
    if (row[i] != 0)
      return false;
  }
  return true;
}

// Zeros Back pattern check module
compSizeList& ZerosBackPatternModule::Compress(Binary &scanned, compSizeList &sizeList)
{
  int residualBits = scanned.GetColSize() - m_NumZeros;

  for (int i = 0; i < scanned.GetRowSize(); i++)
  {
    // if the row is already compressed,
    // do not compress further
    if (sizeList[i] != SCANNED_SYMBOLSIZE)
      continue;

    // compress row
    if (is_rowZerosBack(scanned[i]))
      sizeList[i] = (m_EncodingBits + residualBits);
  }

  return sizeList;
}

bool ZerosBackPatternModule::is_rowZerosBack(std::vector<uint8_t> &row)
{
  for (int i = row.size() - 1; i >= row.size() - m_NumZeros; i--)
    if (row[i] != 0x00)
      return false;
  return true;
}

// Zeros Back-Half pattern check module
compSizeList& ZerosBackHalfPatternModule::Compress(Binary &scanned, compSizeList &sizeList)
{
  int residualBits = scanned.GetColSize() / 2;

  for (int i = 0; i < scanned.GetRowSize(); i++)
  {
    // if the row is already compressed,
    // do not compress further
    if (sizeList[i] != SCANNED_SYMBOLSIZE)
      continue;

    // compress row
    if (is_backHalfZeros(scanned[i]))
      sizeList[i] = (m_EncodingBits + residualBits);
  }

  return sizeList;
}

bool ZerosBackHalfPatternModule::is_backHalfZeros(std::vector<uint8_t> &row)
{
  for (int i = row.size() / 2; i < row.size(); i++)
  {
    if (row[i] != 0)
      return false;
  }
  return true;
}

// Masking pattern check module
compSizeList& MaskingPatternModule::Compress(Binary &scanned, compSizeList &sizeList)
{
  for (int i = 0; i < scanned.GetRowSize(); i++)
  {
    // if the row is already compressed,
    // do not compress further
    if (sizeList[i] != SCANNED_SYMBOLSIZE)
      continue;

    // compress row
    if (is_rowPatternMatched(scanned[i]))
      sizeList[i] = (m_EncodingBits + m_NumResidues);
  }

  return sizeList;
}

int MaskingPatternModule::countDontCare(std::vector<int> maskingVector)
{
  int numDontCare = 0;

  // count the number of don't care bits
  for (int i = 0; i < maskingVector.size(); i++)
  {
    if (maskingVector[i] == DONTCARE)
      numDontCare++;
  }
  return numDontCare;
}

bool MaskingPatternModule::is_rowPatternMatched(std::vector<uint8_t> &row)
{
  for (int i = 0; i < m_MaskingVector.size(); i++)
  {
    if (m_MaskingVector[i] == DONTCARE)
      continue;
    else if(m_MaskingVector[i] == row[i])
      continue;
    else
      return false;
  }
  return true;
}










}

