#ifndef __LOADERGPGPU_H__
#define __LOADERGPGPU_H__

#include <map>
#include "Loader.h"

namespace trace
{

// GPGPU-sim mem request type
enum reqTypeGPU
{
  GLOBAL_ACC_R  = 0,
  LOCAL_ACC_R   = 1,
  CONST_ACC_R   = 2,
  TEXTURE_ACC_R = 3,
  GLOBAL_ACC_W  = 4,
  LOCAL_ACC_W   = 5,
  L1_WRBK_ACC   = 6,
  L2_WRBK_ACC   = 7,
  INST_ACC_R    = 8,
};

// GPGPU-sim mem fetch type
enum fetchTypeGPU
{
  READ_REQUEST  = 0,
  WRITE_REQUEST = 1,
  READ_REPLY    = 2,
  WRITE_ACK     = 3,
};

struct MemReqGPU_t : public MemReq_t
{
  uint8_t kernelID;
  uint64_t cycle;

  uint32_t tpc;    // texture processing cluster
  uint32_t sid;    // shader core id
  uint32_t wid;    // warp id
  uint32_t pc;
  uint32_t instCnt;

  reqTypeGPU reqType;
  fetchTypeGPU mfType;

  // DRAM command
  uint32_t row;
  uint32_t chip;
  uint32_t bank;
  uint32_t col;
};

class LoaderGPGPU : public Loader
{
public:
  /*** constructors ***/
  LoaderGPGPU(const char *filePath)
    : Loader(filePath)
  {
    if (!isValid())
    {
      printf("GPGPU-sim trace file is not valid.\n");
      exit(1);
    }
  }
  LoaderGPGPU(const std::string filePath)
    : Loader(filePath)
  {
    if (!isValid())
    {
      printf("GPGPU-sim trace file is not valid.\n");
      exit(1);
    }
  }

  /*** getters ***/
  virtual MemReq_t* GetLine(MemReq_t *memReq)
  {
//    m_FileStream.ignore(4);                                                      // req_pos
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->kernelID), 1);   // kid
//    m_FileStream.ignore(1);                                                      // rw <WIP, change here to memFetchType (GPGPU-sim and This code)>
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->mfType), 1);     // mf_type
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->cycle), 8);      // cycle
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->tpc), 4);        // tpc
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->sid), 4);        // sid
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->wid), 4);        // wid
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->pc), 4);         // pc
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->instCnt), 4);    // inst_cnt
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->addr), 8);       // mem_addr
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->reqType), 4);    // req_type
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->row), 4);        // row
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->chip), 4);       // chip
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->bank), 4);       // bank
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->col), 4);        // col
    m_FileStream.read((char*)&(static_cast<MemReqGPU_t*>(memReq)->reqSize), 4);    // req_size

    // data
    static_cast<MemReqGPU_t*>(memReq)->data.resize(static_cast<MemReqGPU_t*>(memReq)->reqSize / sizeof(WORD_SIZE));
    m_FileStream.read(reinterpret_cast<char*>(static_cast<MemReqGPU_t*>(memReq)->data.data()), static_cast<MemReqGPU_t*>(memReq)->reqSize);

    if (m_FileStream.eof())
      static_cast<MemReqGPU_t*>(memReq)->isEnd = true;
    else
      static_cast<MemReqGPU_t*>(memReq)->isEnd = false;
    return memReq;
  }

  // Get line size
  // It will reset the filepointer to the beginning
  virtual unsigned GetLineSize()
  {
    MemReqGPU_t firstMemReq;
    GetLine(&firstMemReq);

    unsigned lineSize = firstMemReq.reqSize;
    Reset();
    return lineSize;
  }

  /*** methods ***/
  // reset filepointer
  virtual void Reset()
  {
    m_FileStream.clear();
    m_FileStream.seekg(0);

    if (!isValid())
    {
      printf("GPGPU-sim trace file is not valid.\n");
      exit(1);
    }
  }

private:
  bool isValid()
  {
    if (!m_FileStream.is_open())
    {
      printf("Failed to open a file. Check the path of the file.\n");
      return false;
    }
    
    m_FileStream.read((char*)&(m_NumKeys), 1);
    if (m_NumKeys != 17)
      return false;
    
    for (int i = 0; i < m_NumKeys; i++)
    {
      char buff[7] = { '\0' };
      int size = 0;

      m_FileStream.read((char*)buff, 6);
      m_FileStream.read((char*)&size, 1);

      m_KeySizeList.insert(std::make_pair(buff, size));
    }

    return true;
  }

private:
  // header data
  uint8_t m_NumKeys;
  std::map<std::string, int> m_KeySizeList;
  
};

}

#endif  // __LOADERGPGPU_H__
