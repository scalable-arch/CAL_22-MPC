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

#define BYTE 8
#define BYTEMAX  0xff
#define BYTE2MAX 0xffff
#define BYTE4MAX 0xffffffff
#define BYTE8MAX 0xffffffffffffffff

#define COMPSIZELIMIT 280

namespace comp
{

class Compressor
{
public:
  /*** getters ***/
  std::string GetCompressorName()
  {
    return m_CompName;
  }

  /*** methods ***/
  virtual unsigned CompressLine(std::vector<uint8_t> &dataLine) = 0;
  virtual CompResult* GetResult() { return m_Stat; }

protected:
  std::string m_CompName;
  CompResult *m_Stat; 
};

}

#endif  // __COMPRESSOR_H__
