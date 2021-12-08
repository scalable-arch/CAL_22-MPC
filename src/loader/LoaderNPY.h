#ifndef __LOADERNPY_H__
#define __LOADERNPY_H__

#include "Loader.h"

namespace trace
{

class LoaderNPY : public Loader
{
public:
  /*** constructors ***/
  LoaderNPY(const char *filePath);
  LoaderNPY(const std::string filePath);

  /*** getters ***/
  virtual MemReq_t* GetCacheline(MemReq_t *memReq);
  virtual unsigned GetCachelineSize();

  /*** methods ***/
  virtual void Reset();

private:
  std::vector<WORD_SIZE> m_DataLines;
  std::vector<uint64_t> m_DataShape;
  uint64_t m_CurrentLine;
};

}

#endif  // __LOADERNPY_H__
