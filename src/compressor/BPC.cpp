#include <cstring>
#include "BPC.h"

namespace comp
{

static const unsigned ZRL_CODE_SIZE[34] = {0, 3, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
static const unsigned singleOneSize = 10;
static const unsigned consecutiveDoubleOneSize = 10;
static const unsigned allOneSize = 5;
static const unsigned zeroDBPSize = 5;
// 1        -> uncompressed
// 01       -> Z-RLE: 2~33
// 001      -> Z-RLE: 1
// 00000    -> single 1
// 00001    -> consecutive two 1s
// 00010    -> zero DBP
// 00011    -> All 1s

unsigned BPC::CompressLine(std::vector<uint8_t> &_dataLine)
{
  /*
    The original dataline is placed in row-wise order.
    And their DBPs are placed in col-wise order.
     ------------              -------------------
    |  data[0]   |            |   |   |   |   |   |
     ------------             | D | D | D | . | D |
    |  data[1]   |            | B | B | B | . | B |
     ------------      =>     | P | P | P | . | P |
    |    ...     |            | 0 | 1 | 2 | . | 3 |
     ------------             |   |   |   |   | 1 |
    |  data[31]  |            |   |   |   |   |   |
     ------------              -------------------
    */
  const unsigned lineSize = _dataLine.size();
  const unsigned uncompressedSize = BYTE * lineSize;

  // converts uint8_t to uint64_t
  std::vector<int64_t> dataLine(lineSize / 4, 0);
  for (int i = 0; i < lineSize; i += 4)
  {
    int64_t data;
    std::memcpy(&data, &_dataLine[i], 4);
    dataLine[i/4] = data;
  }
  
  // delta
  std::vector<int64_t> deltas;
  for (int row = 1; row < dataLine.size(); row++)
    deltas.push_back(dataLine[row] - dataLine[row - 1]);

  int32_t prevDBP;
  int32_t DBP[33];
  int32_t DBX[33];
  for (int col = 32; col >= 0; col--)
  {
    // a buffer of bit-plane
    int32_t buf = 0;
    for (int row = deltas.size() - 1; row >= 0; row--)
    {
      buf <<= 1;
      buf |= ((deltas[row] >> col) & 1);
    }

    // base-xor
    if (col == 32)
    {
      DBP[32] = buf;
      DBX[32] = buf;
      prevDBP = buf;
    }
    else
    {
      DBP[col] = buf;
      DBX[col] = buf ^ prevDBP;
      prevDBP = buf;
    }
  }

  // first 32-bit word in original form (dataLine)
  unsigned compressedSize = encodeFirst(dataLine[0]);
  // the rest of the data
  compressedSize += encodeDeltas(DBP, DBX);

  m_Stat->Update(uncompressedSize, compressedSize);
  return compressedSize;
}

unsigned BPC::encodeFirst(int64_t base)
{
  if (base = 0)
    return 3;
  else if (isSignExtended(base, 4))
    return 3 + 4;
  else if (isSignExtended(base, 8))
    return 3 + 8;
  else if (isSignExtended(base, 16))
    return 3 + 16;
  else
    return 1 + 32;
}

unsigned BPC::encodeDeltas(int32_t* DBP, int32_t* DBX)
{
  BPCResult* m_stat = static_cast<BPCResult*>(m_Stat);

  unsigned length = 0;
  unsigned runLength = 0;
  bool firstNZDBX = false;
  bool secondNZDBX = false;
  for (int i = 32; i >= 0; i--)
  {
    if (DBX[i] == 0)
    {
      runLength++;
    }
    else
    {
      // Z-RLE
      if (runLength > 0) 
      {
        length += ZRL_CODE_SIZE[runLength];
        m_stat->UpdatePattern(runLength, (int)BPCPattern::ZRLE);
      }
      runLength = 0;

      // zero DBP
      if (DBP[i] == 0)
      {
        length += zeroDBPSize;
        m_stat->UpdatePattern(1, (int)BPCPattern::Zero);
      }
      // All 1s
      else if (DBX[i] == 0x7fffffff)
      {
        length += allOneSize;
        m_stat->UpdatePattern(1, (int)BPCPattern::AllOnes);
      }
      else
      {
        // find where the 1s are
        int oneCnt = 0;
        for (int j = 0; j < 32; j++)
          if ((DBX[i] >> j) & 1)
            oneCnt++;
        unsigned twoDistance = 0;
        int firstPos = -1;
        if (oneCnt <= 2)
          for (int j = 0; j < 32; j++)
            if ((DBX[i] >> j) & 1)
              if (firstPos == -1)
                firstPos = j;
              else
                twoDistance = j - firstPos;

        // single 1
        if (oneCnt == 1)
        {
          length += singleOneSize;
          m_stat->UpdatePattern(1, (int)BPCPattern::SingleOne);
        }
        // consec double 1s
        else if ((oneCnt == 2) && (twoDistance == 1))
        {
          length += consecutiveDoubleOneSize;
          m_stat->UpdatePattern(1, (int)BPCPattern::ConsecTwoOnes);
        }
        // uncompressible
        else
        {
          length += 32;
          m_stat->UpdatePattern(1, (int)BPCPattern::Uncomp);
        }
      }
    }
  }
  // final Z-RLE
  if (runLength > 0)
  {
    length += ZRL_CODE_SIZE[runLength];
    m_stat->UpdatePattern(runLength, (int)BPCPattern::ZRLE);
  }

  return length;
}

bool BPC::isSignExtended(uint64_t value, uint8_t bitSize)
{
  uint64_t max = (1ULL << (bitSize - 1)) - 1;     // bitSize: 4 -> ...0000111
  uint64_t min = ~max;                            // bitSize: 4 -> ...1111000
  return (value <= max) | (value >= min);
}

bool BPC::isZeroExtended(uint64_t value, uint8_t bitSize)
{
  uint64_t max = (1ULL << (bitSize)) - 1;         // bitSize: 4 -> ...0001111
  return (value <= max);
}

}
