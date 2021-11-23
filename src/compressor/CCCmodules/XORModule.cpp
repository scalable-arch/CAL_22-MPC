#include "XORModule.h"

namespace comp
{
Binary XORModule::ProcessLine(Binary &bitplane)
{
  Binary bitplaneXOR = bitplane;
  
  if (mb_ConsecutiveXOR)
  {
    for (int i = 1; i < bitplane.GetRowSize(); i++)
      for (int j = 1; j < bitplane.GetColSize(); j++)
        bitplaneXOR[i][j] = bitplane[i][j] ^ bitplane[i - 1][j];
  }
  else
  {
    for (int i = 1; i < bitplane.GetRowSize(); i++)
      for (int j = 1; j < bitplane.GetColSize(); j++)
        bitplaneXOR[i][j] = bitplane[i][j] ^ bitplane[0][j];
  }

  return bitplaneXOR;
}
}

