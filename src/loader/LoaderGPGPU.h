#ifndef __LOADERGPGPU_H__
#define __LOADERGPGPU_H__

#include <map>
#include <strutil.h>
#include <fmt/core.h>

#include "Loader.h"

namespace trace {
namespace gpgpusim {

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

namespace samsung {

#define NUM_CH 4
#define ACCESS_GRAN 32

struct DatasetAttr
{
  uint64_t cycle;
  uint8_t clock;

  uint8_t valid[NUM_CH];
  uint8_t ready[NUM_CH];
  uint8_t last[NUM_CH];

  uint8_t data[NUM_CH][ACCESS_GRAN];

  void Reset()
  {
    cycle = 0;
    clock = 0;

    memset(valid, 0, sizeof(uint8_t) * NUM_CH);
    memset(ready, 0, sizeof(uint8_t) * NUM_CH);
    memset(last, 0, sizeof(uint8_t) * NUM_CH);

    memset(data, 0, sizeof(uint8_t) * NUM_CH * ACCESS_GRAN);
  }

  void Print()
  {
    printf("Time:%14lu, clk:%2d, ", cycle, clock);
    for (int i = 0; i < NUM_CH; i++)
      printf("val_%1d:%2d, ", i, valid[i]);
    for (int i = 0; i < NUM_CH; i++)
      printf("rdy_%1d:%2d, ", i, ready[i]);
    for (int i = 0; i < NUM_CH; i++)
      printf("last_%1d:%2d, ", i, last[i]);
    for (int i = 0; i < NUM_CH; i++)
    {
      printf("d_%1d: ", i);
      for (int j = 0; j < ACCESS_GRAN; j++)
        printf("%02x", data[i][j]);
      printf(" ");
    }
    printf("\n");
  }
};

struct MemReqGPU_t : public MemReq_t
{
  virtual void Reset()
  {
    MemReq_t::Reset();

    cycle = 0;
    ch = 0;
    last = 0;
  }

  uint64_t cycle;
  uint8_t ch;
  uint8_t last;
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
      printf("The given traffic file is not valid.\n");
      exit(1);
    }
  }
  LoaderGPGPU(const std::string filePath)
    : Loader(filePath)
  {
    if (!isValid())
    {
      printf("The given traffic file is not valid.\n");
      exit(1);
    }
  }

  /*** getters ***/
  // Get a line in 32B granularity
  virtual MemReq_t* GetLine(MemReq_t *memReq)
  {
    DatasetAttr datasetAttr;
    samsung::MemReqGPU_t *memReqGPU = static_cast<samsung::MemReqGPU_t*>(memReq);
    while (true)
    {
      if (ReadLine(datasetAttr))
      {
        if (!datasetAttr.clock)
          continue;
        if (!datasetAttr.valid[0] && !datasetAttr.valid[1]
            && !datasetAttr.valid[2] && !datasetAttr.valid[3])
          continue;

        // check which channel is handshaking
        uint8_t ch = getHandshakingChannel(datasetAttr);
        if (ch == -1)
          continue;

        // move all data to the "memReq"
        uint64_t &cycle = datasetAttr.cycle;
        uint8_t &last = datasetAttr.last[ch];
        uint8_t *data = datasetAttr.data[ch];
        
        memReqGPU->cycle = cycle;
        memReqGPU->ch = ch;
        memReqGPU->last = last;
        memReqGPU->reqSize = ACCESS_GRAN * 2;   // TODO: AccessGran can be modified in the future
        memReqGPU->data.resize(ACCESS_GRAN);
        std::copy(data, data + ACCESS_GRAN, memReqGPU->data.begin());
        memReqGPU->isEnd = false;
      }
      else
      {
        memReqGPU->Reset();
        memReqGPU->isEnd = true;
      }
      return memReq;
    }
  }

  // Get line size
  virtual unsigned GetLineSize()
  {
    return ACCESS_GRAN;
  }

  // Read line from the file
  bool ReadLine(DatasetAttr &datasetAttr)
  {
    std::string line;
    std::getline(m_FileStream, line);
    if (m_FileStream.eof())
      return false;
    std::vector<std::string> lineAttributes = strutil::split(line, ',');

    datasetAttr.cycle    = std::stoull(lineAttributes[0]);
    datasetAttr.clock    = (uint8_t)std::stoi(lineAttributes[1]);
    for (int i = 0; i < NUM_CH; i++)
    {
      datasetAttr.valid[i] = (uint8_t)std::stoi(lineAttributes[ 2 + i]);
      datasetAttr.ready[i] = (uint8_t)std::stoi(lineAttributes[10 + i]);
      datasetAttr.last[i]  = (uint8_t)std::stoi(lineAttributes[14 + i]);
      std::string &hexStringData = lineAttributes[6 + i];
      for (int j = 0; j < ACCESS_GRAN; j++)
      {
        std::string hex(hexStringData.begin() + 2 * j, hexStringData.begin() + 2 * j + 2);
        uint8_t c = std::stoi(hex, nullptr,  16);
        datasetAttr.data[i][j] = c;
      }
    }
    return true;
  }

  /*** methods ***/
  virtual void Reset()
  {
    m_FileStream.clear();
    m_FileStream.seekg(0);

    if (!isValid())
    {
      printf("The given traffic file is not valid.\n");
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

    std::string firstLine;
    std::getline(m_FileStream, firstLine);

    return true;
  }

  uint8_t getHandshakingChannel(DatasetAttr &datasetAttr)
  {
    for (int i = 0; i < NUM_CH; i++)
      if (datasetAttr.valid[i] == 1 && datasetAttr.ready[i] == 1)
        return i;
    return -1;
  }
};


}}

#endif  // __LOADERGPGPU_H__
