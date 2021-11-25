#pragma once

#include <vector>
#include <fstream>

#include "PredCompModule.h"

#define SCANNED_SYMBOLSIZE 16

namespace comp
{

struct ScanTable
{
  std::vector<int> Rows;
  std::vector<int> Cols;

  int TableSize;
};

class ScanModule
{
friend class PredCompModule;

public:
  // constructor
  ScanModule(int tableSize,
    std::vector<int> rows, std::vector<int> cols)
  {
    m_Table.TableSize = tableSize;

    m_Table.Rows = rows;
    m_Table.Cols = cols;
  }

  Binary ProcessLine(Binary &bitplane);

private:
  void loadTable(const std::string filePath);

private:
  ScanTable m_Table;
};

}
