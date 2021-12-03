#include "FPC.h"
#include "Compressor.h"

namespace comp
{

unsigned FPC::CompressLine(std::vector<uint8_t> &dataLine)
{
//  const unsigned lineSize = dataLine.size();

  std::vector<uint32_t> dataConcat = concatenate(dataLine);
  const unsigned concatSize = dataConcat.size();

  unsigned currCSize = 0;
  unsigned i = 0;
  while(i < concatSize)
  {
    uint32_t val = dataConcat[i];

    // prefix 000 : zero value runs
    if(val == 0x00000000)
    {
      currCSize += 3 + PREFIX_SIZE;
      i++;
      static_cast<FPCResult*>(m_Stat)->Update(4*BYTE, 3 + PREFIX_SIZE, (int)FPCState::Prefix0);
      while(dataConcat[i] == 0x00000000)
      {
        i++;
        static_cast<FPCResult*>(m_Stat)->Update(4*BYTE, 0, (int)FPCState::Prefix0);
      }
      continue;
    }
    // prefix 001 : 4-bit sign extended
    else if(((val & 0xFFFFFFF8) == 0x00000000)
        ||  ((val & 0xFFFFFFF8) == 0xFFFFFFF8))
    {
      currCSize += 4 + PREFIX_SIZE;
      static_cast<FPCResult*>(m_Stat)->Update(4*BYTE, 4 + PREFIX_SIZE, (int)FPCState::Prefix1);
    }
    // prefix 010 : 8-bit sign extended
    else if(((val & 0xFFFFFF80) == 0x00000000)
        ||  ((val & 0xFFFFFF80) == 0xFFFFFF80))
    {
      currCSize += 8 + PREFIX_SIZE;
      static_cast<FPCResult*>(m_Stat)->Update(4*BYTE, 8 + PREFIX_SIZE, (int)FPCState::Prefix2);
    }
    // prefix 011 : 16-bit sign extended
    else if(((val & 0xFFFF8000) == 0x00000000)
        ||  ((val & 0xFFFF8000) == 0xFFFF8000))
    {
      currCSize += 16 + PREFIX_SIZE;
      static_cast<FPCResult*>(m_Stat)->Update(4*BYTE, 16 + PREFIX_SIZE, (int)FPCState::Prefix3);
    }
    // prefix 100 : 16-bit padded with a zero
    else if((val & 0x0000FFFF) == 0x00000000)
    {
      currCSize += 16 + PREFIX_SIZE;
      static_cast<FPCResult*>(m_Stat)->Update(4*BYTE, 16 + PREFIX_SIZE, (int)FPCState::Prefix4);
    }
    // prefix 101 : two halfwords, each a byte sign-extended
    else if(((val & 0xFF80FF80) == 0x00000000)
        ||  ((val & 0xFF80FF80) == 0xFF800000)
        ||  ((val & 0xFF80FF80) == 0x0000FF80)
        ||  ((val & 0xFF80FF80) == 0xFF80FF80))
    {
      currCSize += 16 + PREFIX_SIZE;
      static_cast<FPCResult*>(m_Stat)->Update(4*BYTE, 16 + PREFIX_SIZE, (int)FPCState::Prefix5);
    }
    // prefix 110 : word consisting fo repeated bytes
    else if(((val & 0xFF) == ((val >> BYTE) & 0xFF))
        &&  ((val & 0xFF) == ((val >> 2*BYTE) & 0xFF))
        &&  ((val & 0xFF) == ((val >> 3*BYTE) & 0xFF)))
    {
      currCSize += 8 + PREFIX_SIZE;
      static_cast<FPCResult*>(m_Stat)->Update(4*BYTE, 8 + PREFIX_SIZE, (int)FPCState::Prefix6);
    }
    else
    {
      currCSize += 4*BYTE + PREFIX_SIZE;
      static_cast<FPCResult*>(m_Stat)->Update(4*BYTE, 4*BYTE + PREFIX_SIZE, (int)FPCState::Prefix7);
    }
    i++;
  }

  unsigned compressedSize = currCSize;
  return compressedSize;
}

std::vector<uint32_t> FPC::concatenate(std::vector<uint8_t> &dataLine)
{
  const unsigned granularity = 4;
  std::vector<uint32_t> dataConcat;
  uint32_t temp;

  unsigned dataConcatSize = dataLine.size() / granularity;
  for(unsigned i = 0; i < dataConcatSize; i++)
  {
    temp = 0;
    // little endian
    for(int j = granularity-1; j >= 0; j--)
      temp = (temp << BYTE) | dataLine[i*granularity + j];

    dataConcat.push_back(temp);
  }

  return dataConcat;
}

}


