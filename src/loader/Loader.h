#ifndef __LOADER_H__
#define __LOADER_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define WORD_SIZE uint8_t

typedef uint64_t addr_t;

namespace trace
{

enum rw_t
{
	READ,
	WRITE,
  NA,
};

struct MemReq_t
{
	addr_t addr;
	rw_t rw;

	uint32_t reqSize;
	std::vector<WORD_SIZE> data;

	bool isEnd;
};

class Loader
{
public:
	/*** constructors ***/
	Loader(const char *filePath)
    : m_FilePath(filePath)
  {
    m_FileStream.open(m_FilePath, std::ios_base::in | std::ios_base::binary);
  };
	Loader(const std::string filePath)
    : m_FilePath(filePath)
  {
      m_FileStream.open(filePath.c_str(), std::ios_base::in | std::ios_base::binary);
  }
	/*** getters ***/
	virtual MemReq_t* GetLine(MemReq_t *) = 0;
  virtual unsigned GetLineSize() = 0;

	/*** methods ***/
	virtual void Reset() = 0;

protected:
	const std::string m_FilePath;
	std::ifstream m_FileStream;

};

}

#endif  // __LOADER_H__
