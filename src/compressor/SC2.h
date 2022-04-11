#ifndef __SC2_H__
#define __SC2_H__

#include <iostream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <map>
#include <set>
#include <sstream>
#include <cassert>

#include "CompResult.h"
#include "Compressor.h"

#define SC2_ENTRIES 1000
#define WARM_UP_CNT 10000000

namespace comp
{
namespace huffman
{

struct Node
{
public:
  int64_t symbol;
  uint64_t freq;
  Node *left, *right;
};

Node *createNode(int64_t symbol, uint64_t freq, Node *left = nullptr, Node *right = nullptr);

class MinHeap
{
public:
  // constructor
  MinHeap(std::map<int64_t, uint64_t> symbolMap);

  // methods
  int GetLeftChild(int i);
  int GetRightChild(int i);
  int GetParent(int i);
  Node **GetRoot() { return heapArr; }

  Node *ExtractMin();
  int AddNode(int64_t symbol, uint64_t freq, Node *left = nullptr, Node *right = nullptr);

  int PrintHeap();

  int size() { return heapSize; }

private:
  int buildHeap();
  int minHeapify(int index);
  
  int swapHeapNodes(int i, int j);

protected:
  int heapSize;
  Node **heapArr;

};

MinHeap BuildHuffmanTree(MinHeap minHeap);
int GetHuffmanCode(Node *huffmanNode, std::map<int64_t, std::string> &encodedSymbols, std::string code = "");

bool comparator(std::pair<int64_t, std::string> left, std::pair<int64_t, std::string> right);

// encode
int GetBitSize(int n);
std::string GetBinaryString(int n, int bitSize = -1);
std::map<int64_t, std::string> GetCanonicalCode(std::map<int64_t, std::string> huffmanCode);
std::vector<int> GetBitLengthCodes(std::map<int64_t, std::string> canonicalCode);

// decode
//std::string GetSymbolsForBitLength(std::vector<int> bitCodes, int bitLength);
//std::map<std::string, int> GetCanonicalCodebook(int *bitCodes);


//std::string GetEncodedText(std::string text, std::map<int, std::string> canonicalCodes);
//std::string GetDecodedText(std::string encodedText, std::map<std::string, int> canonicalCodes);

std::map<int64_t, uint64_t> *GetFreqMap(std::vector<int64_t> &dataBlock, std::map<int64_t, uint64_t> *freqMap = nullptr);

bool cmp(const std::pair<int64_t, uint64_t> &lhs, const std::pair<int64_t, uint64_t> &rhs);


}

class SC2 : public Compressor
{
public:
  /*** constructor ***/
  SC2(unsigned lineSize, unsigned warmupCnt = 100000)
    : m_samplingCnt(0), m_maxSamplingCnt(warmupCnt)
  {
    m_Stat = new CompResult(lineSize);
    m_Stat->CompressorName = "SC2-Huffman";

    mp_symFreqMap = new std::map<int64_t, uint64_t>;
  }

  virtual unsigned CompressLine(std::vector<uint8_t> &dataLine);
  void SetSamplingCnt(unsigned cnt);

private:
  unsigned m_samplingCnt;
  unsigned m_maxSamplingCnt;

  std::map<int64_t, uint64_t> *mp_symFreqMap;
  std::map<int64_t, std::string> m_huffmanCodes;
//  huffman::MinHeap *m_minHeap;
};


}


#endif  // __SC2_H__

