#pragma once

#include <ios>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <fmt/core.h>
#include "../loader/Loader.h"
#include "../loader/LoaderGPGPU.h"

#define BYTE 8
#define BYTEMAX  0xff
#define BYTE2MAX 0xffff
#define BYTE4MAX 0xffffffff
#define BYTE8MAX 0xffffffffffffffff

#define COMPSIZELIMIT 280

namespace comp
{

struct CompResult
{
  CompResult()
    : OriginalSize(0), CompressedSize(0), CompRatio(0) {};

  std::string CompressorName;

  uint64_t OriginalSize;
  uint64_t CompressedSize;
  double CompRatio;

  virtual void Update(unsigned uncompSize, unsigned compSize, int selected = 0)
  {
    OriginalSize += uncompSize;
    CompressedSize += compSize;
    CompRatio = (double)OriginalSize / (double)CompressedSize;
  }

  virtual void Print(std::string workloadName = "", std::string filePath = "")
  {
    // select file or stdout
    std::streambuf *buff;
    std::ofstream file;
    if (filePath == "")
    {
      buff = std::cout.rdbuf();
    }
    else
    {
      file.open(filePath, std::ios_base::app);
      if (!file.is_open())
      {
        std::cout << fmt::format("File is not open: \"{}\"", filePath) << std::endl;
        exit(1);
      }
      buff = file.rdbuf();
    }
    std::ostream stream(buff);

    // workloadname, originalsize, compressedsize, compratio
    stream << fmt::format("{0},{1},{2},{3},", workloadName, OriginalSize, CompressedSize, CompRatio) << std::endl;

    if (file.is_open())
      file.close();
  }
};

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

protected:
  std::string m_CompName;
};

}
