#ifndef __CPACK_H__
#define __CPACK_H__

#include <deque>

#include "Compressor.h"
#include "CompResult.h"

#define WORDSIZE 4
#define DICTSIZE 64
#define NUM_ENTRY DICTSIZE/WORDSIZE

#define NUM_CPACK_PATTERN 6

namespace comp
{

enum class CPACKPattern
{
  ZZZZ = 0,
  ZZZX = 1,
  MMMM = 2,
  MMMX = 3,
  MMXX = 4,
  XXXX = 5,
};

struct CPACKResult : public CompResult
{
  /*** constructors ***/
  CPACKResult(unsigned lineSize)
    : CompResult(lineSize), TotalWords(0), Counts(NUM_CPACK_PATTERN, 0) {};

//  virtual void Update(unsigned uncompSize, unsigned compSize)
//  {
//    CompResult::Update(uncompSize, compSize);
//  }

  void UpdatePattern(int selected)
  {
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
        for (int i = 0; i < NUM_CPACK_PATTERN; i++)
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
    for (int i = 0; i < NUM_CPACK_PATTERN; i++)
      stream << fmt::format("{},", Counts[i]);
    stream << std::endl;

    if (file.is_open())
      file.close();
  }

  /*** member variables ***/
  std::vector<uint64_t> Counts;
  uint64_t TotalWords;
};

class CPACK : public Compressor
{
public:
  CPACK(unsigned lineSize)
  {
    m_Stat = new CPACKResult(lineSize);
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
  const unsigned m_PatternLength[NUM_CPACK_PATTERN] = { 2, 34, 6, 24, 12, 16 };
};

}

#endif  // __CPACK_H__
