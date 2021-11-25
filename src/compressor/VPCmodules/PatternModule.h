#pragma once

#include <cmath>
#include <vector>

#include "PredCompModule.h"
#include "CompStruct.h"

#define ZERO     0
#define ONE      1
#define DONTCARE 2

namespace comp
{

/*** Base class ***/
class PatternModule
{
friend class FPCModule;

public:
  // constructor
  PatternModule(int encodingBits)
    : m_EncodingBits(encodingBits) {}

  virtual compSizeList& Compress(Binary &scanned, compSizeList &sizeList) = 0;

protected:
  int m_EncodingBits;
};

/*** Inherited classes ***/
// Uncompressed pattern check module
class UncompressedPatternModule : public PatternModule
{
public:
  // constructors
  UncompressedPatternModule(int encodingBits)
    : PatternModule(encodingBits) {}

  compSizeList& Compress(Binary &scanned, compSizeList &sizeList);
};

// ZRLE & Zero pattern check module
class ZerosPatternModule : public PatternModule
{
public:
  // constructor
  ZerosPatternModule(int encodingBitsZRLE, int encodingBitsZero)
    : PatternModule(encodingBitsZRLE), m_EncodingBitsZero(encodingBitsZero) {}

  compSizeList& Compress(Binary &scanned, compSizeList &sizeList);

private:
  bool is_rowAllZeros(std::vector<uint8_t> &row);

private:
  int m_EncodingBitsZero;
};

// Single One pattern check module
class SingleOnePatternModule : public PatternModule
{
public:
  // constructor
  SingleOnePatternModule(int encodingBits)
    : PatternModule(encodingBits) {}

  compSizeList& Compress(Binary &scanned, compSizeList &sizeList);

private:
  bool is_rowSingleOne(std::vector<uint8_t> &row);
};

// Two Consecutive Ones pattern check module
class TwoConsecutiveOnesPatternModule : public PatternModule
{
public:
  // constructors
  TwoConsecutiveOnesPatternModule(int encodingBits)
    : PatternModule(encodingBits) {}

  compSizeList& Compress(Binary &scanned, compSizeList &sizeList);

private:
  bool is_rowTwoConsecutiveOnes(std::vector<uint8_t> &row);
};

// Zeros Front pattern check module
class ZerosFrontPatternModule : public PatternModule
{
public:
  // constructor
  ZerosFrontPatternModule(int encodingBits, int numZeros)
    : PatternModule(encodingBits), m_NumZeros(numZeros) {}

  compSizeList& Compress(Binary &scanned, compSizeList &sizeList);

private:
  bool is_rowZerosFront(std::vector<uint8_t> &row);

private:
  int m_NumZeros;
};

// Zeros Front Half pattern check module
class ZerosFrontHalfPatternModule : public PatternModule
{
public:
  // constructor
  ZerosFrontHalfPatternModule(int encodingBits)
    : PatternModule(encodingBits) {}

  compSizeList& Compress(Binary &scanned, compSizeList &sizeList);

private:
  bool is_frontHalfZeros(std::vector<uint8_t> &row);
};

// Zeros Back pattern check module
class ZerosBackPatternModule : public PatternModule
{
public:
  // constructor
  ZerosBackPatternModule(int encodingBits, int numZeros)
    : PatternModule(encodingBits), m_NumZeros(numZeros) {}

  compSizeList& Compress(Binary &scanned, compSizeList &sizeList);

private:
  bool is_rowZerosBack(std::vector<uint8_t> &row);

private:
  int m_NumZeros;
};

// Zeros Back Half pattern check module
class ZerosBackHalfPatternModule : public PatternModule
{
public:
  // constructor
  ZerosBackHalfPatternModule(int encodingBits)
    : PatternModule(encodingBits) {}

  compSizeList& Compress(Binary &scanned, compSizeList &sizeList);

private:
  bool is_backHalfZeros(std::vector<uint8_t> &row);
};

// Masking pattern check module
class MaskingPatternModule : public PatternModule
{
public:
  // constructor
  MaskingPatternModule(int encodingBits, std::vector<int> maskingVector)
    : PatternModule(encodingBits), m_MaskingVector(maskingVector)
  {
    // maskingVecor validity check
    for (int i = 0; i < maskingVector.size(); i++)
    {
      if (maskingVector[i] != ZERO && maskingVector[i] != ONE && maskingVector[i] != DONTCARE)
      {
        printf("Wrong number is in the maskingVector!\n");
        exit(1);
      }
    }

    m_NumResidues = countDontCare(maskingVector);
  }

  compSizeList& Compress(Binary &scanned, compSizeList &sizeList);

private:
  int countDontCare(std::vector<int> maskingVector);

  bool is_rowPatternMatched(std::vector<uint8_t> &row);

private:
  std::vector<int> m_MaskingVector;

  int m_NumResidues;
};

/*** if you need more pattern modules, implement here. ***/

}
