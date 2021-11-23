#pragma once

#include <vector>
#include <iostream>

namespace comp
{

typedef std::vector<int> compSizeList;

// Symbol class for cacheLine, PredictedLine, ResidueLine
class Symbol
{
public:
  // constructors
  Symbol()
    : m_RootIndex(-1), m_LineSize(0) {};

  Symbol(std::vector<uint8_t> &symbolLine)
    : m_RootIndex(-1), m_LineSize(symbolLine.size()), m_Line(symbolLine) {}

  Symbol(uint8_t *symbolLine, int lineSize)
    : m_RootIndex(-1), m_LineSize(lineSize), m_Line(symbolLine, symbolLine + lineSize) {}

  // setters
  void SetSize(int len)
  {
    m_LineSize = len;
    m_Line.resize(len);
  }

  void SetRootIndex(int rootIndex)
  {
    m_RootIndex = rootIndex;
  }

  // getters
  int GetRootIndex() { return m_RootIndex; }
  int GetLineSize()  { return m_LineSize; }

  // operator overloading
  uint8_t operator[] (int i) const { return m_Line[i]; }
  uint8_t &operator[] (int i) { return m_Line[i]; }

private:
  int m_RootIndex;
  int m_LineSize;

  std::vector<uint8_t> m_Line;
};

// Binary class for BitplaneArray, ScannedArray
class Binary
{
public:
  // constructors
  Binary()
    : m_RootIndex(-1), m_ArrayRowSize(0), m_ArrayColSize(0) {}

  std::vector<uint8_t> operator[] (int i) const { return m_Array[i]; }
  std::vector<uint8_t> &operator[] (int i) { return m_Array[i]; }

  // setters
  void SetSize(int rows, int cols)
  {
    m_ArrayRowSize = rows;
    m_ArrayColSize = cols;
  
    m_Array.resize(rows);
    for (int i = 0; i < rows; i++)
      m_Array[i].resize(cols);
  }

  void SetRootIndex(int rootIndex)
  {
    m_RootIndex = rootIndex;
  }

  // getters
  int GetRootIndex() { return m_RootIndex; }
  int GetRowSize()   { return m_ArrayRowSize; }
  int GetColSize()   { return m_ArrayColSize; }

  // methods
  bool IsRowZeros(int row)
  {
    for (int i = 0; i < m_ArrayColSize; i++)
      if (m_Array[row][i] != 0x00)
        return false;
    return true;
  }

private:
  int m_RootIndex;

  int m_ArrayRowSize;
  int m_ArrayColSize;

  std::vector<std::vector<uint8_t>> m_Array;
};

}
