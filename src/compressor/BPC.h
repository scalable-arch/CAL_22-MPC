#ifndef __BPC_H__
#define __BPC_H__

#include "Compressor.h"
#include "CompResult.h"

#define NUM_BPC_PATTERN 7

namespace comp
{

enum class BPCPattern
{
  Uncomp          = 0,
  ZRLE            = 1,
  Zero            = 2,
  SingleOne       = 3,
  ConsecTwoOnes   = 4,
  ZeroDBP         = 5,
  AllOnes         = 6,
};

struct BPCResult : public CompResult
{
  /*** constructors ***/
  BPCResult(unsigned lineSize)
    : CompResult(lineSize), TotalWords(0), Counts(NUM_BPC_PATTERN, 0) {};

  void UpdatePattern(unsigned numWords, int selected)
  {
    TotalWords += numWords;
    Counts[selected]++;
  }

//  virtual void Update(unsigned uncompSize, unsigned compSize, int selected)
//  {
//    CompResult::Update(uncompSize, compSize);
//
//    TotalWords++;
//    Counts[selected]++;
//  }

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
        for (int i = 0; i < NUM_BPC_PATTERN; i++)
          file << fmt::format("Pattern{},", i);
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
    for (int i = 0; i < NUM_BPC_PATTERN; i++)
      stream << fmt::format("{},", Counts[i]);
    stream << std::endl;

    if (file.is_open())
      file.close();
  }

  /*** member variables ***/
  std::vector<uint64_t> Counts;
  uint64_t TotalWords;
};

class BPC : public Compressor
{
public:
  /*** constructor ***/
  BPC(unsigned lineSize)
  {
    m_Stat = new CompResult(lineSize);
    m_Stat->CompressorName = "Bit-Plane Compression";
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
