#ifndef __BPC_H__
#define __BPC_H__

#include "Compressor.h"
#include "CompResult.h"

namespace comp
{

class BPC : public Compressor
{
public:
  /*** constructor ***/
  BPC()
  {
    m_Stat = new CompResult();
    m_CompName = "Bit-Plane Compression";
    m_Stat->CompressorName = m_CompName;
  }

  virtual unsigned CompressLine(std::vector<uint8_t> &dataLine);

private:
  unsigned encodeFirst(int64_t base);
  unsigned encodeDeltas(int32_t* DBP, int32_t* DBX);
  bool isSignExtended(uint64_t value, uint8_t bitSize);
  bool isZeroExtended(uint64_t value, uint8_t bitSize);
};

}



#endif  // __BPC_H__