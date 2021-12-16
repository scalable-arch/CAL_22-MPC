#ifndef __BDI_H__
#define __BDI_H__

#include "Compressor.h"
#include "CompResult.h"

namespace comp
{

enum class BDIState
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
  BDIResult(unsigned lineSize)
    : CompResult(lineSize), Counts(9, 0) {};

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
        file << "Workload,Original Size,Compressed Size,Compression Ratio,";
        file << "Zeros,";
        file << "Repeated,";
        file << "B8D1,";
        file << "B8D2,";
        file << "B8D4,";
        file << "B4D1,";
        file << "B4D2,";
        file << "B2D1,";
        file << "Uncompressed,";
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
  BDI(unsigned lineSize)
  {
    m_Stat = new BDIResult(lineSize);
    m_Stat->CompressorName = "Base-Delta Immediate";
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
