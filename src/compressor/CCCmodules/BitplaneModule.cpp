#include "BitplaneModule.h"
#include "PredCompModule.h"
#include "../Compressor.h"

namespace comp
{
  Binary BitplaneModule::ProcessLine(Symbol &residueLine)
  {
    // residueLine to bitArray
    int lineSize = residueLine.GetLineSize();

    Binary bitArray;
    bitArray.SetSize(lineSize, BYTE);
    for (int i = 0; i < lineSize; i++)
      bitArray[i] = convertToBitVector(residueLine[i]);

    // bitArray to bitplane
    Binary bitplane = transposeBinaryArray(bitArray);
    
    bitplane.SetRootIndex(residueLine.GetRootIndex());

    return bitplane;
  }

  std::vector<uint8_t> BitplaneModule::convertToBitVector(uint8_t symbol)
  {
    std::vector<uint8_t> bitVector;
    bitVector.resize(BYTE);

    for (int i = 0; i < BYTE; i++)
    {
      bitVector[i] = (symbol >> ((BYTE - 1) - i)) & m_Mask;
    }

    return bitVector;
  }


  Binary BitplaneModule::transposeBinaryArray(Binary &binary)
  {
    int lineSize = binary.GetRowSize();

    Binary bitplane;
    bitplane.SetSize(BYTE, lineSize);

    for (int row = 0; row < lineSize; row++)
      for (int col = 0; col < BYTE; col++)
        bitplane[col][row] = binary[row][col];

    return bitplane;
  }

}
