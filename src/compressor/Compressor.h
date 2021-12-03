#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include <ios>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <fmt/core.h>
#include "../loader/Loader.h"
#include "../loader/LoaderGPGPU.h"
#include "CompResult.h"

namespace comp
{

class Compressor
{
public:
  /*** getters ***/
  std::string GetCompressorName()
  {
    return m_Stat->CompressorName;
  }

  /*** methods ***/
  virtual unsigned CompressLine(std::vector<uint8_t> &dataLine) = 0;
  virtual CompResult* GetResult() { return m_Stat; }

protected:
  CompResult *m_Stat; 
};

}

#endif  // __COMPRESSOR_H__
