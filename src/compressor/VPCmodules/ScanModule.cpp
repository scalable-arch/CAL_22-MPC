#include "ScanModule.h"

namespace comp
{

Binary ScanModule::ProcessLine(Binary &bitplane)
{
  int size = bitplane.GetRowSize() * bitplane.GetColSize();

  Binary scanned;
  scanned.SetSize(size / SCANNED_SYMBOLSIZE, SCANNED_SYMBOLSIZE);

  for (int i = 0; i < m_Table.TableSize; i++)
  {
    int row = m_Table.Rows[i];
    int col = m_Table.Cols[i];

    scanned[i / scanned.GetColSize()][i % scanned.GetColSize()] = bitplane[row][col];
  }

  return scanned;
}

void ScanModule::loadTable(const std::string filePath)
{
  std::ifstream inFile;
  int intBuffer;
  
  inFile.open(filePath, std::ios::binary);

  // read TableSize
  inFile.read(reinterpret_cast<char *>(&intBuffer), sizeof(intBuffer));
  m_Table.TableSize = intBuffer;

  // read Rows
  m_Table.Rows.resize(m_Table.TableSize);
  for (int i = 0; i < m_Table.TableSize; i++)
  {
    inFile.read(reinterpret_cast<char *>(&intBuffer), sizeof(intBuffer));
    m_Table.Rows[i] = intBuffer;
  }

  // read Cols
  m_Table.Cols.resize(m_Table.TableSize);
  for (int i = 0; i < m_Table.TableSize; i++)
  {
    inFile.read(reinterpret_cast<char *>(&intBuffer), sizeof(intBuffer));
    m_Table.Cols[i] = intBuffer;
  }
}

}
