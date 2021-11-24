#ifndef __COMPRESULT_H__
#define __COMPRESULT_H__

#include <iostream>
#include <fstream>
#include <string>

#include <fmt/core.h>

#include "../utils.h"

namespace comp
{

struct CompResult
{
  CompResult()
    : OriginalSize(0), CompressedSize(0), CompRatio(0) {};

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
      // write column index description
      if (!isFileExists(filePath))
      {
        file.open(filePath);
        if (!file.is_open())
        {
          std::cout << fmt::format("File is not open: \"{}\"", filePath) << std::endl;
          exit(1);
        }
        // first line
        file << "workload,original_size,compressed_size,compression_ratio,";
        file << std::endl;
        file.close();
      }
      file.open(filePath, std::ios_base::app);
      buff = file.rdbuf();
    }
    std::ostream stream(buff);

    // print result
    // workloadname, originalsize, compressedsize, compratio
    stream << fmt::format("{0},{1},{2},{3},", workloadName, OriginalSize, CompressedSize, CompRatio);
    stream << std::endl;

    if (file.is_open())
      file.close();
  }

  virtual void PrintDetail(std::string workloadName = "", std::string filePath = "") {}

  /*** member varibles ***/
  std::string CompressorName;

  uint64_t OriginalSize;
  uint64_t CompressedSize;
  double CompRatio;

};

}

#endif  // __COMPRESULTS_H__

