#include <cmath>

#include <json/json.h>
#include "Compressor.h"
#include "VPC.h"
#include "VPCmodules/AllWordSameModule.h"
#include "VPCmodules/AllZeroModule.h"
#include "VPCmodules/BitplaneModule.h"
#include "VPCmodules/CompressionModule.h"
#include "VPCmodules/FPCModule.h"
#include "VPCmodules/PatternModule.h"
#include "VPCmodules/PredCompModule.h"
#include "VPCmodules/PredictorModule.h"
#include "VPCmodules/ResidueModule.h"
#include "VPCmodules/ScanModule.h"
#include "VPCmodules/XORModule.h"
#include "VPCmodules/CompStruct.h"

namespace comp
{

unsigned VPC::CompressLine(std::vector<uint8_t> &dataLine)
{
  return (this->*compressLine)(dataLine);
}

unsigned VPC::compressLineAllWordSame(std::vector<uint8_t> &dataLine)
{
  // Check ALLZERO
  {
    bool isAllZeros;
    unsigned compressedSize = checkAllZeros(0, isAllZeros, dataLine);

    if (isAllZeros)
      return compressedSize;
  }

  // Check ALLWORDSAME
  {
    bool isAllWordSame;
    unsigned compressedSize = checkAllWordSame(1, isAllWordSame, dataLine);

    if (isAllWordSame)
      return compressedSize;
  }

  // Check other patterns
  {
    unsigned compressedSize = checkOtherPatterns(2, dataLine);
    return compressedSize;
  }
}

unsigned VPC::compressLineOnlyAllZero(std::vector<uint8_t> &dataLine)
{
  // Check ALLZERO
  {
    bool isAllZeros;
    unsigned compressedSize = checkAllZeros(0, isAllZeros, dataLine);

    if (isAllZeros)
      return compressedSize;
  }

  // Check other patterns
  {
    unsigned compressedSize = checkOtherPatterns(1, dataLine);
    return compressedSize;
  }
}

void VPC::parseConfig(std::string &configPath)
{
  // open config file
  std::ifstream configFile;
  configFile.open(configPath);
  if (!configFile.is_open())
  {
    printf("Invalid File! \"%s\" is not valid path.\n", configPath.c_str());
    exit(1);
  }

  // declare json root
  Json::Value root;

  // instantiate json parser
  Json::CharReaderBuilder builder;
  builder["collecComments"] = false;
  JSONCPP_STRING errs;
  if (!parseFromStream(builder, configFile, &root, &errs))
  {
    std::cout << errs << std::endl;
    printf("Parsing ERROR! \"%s\" is not valid json file.\n", configPath.c_str());
    exit(1);
  }

  // parse overview field
  {
    m_NumModules = root["overview"]["num_modules"].asInt();
    m_NumClusters = m_NumModules + 1;
    m_LineSize = root["overview"]["lineSize"].asInt();
    if (root["overview"]["encoding_bits"].isNull())
    {
      int encodingBits = (int)ceil(log2f((float)m_NumClusters));
      m_EncodingBits.insert(std::make_pair(-1, encodingBits));
      for (int i = 0; i < m_NumModules; i++)
        m_EncodingBits.insert(std::make_pair(i, encodingBits));
    }
    else
    {
      for (int i = 0; i < m_NumClusters; i++)
      {
        int encodingBits = root["overview"]["encoding_bits"][i].asInt();
        m_EncodingBits.insert(std::make_pair(i - 1, encodingBits));
      }
    }
  }

  // parse module field
  {
    m_CompModules.resize(m_NumModules);
    for (int i = 0; i < m_NumModules; i++)
    {
      // reference of a module spec for convenient
      Json::Value &moduleSpec = root["modules"][std::to_string(i)];

      CompressionModule *compModule;
      std::string moduleName = moduleSpec["name"].asString();

      // if module is 'PredComp', parse submodules
      if (moduleName == "PredComp")
      {
        Json::Value &subModuleSpec = moduleSpec["submodules"];

        // residue module
        PredictorModule *predModule;

        Json::Value &predModuleSpec = subModuleSpec["ResidueModule"]["PredictorModule"];
        std::string predModuleName = predModuleSpec["name"].asString();
        int lineSize = predModuleSpec["LineSize"].asInt();
        int rootIndex = predModuleSpec["RootIndex"].asInt();
        if (predModuleName == "WeightBasePredictor")
        {
          std::vector<int> baseIndexTable;
          std::vector<float> weightTable;

          baseIndexTable.resize(lineSize);
          weightTable.resize(lineSize);
          for (int j = 0; j < lineSize; j++)
          {
            baseIndexTable[j] = predModuleSpec["BaseIndexTable"][j].asInt();
            weightTable[j] = predModuleSpec["WeightTable"][j].asFloat();
          }
          
          predModule = new WeightBasePredictor(rootIndex, lineSize, baseIndexTable, weightTable);
        }
        else if (predModuleName == "DiffBasePredictor")
        {
          std::vector<int> baseIndexTable;
          std::vector<int> diffTable;

          baseIndexTable.resize(lineSize);
          diffTable.resize(lineSize);
          for (int j = 0; j < lineSize; j++)
          {
            baseIndexTable[j] = predModuleSpec["BaseIndexTable"][j].asInt();
            diffTable[j] = predModuleSpec["DiffTable"][j].asInt();
          }

          predModule = new DiffBasePredictor(rootIndex, lineSize, baseIndexTable, diffTable);
        }
        else if (predModuleName == "OneBasePredictor")
        {
          predModule = new OneBasePredictor(rootIndex, lineSize);
        }
        else if (predModuleName == "ConsecutiveBasePredictor")
        {
          predModule = new ConsecutiveBasePredictor(rootIndex, lineSize);
        }
        else
        {
          printf("""%s"" is not a valid predictor module. Check the config file.\n", predModuleName.c_str());
          exit(1);
        }
        ResidueModule *residueModule = new ResidueModule(predModule);

        // bitplane module
        BitplaneModule *bitplaneModule = new BitplaneModule();

        // xor module
        bool consecutiveXOR = subModuleSpec["XORModule"]["consecutiveXOR"].asBool();
        XORModule *xorModule = new XORModule(consecutiveXOR);

        // scan module
        int tableSize = subModuleSpec["ScanModule"]["TableSize"].asInt();
        std::vector<int> rows;
        std::vector<int> cols;

        rows.resize(tableSize);
        cols.resize(tableSize);
        for (int j = 0; j < tableSize; j++)
        {
          rows[j] = subModuleSpec["ScanModule"]["Rows"][j].asInt();
          cols[j] = subModuleSpec["ScanModule"]["Cols"][j].asInt();
        }
        ScanModule *scanModule = new ScanModule(tableSize, rows, cols);

        // fpc module
        std::vector<PatternModule*> patternModules;

        int numPatternModule = subModuleSpec["FPCModule"]["num_modules"].asInt();
        patternModules.resize(numPatternModule);
        for (int j = 0; j < numPatternModule; j++)
        {
          Json::Value &patternModuleSpec = subModuleSpec["FPCModule"][std::to_string(j)];

          PatternModule *patternModule;
          std::string patternModuleName = patternModuleSpec["name"].asString();

          // parse pattern modules
          if (patternModuleName == "ZerosPattern")
          {
            int encodingBitsZRLE = patternModuleSpec["encodingBitsZRLE"].asInt();
            int encodingBitsZero = patternModuleSpec["encodingBitsZero"].asInt();

            patternModule = new ZerosPatternModule(encodingBitsZRLE, encodingBitsZero);
          }
          else if (patternModuleName == "SingleOnePattern")
          {
            int encodingBits = patternModuleSpec["encodingBits"].asInt();

            patternModule = new SingleOnePatternModule(encodingBits);
          }
          else if (patternModuleName == "TwoConsecutiveOnesPattern")
          {
            int encodingBits = patternModuleSpec["encodingBits"].asInt();

            patternModule = new TwoConsecutiveOnesPatternModule(encodingBits);
          }
          else if (patternModuleName == "MaskingPattern")
          {
            int encodingBits = patternModuleSpec["encodingBits"].asInt();
            std::vector<int> maskingVector;
            maskingVector.resize(patternModuleSpec["maskingVector"].size());

            for (int k = 0; k < maskingVector.size(); k++)
              maskingVector[k] = patternModuleSpec["maskingVector"][k].asInt();

            int isZerosFrontHalf = 1;
            int isZerosBackHalf = 1;
            // zeros front half check
            for (int k = 0; k < maskingVector.size(); k++)
            {
              if ((k < maskingVector.size() / 2) && (maskingVector[k] != 0))
              {
                isZerosFrontHalf = 0;
                break;
              }
              else if ((k >= maskingVector.size() / 2) && (maskingVector[k] != 2))
              {
                isZerosFrontHalf = 0;
                break;
              }
            }

            // zeros back half check
            for (int k = 0; k < maskingVector.size(); k++)
            {
              if ((k < maskingVector.size() / 2) && (maskingVector[k] != 2))
              {
                isZerosBackHalf = 0;
                break;
              }
              else if ((k >= maskingVector.size() / 2) && (maskingVector[k] != 0))
              {
                isZerosBackHalf = 0;
                break;
              }
            }

            // instantiate modules
            if (isZerosFrontHalf == 1)
              patternModule = new ZerosFrontHalfPatternModule(encodingBits);
            else if (isZerosBackHalf == 1)
              patternModule = new ZerosBackHalfPatternModule(encodingBits);
            else
              patternModule = new MaskingPatternModule(encodingBits, maskingVector);
          }
          else if (patternModuleName == "UncompressedPattern")
          {
            int encodingBits = patternModuleSpec["encodingBits"].asInt();

            patternModule = new UncompressedPatternModule(encodingBits);
          }
          else
          {
            printf("""%s"" is not a valid pattern module. Check the config file.\n", patternModuleName.c_str());
            exit(1);
          }
          patternModules[j] = patternModule;
        }

        FPCModule *fpcModule = new FPCModule(patternModules);

        // PredCompModule
        compModule = new PredCompModule(m_LineSize, residueModule, bitplaneModule, xorModule, scanModule);
      }
      else if (moduleName == "AllZero")
      {
        // AllZeroModule
        compModule = new AllZeroModule(m_LineSize);
        (this->compressLine) = &VPC::compressLineOnlyAllZero;
      }
      else if (moduleName == "ByteplaneAllSame" || moduleName == "AllWordSame")
      {
        // AllWordSameModule
        compModule = new AllWordSameModule(m_LineSize);
        (this->compressLine) = &VPC::compressLineAllWordSame;
      }
      else
      {
        printf("\"%s\" is not a valid compression module. Check the config file.\n", moduleName.c_str());
        exit(1);
      }

      m_CompModules[i] = compModule;
    }
  }
//  static_cast<VPCResult*>(m_Stat)->SetNumModules(m_NumModules);
}

unsigned VPC::checkAllZeros(const int chosenCompModule, bool &isAllZeros, std::vector<uint8_t> &dataLine)
{
  isAllZeros = false;
  const unsigned uncompressedSize = dataLine.size() * BYTE;
  AllZeroModule *allZeroModule = static_cast<AllZeroModule*>(m_CompModules[chosenCompModule]);
  unsigned compressedSize = allZeroModule->CompressLine(dataLine);

  if (compressedSize == 0) 
  {
    isAllZeros = true;
    compressedSize += m_EncodingBits[chosenCompModule];
    static_cast<VPCResult*>(m_Stat)->Update(uncompressedSize, compressedSize, chosenCompModule);
  }

  return compressedSize;
}

unsigned VPC::checkAllWordSame(const int chosenCompModule, bool &isAllWordSame, std::vector<uint8_t> &dataLine)
{
  isAllWordSame = false;
  const unsigned uncompressedSize = dataLine.size() * BYTE;
  AllWordSameModule *allWordSameModule = static_cast<AllWordSameModule*>(m_CompModules[1]);
  unsigned compressedSize = allWordSameModule->CompressLine(dataLine);

  if (compressedSize == 4*BYTE)
  {
    isAllWordSame = true;
    compressedSize += m_EncodingBits[chosenCompModule];
    static_cast<VPCResult*>(m_Stat)->Update(uncompressedSize, compressedSize, chosenCompModule);
  }

  return compressedSize;
}

unsigned VPC::checkOtherPatterns(const int numStartingModule, std::vector<uint8_t> &dataLine)
{
  int chosenCompModule = -1;
  const unsigned uncompressedLineSize = dataLine.size() * BYTE;
  unsigned compressedLineSize = uncompressedLineSize;

  int numMaxScannedZRL = 0;
  Binary maxScanned;
  for (int i = numStartingModule; i < m_NumModules; i++)
  {
    PredCompModule *predCompModule = static_cast<PredCompModule*>(m_CompModules[i]);
    Binary scanned = predCompModule->CompressLine(dataLine, 0);

    // count zrl
    int numScannedZRL = 0;
    for (int row = 0; row <scanned.GetRowSize(); row++)
    {
      if (scanned.IsRowZeros(row))
        numScannedZRL++;
      else
        break;
    }

    if (numMaxScannedZRL <= numScannedZRL)
    {
      chosenCompModule = i;
      numMaxScannedZRL = numScannedZRL;
      maxScanned = scanned;
    }
  }

  int compressedSize = m_CommonEncoder.ProcessLine(maxScanned);
  if (compressedSize < uncompressedLineSize)
  {
    compressedLineSize = compressedSize;
  }
  else
  {
    chosenCompModule = -1;
    compressedLineSize = uncompressedLineSize;
  }
  compressedLineSize += m_EncodingBits[chosenCompModule];

  // update compression stat
  static_cast<VPCResult*>(m_Stat)->Update(uncompressedLineSize, compressedLineSize, chosenCompModule);
  // update residue stat
  updateResidueStat(dataLine, chosenCompModule);

  return compressedLineSize;
}

void VPC::updateResidueStat(std::vector<uint8_t> &dataLine, const int chosenCompModule)
{
  double mae = 0;
  double mse = 0;
  const int lineSize = dataLine.size();

  if (chosenCompModule != -1)
  {
    PredCompModule *predCompModule = static_cast<PredCompModule*>(m_CompModules[chosenCompModule]);
    mae = predCompModule->GetMAE(dataLine);
    mse = predCompModule->GetMSE(dataLine);
  }
  else
  {
    for (int i = 0; i < lineSize; i++)
    {
      uint8_t residue = dataLine[i];
      mae += abs((double)residue);
      mse += pow((double)residue, 2);
    }

    mae /= (lineSize);
    mse /= (lineSize);
  }

  static_cast<VPCResult*>(m_Stat)->UpdateResidueStat(mae, mse, chosenCompModule);
}

}
