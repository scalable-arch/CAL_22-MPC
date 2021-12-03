#ifndef __CPACK_H__
#define __CPACK_H__

#include <deque>

#include "Compressor.h"
#include "CompResult.h"

#define WORDSIZE 4
#define DICTSIZE 64
#define NUM_ENTRY DICTSIZE/WORDSIZE

#define NUM_PATTERN 6

namespace comp
{

class CPACK : public Compressor
{
public:
  CPACK(unsigned lineSize)
  {
    m_Stat = new CompResult(lineSize);
    m_Stat->CompressorName = "C-Pack";

    // init dictionary
    for (int i = 0; i < NUM_ENTRY; i++)
    {
      uint8_t *init = new uint8_t[WORDSIZE];
      for (int j = 0; j < WORDSIZE; j++)
        init[j] = 0;
      m_Dictionary.push_back(init);
    }
  }

  virtual unsigned CompressLine(std::vector<uint8_t> &dataLine); 

private:
  std::deque<uint8_t*> m_Dictionary;

  // 0. zzzz (00)         : 2
  // 1. xxxx (01)BBBB     : 34
  // 2. mmmm (10)bbbb     : 6
  // 3. mmxx (1100)bbbbBB : 24
  // 4. zzzx (1100)B      : 12
  // 5. mmmx (1110)bbbbB  : 16
  const unsigned m_PatternLength[NUM_PATTERN] = { 2, 34, 6, 24, 12, 16 };
};

}

#endif  // __CPACK_H__
