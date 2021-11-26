#include "BDI.h"

namespace comp
{

unsigned BDI::CompressLine(std::vector<uint8_t> &dataLine)
{
  const unsigned lineSize = dataLine.size();
  const unsigned uncompressedSize = BYTE * lineSize;

  BDIState select = BDIState::Uncompressed;

  unsigned bestCSize, currCSize;
  currCSize = uncompressedSize;
  bestCSize = currCSize;

  if(isZeros(dataLine))
  {
    bestCSize = BYTE;
    select = BDIState::Zeros;
  }
  else if(isRepeated(dataLine, 8))
  {
    bestCSize = BYTE * 8;
    select = BDIState::Repeat;
  }
  else
  {
    // base8-delta1
    // bestcase[32B / 64B] : (8+3)Bytes+4bits / (8+7)Bytes+8bits
    currCSize = checkBDI(dataLine, 8, 1);
    select = bestCSize > currCSize ? BDIState::Base8Delta1 : select;
    bestCSize = bestCSize > currCSize ? currCSize : bestCSize;

    // base8-delta2
    // bestcase[32B / 64B] : (8+6)Bytes+4bits / (8+14)Bytes+8bits
    currCSize = checkBDI(dataLine, 8, 2);
    select = bestCSize > currCSize ? BDIState::Base8Delta2 : select;
    bestCSize = bestCSize > currCSize ? currCSize : bestCSize;

    // base8-delta4
    // bestcase[32B / 64B] : (8+12)Bytes+4bits / (8+28)Bytes+8bits
    currCSize = checkBDI(dataLine, 8, 4);
    select = bestCSize > currCSize ? BDIState::Base8Delta4 : select;
    bestCSize = bestCSize > currCSize ? currCSize : bestCSize;

    // base4-delta1
    // bestcase[32B / 64B] : (4+7)Bytes+8bits / (4+15)Bytes+16bits
    currCSize = checkBDI(dataLine, 4, 1);
    select = bestCSize > currCSize ? BDIState::Base4Delta1 : select;
    bestCSize = bestCSize > currCSize ? currCSize : bestCSize;

    // base4-delta2
    // bestcase[32B / 64B] : (4+14)Bytes+8bits / (4+30)Bytes+16bits
    currCSize = checkBDI(dataLine, 4, 2);
    select = bestCSize > currCSize ? BDIState::Base4Delta2 : select;
    bestCSize = bestCSize > currCSize ? currCSize : bestCSize;

    // base2-delta1
    // bestcase[32B / 64B] : (2+15)Bytes+16bits / (2+31)Bytes+32bits
    currCSize = checkBDI(dataLine, 2, 1);
    select = bestCSize > currCSize ? BDIState::Base2Delta1 : select;
    bestCSize = bestCSize > currCSize ? currCSize : bestCSize;

    // incompressible
    select = (bestCSize == uncompressedSize) ? BDIState::Uncompressed : select;

  }

  // compressedSize + encodingBits
  int compressedSize = (bestCSize + 4);
  static_cast<BDIResult*>(m_Stat)->Update(uncompressedSize, compressedSize, (int)select);
  return compressedSize;
}

bool BDI::isZeros(std::vector<uint8_t> &dataLine)
{
  for(int i = 0; i < dataLine.size(); i++)
    if(dataLine[i] != 0)
      return false;
  return true;
}

bool BDI::isRepeated(std::vector<uint8_t> &dataLine, const unsigned granularity)
{
  // data concatenation
  std::vector<uint64_t> dataConcat;
  uint64_t temp;

  unsigned dataConcatSize = dataLine.size() / granularity;
  for(int i = 0; i < dataConcatSize; i++)
  {
    temp = 0;
    for(int j = 0; j < granularity; j++)
      temp = (temp << BYTE) | dataLine[i*granularity + j];

    dataConcat.push_back(temp);
  }

  // repeated block check
  uint64_t firstVal = dataConcat[0];
  for(int i = 1; i < dataConcatSize; i++)
    if(dataConcat[i] != firstVal)
      return false;
  return true;
}

unsigned BDI::checkBDI(std::vector<uint8_t> &dataLine,
    const unsigned baseSize, const unsigned deltaSize)
{
  const unsigned lineSize = dataLine.size();
  uint64_t deltaLimit = 0;

  // maximum size delta can have
  switch(deltaSize)
  {
  case 1:
    deltaLimit = BYTEMAX;
    break;
  case 2:
    deltaLimit = BYTE2MAX;
    break;
  case 4:
    deltaLimit = BYTE4MAX;
    break;
  }

  // little endian
  std::vector<uint8_t> littleEndian;
  for(int i = 0; i < lineSize; i += baseSize)
    for(int j = baseSize-1; j >= 0; j--)
      littleEndian.push_back(dataLine[i + j]);

  // define appropriate size for the immediate-mask
  unsigned maskSize = lineSize / baseSize;
  bool* mask = new bool[maskSize];
  for(int i = 0; i < maskSize; i++)
    mask[i] = false;

  // concatenate little endian'ed data in a new vector
  std::vector<uint64_t> dataConcat;
  uint64_t temp;
  for(int i = 0; i < maskSize; i++)
  {
    temp = 0;
    for(int j = 0; j < baseSize; j++)
      temp = (temp << BYTE) | littleEndian[i*baseSize + j];

    // sign extension
    if((temp & (1 << (BYTE*baseSize-1))) != 0)
      temp &= BYTE8MAX;

    dataConcat.push_back(temp);
  }

  // find immediate block
  unsigned immediateCount = 0;
  for(int i = 0; i < maskSize; i++)
  {
    if(reduceSign(dataConcat[i]) <= deltaLimit)
    {
      mask[i] = true;
      immediateCount++;
    }
  }

  // find non-zero base
  uint64_t base = 0;
  int baseIdx = 0;
  for(int i = 0; i < maskSize; i++)
  {
    if(!mask[i])
    {
      base = dataConcat[i];
      baseIdx = i;
      break;
    }
  }

  // base-delta computation
  bool notAllDelta = false;
  for(int i = baseIdx + 1; i < maskSize; i++)
  {
    if(!mask[i])
    {
      if(reduceSign(base - dataConcat[i]) > deltaLimit)
      {
        notAllDelta = true;
        break;
      }
    }
  }

  delete[] mask;

  // immediateMask + immediateDeltas + base + deltas
  if(notAllDelta)
    return maskSize + BYTE*((immediateCount*deltaSize) + ((maskSize-immediateCount)*baseSize));
  else
    return maskSize + BYTE*((immediateCount*deltaSize) + (baseSize + (maskSize - immediateCount - 1)*deltaSize));
}

uint64_t BDI::reduceSign(uint64_t x)
{
  uint64_t t = x >> 63;
  if(t)
  {
    for(int i = 62; i >= 0; i--)
    {
      t = (x >> i) & 0x01;
      if(t == 0)
      {
        return x & (BYTE8MAX >> (63 - (i + 1)));
      }
    }
  }
  return x;
}

}
