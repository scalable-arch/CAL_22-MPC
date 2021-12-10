#include "LoaderGPGPU.h"
#include <strutil.h>

namespace trace {
namespace gpgpusim {
  /*** constructors ***/
  LoaderGPGPU::LoaderGPGPU(const char *filePath)
    : Loader(filePath) { isValid(); }
  LoaderGPGPU::LoaderGPGPU(const std::string filePath)
    : Loader(filePath) { isValid(); }

  /*** getters ***/
  unsigned LoaderGPGPU::GetCachelineSize()
  {
    MemReqGPU_t firstMemReq;
    GetCacheline(&firstMemReq);

    unsigned lineSize = firstMemReq.reqSize;
    Reset();
    return lineSize;
  }

  MemReq_t* LoaderGPGPU::GetCacheline(MemReq_t *memReq)
  {
    MemReqGPU_t *memReqGPU = static_cast<MemReqGPU_t*>(memReq);
    m_FileStream.read((char*)&(memReqGPU->kernelID), 1);   // kid
    m_FileStream.read((char*)&(memReqGPU->mfType), 1);     // mf_type
    m_FileStream.read((char*)&(memReqGPU->cycle), 8);      // cycle
    m_FileStream.read((char*)&(memReqGPU->tpc), 4);        // tpc
    m_FileStream.read((char*)&(memReqGPU->sid), 4);        // sid
    m_FileStream.read((char*)&(memReqGPU->wid), 4);        // wid
    m_FileStream.read((char*)&(memReqGPU->pc), 4);         // pc
    m_FileStream.read((char*)&(memReqGPU->instCnt), 4);    // inst_cnt
    m_FileStream.read((char*)&(memReqGPU->addr), 8);       // mem_addr
    m_FileStream.read((char*)&(memReqGPU->reqType), 4);    // req_type
    m_FileStream.read((char*)&(memReqGPU->row), 4);        // row
    m_FileStream.read((char*)&(memReqGPU->chip), 4);       // chip
    m_FileStream.read((char*)&(memReqGPU->bank), 4);       // bank
    m_FileStream.read((char*)&(memReqGPU->col), 4);        // col
    m_FileStream.read((char*)&(memReqGPU->reqSize), 4);    // req_size

    // data
    memReqGPU->data.resize(memReqGPU->reqSize / sizeof(WORD_SIZE));
    m_FileStream.read(reinterpret_cast<char*>(memReqGPU->data.data()), memReqGPU->reqSize);

    if (m_FileStream.eof())
      memReqGPU->isEnd = true;
    else
      memReqGPU->isEnd = false;
    return memReq;
  }

  /*** public methods ***/
  void LoaderGPGPU::Reset()
  {
    m_FileStream.clear();
    m_FileStream.seekg(0);

    isValid();
  }

  /*** private methods ***/
  void LoaderGPGPU::isValid()
  {
    if (!m_FileStream.is_open())
    {
      printf("Failed to open a file. Check the path of the file.\n");
      exit(1);
    }
    
    m_FileStream.read((char*)&(m_NumKeys), 1);
    if (m_NumKeys != NUM_KEYS)
    {
      printf("The header of the GPGPU-sim trace file is not valid.\n");
      exit(1);
    }
    
    for (int i = 0; i < m_NumKeys; i++)
    {
      char buff[7] = { '\0' };
      int size = 0;

      m_FileStream.read((char*)buff, 6);
      m_FileStream.read((char*)&size, 1);

      m_KeySizeList.insert(std::make_pair(buff, size));
    }

    if (m_FileStream.eof())
    {
      printf("The header of the GPGPU-sim trace file is not valid.\n");
      exit(1);
    }
  }

}

namespace samsung {
  /*** constructors ***/
  LoaderGPGPU::LoaderGPGPU(const char *filePath)
    : Loader(filePath), m_RW(NA) { isValid(); }
  LoaderGPGPU::LoaderGPGPU(const std::string filePath)
    : Loader(filePath), m_RW(NA) { isValid(); }

  /*** getters ***/
  unsigned LoaderGPGPU::GetCachelineSize() { return ACCESS_GRAN; }

  MemReq_t* LoaderGPGPU::GetCacheline(MemReq_t *memReq) { return (this->*mp_GetCacheline)(memReq); }

  /*** public methods ***/
  bool LoaderGPGPU::ReadLine(DatasetAttr &datasetAttr) { return (this->*mp_ReadLine)(datasetAttr); }

  void LoaderGPGPU::Reset()
  {
    m_FileStream.clear();
    m_FileStream.seekg(0);

    isValid();
  }

  /*** private methods ***/
  void LoaderGPGPU::isValid()
  {
    if (!m_FileStream.is_open())
    {
      printf("Failed to open a file. Check the path of the file.\n");
      exit(1);
    }

    // get the first line from the file
    std::string firstLine;
    std::getline(m_FileStream, firstLine);

    // check whether the file is read or write
    std::vector<std::string> splitFirstLine = strutil::split(firstLine, ',');
    for (int i = 0; i < splitFirstLine.size(); i++)
    {
      std::string &col = splitFirstLine[i];
      if (strutil::contains(col, "last"))
      {
        m_RW = READ;
        break;
      }
      else if (strutil::contains(col, "strb"))
      {
        m_RW = WRITE;
        break;
      }
    }
    if (m_RW == NA)
    {
      printf("The header of the GPU traffic file is not valid.\n");
      exit(1);
    }

    // assign appropriate function to the function pointer
    if (m_RW == READ)
    {
      mp_GetCacheline = &LoaderGPGPU::GetCachelineRead;
      mp_ReadLine = &LoaderGPGPU::readLineR;
    }
    else if (m_RW == WRITE)
    {
      mp_GetCacheline = &LoaderGPGPU::GetCachelineWrite;
      mp_ReadLine = &LoaderGPGPU::readLineW;
    }

    if (m_FileStream.eof())
    {
      printf("The header of the GPU traffic file is not valid.\n");
      exit(1);
    }
  }

  bool LoaderGPGPU::readLineR(DatasetAttr &datasetAttr)
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

  bool LoaderGPGPU::readLineW(DatasetAttr &datasetAttr)
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
      datasetAttr.strb[i]  = (uint8_t)std::stoul(lineAttributes[14 + i], 0, 16);
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

  MemReq_t* LoaderGPGPU::GetCachelineRead(MemReq_t *memReq)
  {
    DatasetAttr datasetAttr;

    // push memReq to the queue
    if (m_MemReqQueue.empty())
    {
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
          std::vector<uint8_t> channels = getHandshakingChannels(datasetAttr);
          if (channels.empty())
            continue;

          // enqueue
          for (auto it = channels.begin(); it != channels.end(); it++)
          {
            MemReqGPU_t memReqGPU;
            uint8_t &ch = *it;

            uint64_t &cycle = datasetAttr.cycle;
            uint8_t &last = datasetAttr.last[ch];
            uint8_t *data = datasetAttr.data[ch];

            memReqGPU.rw = READ;
            
            memReqGPU.cycle = cycle;
            memReqGPU.ch = ch;
            memReqGPU.last = last;
            memReqGPU.reqSize = ACCESS_GRAN * 2;   // TODO: AccessGran can be modified in the future
            memReqGPU.data.resize(ACCESS_GRAN);
            std::copy(data, data + ACCESS_GRAN, memReqGPU.data.begin());
            memReqGPU.isEnd = false;

            m_MemReqQueue.push(memReqGPU);
          }
          break;
        }
        // enqueue empty memReq (last)
        else 
        {
          MemReqGPU_t memReqGPU;
          memReqGPU.Reset();
          memReqGPU.isEnd = true;

          m_MemReqQueue.push(memReqGPU);
          break;
        }
      }
    }

    // pop from the queue and return it
    MemReqGPU_t *memReqGPU = static_cast<MemReqGPU_t*>(memReq);
    MemReqGPU_t popMemReqGPU = m_MemReqQueue.front();

    *memReqGPU = popMemReqGPU;
    m_MemReqQueue.pop();
    return memReq;
  }

  MemReq_t* LoaderGPGPU::GetCachelineWrite(MemReq_t *memReq)
  {
    DatasetAttr datasetAttr;

    // push memReq to the queue
    if (m_MemReqQueue.empty())
    {
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
          std::vector<uint8_t> channels = getHandshakingChannels(datasetAttr);
          if (channels.empty())
            continue;

          for (auto it = channels.begin(); it != channels.end(); it++)
          {
            MemReqGPU_t memReqGPU;
            uint8_t &ch = *it;

            uint64_t &cycle = datasetAttr.cycle;
            uint32_t &strb = datasetAttr.strb[ch];
            uint8_t *data = datasetAttr.data[ch];

            memReqGPU.rw = WRITE;
            
            memReqGPU.cycle = cycle;
            memReqGPU.ch = ch;
            memReqGPU.strb = strb;
            memReqGPU.reqSize = ACCESS_GRAN * 2;   // TODO: AccessGran can be modified in the future
            memReqGPU.data.resize(ACCESS_GRAN);
            std::copy(data, data + ACCESS_GRAN, memReqGPU.data.begin());
            memReqGPU.isEnd = false;

            m_MemReqQueue.push(memReqGPU);
          }
          break;
        }
        else // last memReq
        {
          MemReqGPU_t memReqGPU;
          memReqGPU.Reset();
          memReqGPU.isEnd = true;

          m_MemReqQueue.push(memReqGPU);
          break;
        }
      }
    }

    // pop from the queue and return it
    MemReqGPU_t *memReqGPU = static_cast<MemReqGPU_t*>(memReq);
    MemReqGPU_t popMemReqGPU = m_MemReqQueue.front();

    *memReqGPU = popMemReqGPU;
    m_MemReqQueue.pop();
    return memReq;
  }

  std::vector<uint8_t> LoaderGPGPU::getHandshakingChannels(DatasetAttr &datasetAttr)
  {
    std::vector<uint8_t> channels;
    for (int i = 0; i < NUM_CH; i++)
      if (datasetAttr.valid[i] == 1 && datasetAttr.ready[i] == 1)
        channels.push_back(i);
    return channels;
  }
  
  

}
}
