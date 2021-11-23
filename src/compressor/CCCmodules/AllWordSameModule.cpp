#include "AllWordSameModule.h"
#include "../Compressor.h"

namespace comp
{

unsigned AllWordSameModule::CompressLine(std::vector<uint8_t> &dataLine)
{
  // initialize base symbols of the dataLine
  uint8_t base[4];
  for (int i = 0; i < 4; i++)
    base[i] = dataLine[i];

  // check byteplane all same
  for (int i = 4; i < m_LineSize; i++)
  {
    if (dataLine[i] != base[i % 4])
      return m_LineSize * BYTE;
  }
  return BYTE * 4;
}

}
