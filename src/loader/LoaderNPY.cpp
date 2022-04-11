#include "LoaderNPY.h"
#include <npy.hpp>

namespace trace
{

/*** constructors ***/
LoaderNPY::LoaderNPY(const char *filePath)
  : Loader(filePath), m_CurrentLine(0) { Reset(); }
LoaderNPY::LoaderNPY(const std::string filePath)
  : Loader(filePath), m_CurrentLine(0) { Reset(); }

/*** getters ***/
MemReq_t* LoaderNPY::GetCacheline(MemReq_t *memReq)
{
  uint64_t &numTotalLines = m_DataShape[0];
  uint64_t &lineSize = m_DataShape[1];
  
  memReq->addr = 0;
  memReq->rw = NA;

  memReq->reqSize = lineSize;
  memReq->data.resize(lineSize / sizeof(WORD_SIZE));
  memReq->data.assign(
      m_DataLines.begin() + m_CurrentLine * lineSize,
      m_DataLines.begin() + (m_CurrentLine + 1) * lineSize);

  m_CurrentLine++;
  if (m_CurrentLine == numTotalLines)
    memReq->isEnd = true;
  else
    memReq->isEnd = false;
  return memReq;
}

unsigned LoaderNPY::GetCachelineSize()
{
  unsigned lineSize = m_DataShape[1];
  return lineSize;
}

unsigned long long LoaderNPY::GetNumLines()
{
  unsigned long long numLines = m_DataShape[0];
  return numLines;
}

void LoaderNPY::Reset()
{
  bool fortran_order;
  npy::LoadArrayFromNumpy(m_FilePath, m_DataShape, fortran_order, m_DataLines);

  m_CurrentLine = 0;
}

}
