#include "CPACK.h"
#include <endian.h>

namespace comp
{

unsigned CPACK::CompressLine(std::vector<uint8_t> &dataLine)
{
  unsigned uncompSize = dataLine.size() * BYTE;
  unsigned currCSize = 0;

  CPACKResult* m_stat = static_cast<CPACKResult*>(m_Stat);

  for (int i = 0; i < dataLine.size() / WORDSIZE; i++)
  {
    uint8_t word[WORDSIZE] = {
      dataLine[WORDSIZE * i], dataLine[WORDSIZE * i + 1], dataLine[WORDSIZE * i + 2], dataLine[WORDSIZE * i + 3]
    };

    // check zero patterns
    if (word[0] == 0 && word[1] == 0 && word[2] == 0)
    {
      // pattern zzzz (00)
      if (word[3] == 0)
      {
        currCSize += m_PatternLength[0];
        m_stat->UpdatePattern((int)CPACKPattern::ZZZZ);
      }
      // pattern zzzx (1100)B
      else
      {
        currCSize += m_PatternLength[4];
        m_stat->UpdatePattern((int)CPACKPattern::ZZZX);
      }
      continue;
    }

    // check dictionary patterns
    bool found = false;
    for (int j = 0; j < NUM_ENTRY; j++)
    {
      uint8_t *currEntry = m_Dictionary[j];
      uint8_t dictWord[WORDSIZE] = {
        currEntry[0], currEntry[1], currEntry[2], currEntry[3]
      };

      if (word[0] == dictWord[0] && word[1] == dictWord[1])
      {
        if (word[2] == dictWord[2])
        {
          // pattern mmmm (10)bbbb
          if (word[3] == dictWord[3])
          {
            currCSize += m_PatternLength[2];
            m_stat->UpdatePattern((int)CPACKPattern::MMMM);
          }
          // pattern mmmx (1110)bbbbB
          else
          {
            currCSize += m_PatternLength[5];
            m_stat->UpdatePattern((int)CPACKPattern::MMMX);
          }
        }
        // pattern mmxx (1100)bbbbBB
        else
        {
          currCSize += m_PatternLength[3];
          m_stat->UpdatePattern((int)CPACKPattern::MMXX);
        }
        found = true;
        break;
      }
    }

    if (found)
    {
      continue;
    }
   
    // pattern xxxx (01)BBBB
    else
    {
      currCSize += m_PatternLength[1];
      
      // add new pattern to dictionary
      uint8_t *newEntry = new uint8_t[WORDSIZE];
      for (int k = 0; k < WORDSIZE; k++)
        newEntry[k] = word[k];
      m_Dictionary.push_back(newEntry);
      
      uint8_t *popEntry = m_Dictionary.front();
      m_Dictionary.pop_front();
      delete[] popEntry;

      m_stat->UpdatePattern((int)CPACKPattern::XXXX);
    }
  }

  m_stat->Update(uncompSize, currCSize);
  return currCSize;
}

}
