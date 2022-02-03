#ifndef __FPC_H__
#define __FPC_H__

#include "Compressor.h"
#include "CompResult.h"

#define PREFIX_SIZE 3
#define NUM_FPC_PATTERN 8

namespace comp
{

enum class FPCState
{
  Prefix0 = 0,
  Prefix1 = 1,
  Prefix2 = 2,
  Prefix3 = 3,
  Prefix4 = 4,
  Prefix5 = 5,
  Prefix6 = 6,
  Prefix7 = 7
};

struct FPCResult : public CompResult
{
  /*** constructors ***/
  FPCResult(unsigned lineSize)
    : CompResult(lineSize), TotalWords(0), Counts(NUM_FPC_PATTERN, 0) {};

  virtual void Update(unsigned uncompSize, unsigned compSize, int selected)
  {
    CompResult::Update(uncompSize, compSize);

    TotalWords++;
    Counts[selected]++;
  }

  virtual void Print(std::string workloadName = "", std::string filePath = "")
  {
    // select file or stdout
    std::streambuf *buff;
    std::ofstream file;
    if (filePath == "")  // stdout
    {
      buff = std::cout.rdbuf();
    }
    else  // file
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
        file << "Workload,Original Size,Compressed Size,Compression Ratio,";
        file << "Total Words,";
        for (int i = 0; i < NUM_FPC_PATTERN; i++)
          file << fmt::format("Prefix{},", i);
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
    // word counts
    stream << fmt::format("{0},", TotalWords);
    // selection counts
    for (int i = 0; i < 8; i++)
      stream << fmt::format("{},", Counts[i]);
    stream << std::endl;

    if (file.is_open())
      file.close();
  }

  /*** member variables ***/
  std::vector<uint64_t> Counts;
  uint64_t TotalWords;
};

class FPC : public Compressor
{
public:
  FPC(unsigned lineSize)
  {
    m_Stat = new FPCResult(lineSize);
    m_Stat->CompressorName = "Frequent Pattern Compression";
  }

  virtual unsigned CompressLine(std::vector<uint8_t> &dataLine);

private:
  std::vector<uint32_t> concatenate(std::vector<uint8_t> &dataLine);

};

}

#endif  // __FPC_H__
