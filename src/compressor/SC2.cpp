#include "SC2.h"
#include <iterator>
#include <sstream>
#include <utility>

#define HEAP_CAPACITY 1000
#define WARM_UP_CNT 1000000
#define WORD_GRAN 4

namespace comp
{
namespace huffman
{

Node *createNode(int64_t symbol, uint64_t freq, Node *left, Node *right)
{
  Node *node = new Node;
  node->symbol = symbol;
  node->freq = freq;
  node->left = left;
  node->right = right;
  return node;
}

MinHeap::MinHeap(std::map<int64_t, uint64_t> symbolMap)
{
  this->heapSize = symbolMap.size();
  assert (heapSize <= HEAP_CAPACITY);

  this->heapArr = new Node * [HEAP_CAPACITY];

  for (auto it = symbolMap.begin(); it != symbolMap.end(); it++)
  {
    int index = std::distance(symbolMap.begin(), it);
    this->heapArr[index] = createNode(it->first, it->second);
  }

  this->buildHeap();
}

int MinHeap::GetLeftChild(int i)
{
  return 2 * i + 1;
}

int MinHeap::GetRightChild(int i)
{
  return 2 * i + 2;
}

int MinHeap::GetParent(int i)
{
  return ceil((float) i / 2) - 1;
}

int MinHeap::buildHeap()
{
  int lastParentIndex = this->heapSize / 2 - 1;

  for (int i = lastParentIndex; i >= 0; i--)
  {
    this->minHeapify(i);
  }

  return 0;
}

int MinHeap::minHeapify(int index)
{
  int minIndex = index;
  int leftChild = this->GetLeftChild(index);
  int rightChild = this->GetRightChild(index);

  if (leftChild <= this->heapSize - 1 && this->heapArr[leftChild]->freq < this->heapArr[minIndex]->freq)
  {
    minIndex = leftChild;
  }

  if (rightChild <= this->heapSize - 1 && this->heapArr[rightChild]->freq < this->heapArr[minIndex]->freq)
  {
    minIndex = rightChild;
  }

  if (minIndex != index)
  {
    this->swapHeapNodes(index, minIndex);
    this->minHeapify(minIndex);
  }

  return 0;
}

Node* MinHeap::ExtractMin()
{
  Node *minNode = this->heapArr[0];
  this->swapHeapNodes(0, this->heapSize - 1);
  this->heapSize--;
  this->minHeapify(0);

  return minNode;
}

int MinHeap::AddNode(int64_t symbol, uint64_t freq, Node *left, Node *right)
{
  assert (this->heapSize <= HEAP_CAPACITY);

  this->heapArr[this->heapSize++] = createNode(symbol, freq, left, right);

  for (int i = this->heapSize - 1; i > 0 && this->heapArr[this->GetParent(i)]->freq > this->heapArr[i]->freq;)
  {
    int j = this->GetParent(i);
    this->swapHeapNodes(i, j);
    i = j;
  }

  return 0;
}

int MinHeap::swapHeapNodes(int i, int j)
{
  Node *temp = this->heapArr[i];
  this->heapArr[i] = this->heapArr[j];
  this->heapArr[j] = temp;

  return 0;
}

int MinHeap::PrintHeap()
{
  for (int i = 0; i < this->heapSize; i++)
  {
    std::cout << this->heapArr[i]->symbol << " - " << this->heapArr[i]->freq << std::endl;
  }

  return 0;

}

MinHeap BuildHuffmanTree(MinHeap minHeap)
{
  while (minHeap.size() > 1)
  {
    Node *leftNode = minHeap.ExtractMin();
    Node *rightNode = minHeap.ExtractMin();
    minHeap.AddNode(-1, leftNode->freq + rightNode->freq, leftNode, rightNode);
  }

  return minHeap;
}

int GetHuffmanCode(Node *huffmanNode, std::map<int64_t, std::string> &encodedSymbols, std::string code)
{
  if (!huffmanNode->left && !huffmanNode->right)
  {
    encodedSymbols[huffmanNode->symbol] = code;
    return 0;
  }

  GetHuffmanCode(huffmanNode->left, encodedSymbols, code + "0");
  GetHuffmanCode(huffmanNode->right, encodedSymbols, code + "1");

  return 0;
}

bool comparator(std::pair<int64_t, std::string> left, std::pair<int64_t, std::string> right)
{
  return left.second.size() == right.second.size() ? left.first < right.first : left.second.size() < right.second.size();
}

int GetBitSize(int n)
{
  int bits = 1;
  while (pow(2, bits) <= n)
  {
    bits++;
  }

  return bits;
}

std::string GetBinaryString(int n, int bitSize)
{
  std::stringstream stream;
  std::string reverseBinary, binaryStr;

  do {
    stream << (char) n % 2;
    n /= 2;
  } while(n);

  if (bitSize != -1 && stream.str().size() < bitSize)
  {
    int paddingSize = bitSize - stream.str().size();
    while (paddingSize--)
    {
      stream << '0';
    }
  }

  reverseBinary = stream.str();
  binaryStr.assign(reverseBinary.rbegin(), reverseBinary.rend());

  return binaryStr;
}

std::map<int64_t, std::string> GetCanonicalCode(std::map<int64_t, std::string> huffmanCode)
{
  std::set<std::pair<int64_t, std::string>, bool(*)(std::pair<int64_t, std::string>, std::pair<int64_t, std::string>)> orderedHuffman(huffmanCode.begin(), huffmanCode.end(), &comparator);
  int currentVal, prevBitLength;
  currentVal = 0;
  prevBitLength = (int)orderedHuffman.begin()->second.size();

  std::map<int64_t, std::string> canonicalCode;

  for (std::pair<int64_t, std::string> current: orderedHuffman)
  {
    int shiftBits = current.second.size() - prevBitLength;
    currentVal = currentVal << shiftBits;
    canonicalCode[current.first] = GetBinaryString(currentVal, current.second.size());
    ++currentVal;
    prevBitLength = current.second.size();
  }

  return canonicalCode;
}

std::vector<int> GetBitLengthCodes(std::map<int64_t, std::string> canonicalCode)
{
  std::vector<int> bitCodes;

  for (auto it = canonicalCode.begin(); it != canonicalCode.end(); it++)
  {
    bitCodes.push_back(it->second.size());
  }

  return bitCodes;
}

std::map<int64_t, uint64_t> *GetFreqMap(std::vector<int64_t> &dataBlock, std::map<int64_t, uint64_t> *freqMap)
{
  if (freqMap == nullptr)
    freqMap = new std::map<int64_t, uint64_t>;

  for (int i = 0; i < dataBlock.size(); i++)
  {
    if (freqMap->find(dataBlock[i]) == freqMap->end())
    {
      freqMap->insert(std::make_pair(dataBlock[i], 1));
      continue;
    }
    freqMap->at(dataBlock[i])++;
  }

  return freqMap;
}


bool cmp(const std::pair<int64_t, uint64_t> &lhs, const std::pair<int64_t, uint64_t> &rhs)
{
  if (lhs.second == rhs.second) return lhs.first < rhs.first;
  return lhs.second < rhs.second;
}

}

unsigned SC2::CompressLine(std::vector<uint8_t> &dataLine)
{
  const unsigned lineSize = dataLine.size();
  const unsigned uncompressedSize = BYTE * lineSize;

  // convert uint8_t vector into int64_t vector
  unsigned *words = (unsigned*)dataLine.data();
  std::vector<int64_t> dataBlock;
  dataBlock.resize(lineSize/WORD_GRAN);
  for (int i = 0; i < lineSize / WORD_GRAN; i++)
  {
    unsigned &elem = words[i];
    dataBlock[i] = elem;
  }

  if (m_samplingCnt < m_maxSamplingCnt) // count frequency
  {
    mp_symFreqMap = huffman::GetFreqMap(dataBlock, mp_symFreqMap);
    m_samplingCnt++;
  }
  else if (m_samplingCnt == m_maxSamplingCnt) // build huffmanTree
  {
    // erase least freq symbols from the symFreqMap
    // The vector below is ordered by freq
    if (mp_symFreqMap->size() > HEAP_CAPACITY)
    {
      std::vector<std::pair<int64_t, uint64_t>> symFreqVec(mp_symFreqMap->begin(), mp_symFreqMap->end());
      std::sort(symFreqVec.begin(), symFreqVec.end(), huffman::cmp);
      for (auto it = symFreqVec.begin(); it != symFreqVec.end(); it++)
      {
        int64_t &symbol = it->first;
        uint64_t &freq = it->second;

        mp_symFreqMap->erase(symbol);
        if (!(mp_symFreqMap->size() > HEAP_CAPACITY))
          break;
      }
    }

    huffman::MinHeap minHeap(*mp_symFreqMap);
    minHeap = huffman::BuildHuffmanTree(minHeap);
    huffman::GetHuffmanCode(minHeap.GetRoot()[0], m_huffmanCodes);
    m_samplingCnt++;
  }

  unsigned compressedSize = 0;
  for (int i = 0; i < dataBlock.size(); i++)
  {
    int64_t &symbol = dataBlock[i];
    auto it = m_huffmanCodes.find(symbol);
    if (it == m_huffmanCodes.end()) // Not found from the huffman tree
    {
      // +1b for uncompressed tag
      compressedSize += BYTE*WORD_GRAN + 1;
    }
    else
    {
      unsigned encodedSymbolSize = it->second.size();
      compressedSize += encodedSymbolSize;
    }
  }

  m_Stat->Update(uncompressedSize, compressedSize);
  return compressedSize;
}




}
