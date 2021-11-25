#pragma once

#include "CompressionModule.h"

namespace comp
{

class AllWordSameModule : public CompressionModule
{
public:
  // constructor
  AllWordSameModule(int lineSize)
    : CompressionModule(lineSize) {}

  unsigned CompressLine(std::vector<uint8_t> &dataLine);
  Binary CompressLine(std::vector<uint8_t> &dataLine, int nothing) { std::cout << "Not implemented." << std::endl; exit(1); }
};

}
