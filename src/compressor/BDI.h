#ifndef __BDI_H__
#define __BDI_H__

#include "Compressor.h"
#include "CompResult.h"

namespace comp
{

enum BDIState
{
  Zeros = 0,
  Repeat = 1,
  Base8Delta1 = 2,
  Base8Delta2 = 3,
  Base8Delta4 = 4,
  Base4Delta1 = 5,
  Base4Delta2 = 6,
  Base2Delta1 = 7,
  Uncompressed = 8
};

struct BDIResult : public CompResult
{
  /*** constructors ***/
  BDIResult()
    : CompResult(), Counts(9, 0) {};

  virtual void Update(unsigned uncompSize, unsigned compSize, int selected)
  {
    CompResult::Update(uncompSize, compSize);
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
        file << "workload,original_size,compressed_size,compression_ratio,";
        file << "zeros,";
        file << "repeat,";
        file << "b8d1,";
        file << "b8d2,";
        file << "b8d4,";
        file << "b4d1,";
        file << "b4d2,";
        file << "b2d1,";
        file << "uncomp,";
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
    // selection counts
    for (int i = 0; i < 9; i++)
      stream << fmt::format("{},", Counts[i]);
    stream << std::endl;

    if (file.is_open())
      file.close();
  }

  /*** member variables ***/
  std::vector<uint64_t> Counts;
};

class BDI : public Compressor
{
public:
  /*** constructor ***/
  BDI()
  {
    m_Stat = new BDIResult();
    m_CompName = "Base-Delta Immediate";
    m_Stat->CompressorName = m_CompName;
  }

  virtual unsigned CompressLine(std::vector<uint8_t> &dataLine);

private:
  bool isZeros(std::vector<uint8_t>& dataLine);
  bool isRepeated(std::vector<uint8_t>& dataLine, const unsigned granularity);
  unsigned checkBDI(std::vector<uint8_t>& dataLine, const unsigned baseSize, const unsigned deltaSize);
  uint64_t reduceSign(uint64_t x);

};

}

#endif  // __BDI_H__
