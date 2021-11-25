#pragma once

#include "CompressionModule.h"
#include "CompStruct.h"

namespace comp
{

class ResidueModule;
class BitplaneModule;
class XORModule;
class ScanModule;

class PredCompModule : public CompressionModule
{
public:
  // constructor
  PredCompModule(int lineSize,
    ResidueModule *residueModule,
    BitplaneModule *bitplaneModule, XORModule *xorModule, ScanModule *scanModule)
    : CompressionModule(lineSize),
      mp_ResidueModule(residueModule),
      mp_BitplaneModule(bitplaneModule), mp_XORModule(xorModule), mp_ScanModule(scanModule) {}


  unsigned CompressLine(std::vector<uint8_t> &dataLine) { std::cout << "Not implemented." << std::endl; exit(1); }
  Binary CompressLine(std::vector<uint8_t> &dataLine, int nothing=0);

protected:
  ResidueModule  *mp_ResidueModule;
  BitplaneModule *mp_BitplaneModule;
  XORModule      *mp_XORModule;
  ScanModule     *mp_ScanModule;
};

}
