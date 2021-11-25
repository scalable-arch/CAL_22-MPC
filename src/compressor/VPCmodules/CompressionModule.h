#pragma once

#include <iostream>
#include <vector>
#include "CompStruct.h"

namespace comp
{

class Compressor;

class CompressionModule
{
friend class Compressor;

public:
  // constuctor
  CompressionModule(int lineSize)
    : m_LineSize(lineSize)
  {
    // error statement
    if (m_LineSize <= 0)
    {
      printf("Given linesize is not valid. Out of bounds.\n");
      exit(1);
    }
  }

  virtual unsigned CompressLine(std::vector<uint8_t> &dataLine) = 0;
  virtual Binary CompressLine(std::vector<uint8_t> &dataLine, int nothing) = 0;

protected:
  int m_LineSize;             // number of symbols
};

}
