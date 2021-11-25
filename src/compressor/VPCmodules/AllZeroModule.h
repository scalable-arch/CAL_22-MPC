#pragma once

#include "CompressionModule.h"
#include <bits/stdint-uintn.h>

namespace comp
{

class AllZeroModule : public CompressionModule
{
public:
  // constructor
  AllZeroModule(int lineSize)
    : CompressionModule(lineSize) {}

  unsigned CompressLine(std::vector<uint8_t> &dataLine);
  Binary CompressLine(std::vector<uint8_t> &dataLine, int nothing) { std::cout << "Not implemented." << std::endl; exit(1); }

};

}
