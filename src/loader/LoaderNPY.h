#ifndef __LOADERNPY_H__
#define __LOADERNPY_H__

#include <npy.hpp>

#include "Loader.h"

namespace trace
{

class LoaderNPY : public Loader
{
public:
  /*** constructors ***/
  LoaderNPY(const char *filePath)
    : Loader(filePath), m_CurrentLine(0)
  {
    bool fortran_order;
    npy::LoadArrayFromNumpy(m_FilePath, m_DataShape, fortran_order, m_DataLines);
  };
  LoaderNPY(const std::string filePath)
    : Loader(filePath), m_CurrentLine(0)
  {
    bool fortran_order;
    npy::LoadArrayFromNumpy(m_FilePath, m_DataShape, fortran_order, m_DataLines);
  };

  /*** getters ***/
  virtual MemReq_t* GetLine(MemReq_t *memReq)
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

  /*** methods ***/
  virtual void Reset()
  {
    m_CurrentLine = 0;
  }

private:
  std::vector<WORD_SIZE> m_DataLines;
  std::vector<uint64_t> m_DataShape;
  uint64_t m_CurrentLine;
};

}

#endif  // __LOADERNPY_H__
