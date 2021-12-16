#ifndef __PATTERN_H__
#define __PATTERN_H__

#include <utility>
#include <math.h>

#include <fmt/core.h>

#include "Compressor.h"
#include "CompResult.h"
#include "LRU.h"

namespace comp
{

enum class PatternState
{
  Base8Delta1 = 0,
  Base8Delta2 = 1,
  Base8Delta4 = 2,
  Base4Delta1 = 3,
  Base4Delta2 = 4,
  Base2Delta1 = 5,
  Zeros = 6,
  Repeat = 7,
  TemporalLocality = 8,
  NotDefined = 9,
};

struct PatternResult : public CompResult
{
  /*** constructors ***/
  PatternResult(unsigned lineSize)
    : CompResult(lineSize), Total(0), Z(0), R(0), T(0), U(0),
      ImplicitCounts(6, 0), ExplicitCounts(6, 0) {};

  // count bytes
  void UpdateStat(int selected, unsigned baseSize=1, bool isImplicit=false)
  {
    if (selected == (int)PatternState::Zeros)
    {
      Z += baseSize;
    }
    else if (selected == (int)PatternState::Repeat)
    {
      R += baseSize;
    }
    else if (selected == (int)PatternState::TemporalLocality)
    {
      T += baseSize;
    }
    else if (selected == (int)PatternState::NotDefined)
    {
      U += baseSize;
    }
    else
    {
      if (isImplicit)
        ImplicitCounts[selected] += baseSize;
      else
        ExplicitCounts[selected] += baseSize;
    }
  }

  void UpdateTotal()
  {
    Total += LineSize;
  }

  void UpdateCountMap(std::vector<uint8_t> &dataLine)
  {
    auto it = dataLine.begin();
    for (; it != dataLine.end(); it++)
    {
      uint8_t &symbol = *it;
      auto countIter = SymbolCounts.find(symbol);

      if (countIter != SymbolCounts.end())
        SymbolCounts[symbol]++;
      else
        SymbolCounts.insert(std::make_pair(symbol, 1));
    }
  }

  double ComputeEntropy()
  {
    uint64_t sum = 0;
    for (auto it = SymbolCounts.begin(); it != SymbolCounts.end(); it++)
    {
      uint64_t &counts = it->second;
      sum += counts;
    }

    std::map<uint8_t, double> SymbolProbs;
    for (auto it = SymbolCounts.begin(); it != SymbolCounts.end(); it++)
    {
      const uint8_t &symbol = it->first;
      uint64_t &counts = it->second;
      double probability = (double)counts / (double)sum;

      SymbolProbs.insert(std::make_pair(symbol, probability));
    }

    // entropy = sum(-p * log2(p))
    double entropy = 0;
    for (auto it = SymbolProbs.begin(); it != SymbolProbs.end(); it++)
    {
      const uint8_t &symbol = it->first;
      double &probability = it->second;

      entropy += -probability * log2(probability);
    }

    return entropy;
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
        file << "workload,";
        file << "zeros,";
        file << "repeat,";
        file << "templocality,";
        file << "b8d1-implicit,b8d1-explicit,";
        file << "b8d2-implicit,b8d2-explicit,";
        file << "b8d4-implicit,b8d4-explicit,";
        file << "b4d1-implicit,b4d1-explicit,";
        file << "b4d2-implicit,b4d2-explicit,";
        file << "b2d1-implicit,b2d1-explicit,";
        file << "uncomp,";
        file << "total,";
        file << "entropy,";
        file << std::endl;
        file.close();
      }
      file.open(filePath, std::ios_base::app);
      buff = file.rdbuf();
    }
    std::ostream stream(buff);

    double entropy = ComputeEntropy();
    // print result
    // workloadname, originalsize, compressedsize, compratio
    stream << fmt::format("{},", workloadName);
    // selection counts
    stream << fmt::format("{},{},{},", Z, R, T);
    for (int i = 0; i < 6; i++)
      stream << fmt::format("{},{},", ImplicitCounts[i], ExplicitCounts[i]);
    stream << fmt::format("{},", U);
    stream << fmt::format("{},", Total);
    stream << fmt::format("{},", entropy);
    stream << std::endl;

    if (file.is_open())
      file.close();
  }

  /*** member variables ***/
  std::vector<uint64_t> ImplicitCounts;
  std::vector<uint64_t> ExplicitCounts;
  std::map<uint8_t, uint64_t> SymbolCounts;
  uint64_t Z, R, T, U;   // Zeros, Repeated, TemporalLocality, NotDefined
  uint64_t Total;
};

class Pattern : public Compressor
{
public:
  /*** constructor ***/
  Pattern(unsigned lineSize)
    : m_DataCache(CACHESIZE)
  {
    m_Stat = new PatternResult(lineSize);
    m_Stat->CompressorName = "Pattern Checker";
  }

  virtual unsigned CompressLine(std::vector<uint8_t> &dataLine);

private:
  bool isZeros(std::vector<uint8_t>& dataLine);
  bool isRepeated(std::vector<uint8_t>& dataLine, const unsigned granularity);
  bool isExistedBefore(std::vector<uint8_t>& dataLine);
  unsigned checkPattern(std::vector<uint8_t>& dataLine, const unsigned baseSize, const unsigned deltaSize);
  void countPattern(std::vector<uint8_t>& dataLine, PatternState pattern);
  uint64_t reduceSign(uint64_t x);

public:
  LRUCache<std::vector<uint8_t>, int, std::vector<uint8_t>> m_DataCache;

};

}

#endif  // __PATTERN_H__
