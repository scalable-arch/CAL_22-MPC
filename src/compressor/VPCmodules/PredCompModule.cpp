#include "PredCompModule.h"

#include "ResidueModule.h"
#include "BitplaneModule.h"
#include "XORModule.h"
#include "ScanModule.h"
#include "FPCModule.h"

namespace comp
{
Binary PredCompModule::CompressLine(std::vector<uint8_t> &dataLine, int nothing)
{
  Symbol residue;
  Binary bitplane;
  Binary bitplaneXOR;
  Binary scanned;
//  int compressedSize;

  residue = mp_ResidueModule->ProcessLine(dataLine);
  bitplane = mp_BitplaneModule->ProcessLine(residue);
  bitplaneXOR = mp_XORModule->ProcessLine(bitplane);
  scanned = mp_ScanModule->ProcessLine(bitplaneXOR);
//  compressedSize = mp_FPCModule->ProcessLine(scanned);

  return scanned;
}

double PredCompModule::GetMAE(std::vector<uint8_t> &dataLine)
{
  return mp_ResidueModule->GetMAE(dataLine);
}
double PredCompModule::GetMSE(std::vector<uint8_t> &dataLine)
{
  return mp_ResidueModule->GetMSE(dataLine);
}
}
