#include <fstream>
#include <ios>
#include <iostream>
#include <cassert>

#include <fmt/core.h>
#include <cxxopts.hpp>
#include <strutil.h>

#include "compressor/Compressor.h"
#include "compressor/CCC.h"
#include "loader/LoaderGPGPU.h"
#include "loader/LoaderNPY.h"

comp::CCCResult compressLines(comp::Compressor *compressor, trace::Loader *loader);
std::string parseConfig(const std::string &configPath);

int main(int argc, char **argv)
{
  std::string tracePath;
  std::string configPath;
  std::string outputDirPath;
  
  // parse arguments
  cxxopts::Options options("compressor");

  options.add_options()
    ("i,input",     "Input GPGPU-Sim trace file path (.log)", cxxopts::value<std::string>())
    ("c,config",    "Config file path (.json)", cxxopts::value<std::string>())
    ("o,output",    "Output directory path", cxxopts::value<std::string>())
    ("h,help",      "Print usage");
  auto args = options.parse(argc, argv);

  int help = args.count("help");
  if (args.count("input"))
    tracePath = args["input"].as<std::string>();
  else
    help = 1;
  if (args.count("config"))
    configPath = args["config"].as<std::string>();
  else
    help = 1;
  if (args.count("output"))
    outputDirPath = args["output"].as<std::string>();
  else
    outputDirPath = "";

  // help message
  if (help)
  {
    std::cout << options.help() << std::endl;
    exit(0);
  }

  // instantiates loader
  trace::Loader *loader;
  if (strutil::ends_with(tracePath, ".log"))
    loader = new trace::LoaderGPGPU(tracePath);
  else if (strutil::ends_with(tracePath, ".npy"))
    loader = new trace::LoaderNPY(tracePath);
  else
    assert(false && "Unsupported extension.");

  // instantiates compressor
  comp::Compressor *compressor;
  std::string compressorName;
  int numCompModules;

  compressor = new comp::CCC(configPath);
  compressorName = compressor->GetCompressorName();
  numCompModules = static_cast<comp::CCC*>(compressor)->GetNumModules();

  // results file
  std::string saveFileName = parseConfig(configPath);
  std::string compOutputSavePath = outputDirPath + fmt::format("/{}_results.csv", saveFileName);;
  std::string compDetailedOutputSavePath = outputDirPath + fmt::format("/{}_results_detail.csv", saveFileName);

  // compress
  comp::CCCResult compStat = compressLines(compressor, loader);

  // print
  std::cout << fmt::format("compratio: {}", compStat.CompRatio) << std::endl;
  compStat.Print(tracePath, compOutputSavePath);
  compStat.PrintHistogram(tracePath, compDetailedOutputSavePath);

  delete loader;
  delete compressor;
  return 0;
}

comp::CCCResult compressLines(comp::Compressor *compressor, trace::Loader *loader)
{
  // init MemReqGPU_t
  trace::MemReq_t *memReq = new trace::MemReqGPU_t;

  // compress
  while (1)
  {
    memReq = static_cast<trace::MemReqGPU_t*>(loader->GetLine(memReq));
    if (memReq->isEnd) break;
    if (static_cast<trace::MemReqGPU_t*>(memReq)->reqType == trace::GLOBAL_ACC_R
        || static_cast<trace::MemReqGPU_t*>(memReq)->reqType == trace::GLOBAL_ACC_W)
    {
      std::vector<uint8_t> &dataLine = memReq->data;
      compressor->CompressLine(dataLine);
    }
  }
  comp::CCCResult &CCCstat = static_cast<comp::CCC*>(compressor)->m_Stat;

  return CCCstat;
}

std::string parseConfig(const std::string &configPath)
{
  std::vector<std::string> parsedBySlash = strutil::split(configPath, "/");
//  std::vector<std::string> parsedBySlash = (configPath, "/");

  std::string cfgFileName = parsedBySlash[parsedBySlash.size() - 1];
  if (!strutil::replace_all(cfgFileName, ".json", ""))
  {
    fmt::print("String parsing have failed!\n");
    exit(1);
  }
//  std::string parsedString = eraseSubString(cfgFileName, ".json");

  return cfgFileName;
}

