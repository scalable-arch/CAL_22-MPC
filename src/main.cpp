#include <fstream>
#include <ios>
#include <iostream>
#include <cassert>

#include <fmt/core.h>
#include <cxxopts.hpp>
#include <strutil.h>

#include "compressor/VPC.h"
#include "compressor/FPC.h"
#include "compressor/BDI.h"
#include "compressor/CPACK.h"
#include "compressor/Pattern.h"
#include "compressor/BPC.h"

#include "loader/LoaderGPGPU.h"
#include "loader/LoaderNPY.h"

comp::CompResult* compressLines(comp::Compressor *compressor, trace::Loader *loader);

int main(int argc, char **argv)
{
  std::string algorithm;
  std::string tracePath;
  std::string configPath;
  std::string outputDirPath;
  
  // parse arguments
  {
  cxxopts::Options options("Compressor");

  options.add_options()
    ("a,algorithm", "Compression algorithm [VPC/FPC/BDI/BPC/CPACK/PATTERN]. Default=VPC", cxxopts::value<std::string>())
    ("i,input",     "Input GPGPU-Sim trace file path. Supported extensions: .log, .npy", cxxopts::value<std::string>())
    ("c,config",    "Config file path (.json).", cxxopts::value<std::string>())
    ("o,output",    "Output directory path", cxxopts::value<std::string>())
    ("h,help",      "Print usage");
  auto args = options.parse(argc, argv);

  int help = args.count("help");
  if (args.count("algorithm"))
    algorithm = args["algorithm"].as<std::string>();
  else
    algorithm = "VPC";
  if (args.count("input"))
    tracePath = args["input"].as<std::string>();
  else
    help = 1;
  if (algorithm == "VPC")
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
  }

  // instantiates loader
  trace::Loader *loader;
  if (strutil::ends_with(tracePath, ".log"))
    loader = new trace::gpgpusim::LoaderGPGPU(tracePath);
  else if (strutil::ends_with(tracePath, ".npy"))
    loader = new trace::LoaderNPY(tracePath);
  else if (strutil::ends_with(tracePath, ".txt"))
    loader = new trace::samsung::LoaderGPGPU(tracePath);
  else
    assert(false && "Unsupported extension.");

  // instantiates compressor
  const unsigned lineSize = loader->GetLineSize();
  comp::Compressor *compressor;
  if (algorithm == "VPC")
    compressor = new comp::VPC(configPath);
  else if (algorithm == "FPC")
    compressor = new comp::FPC(lineSize);
  else if (algorithm == "BDI")
    compressor = new comp::BDI(lineSize);
  else if (algorithm == "BPC")
    compressor = new comp::BPC(lineSize);
  else if (algorithm == "CPACK")
    compressor = new comp::CPACK(lineSize);
  else if (algorithm == "PATTERN")
    compressor = new comp::Pattern(lineSize);
  else
    assert(false && "Invalid name of algorithm.");

  // results file
  std::string saveFileName;
  if (algorithm == "VPC")
    saveFileName = parseConfig(configPath);
  else
    saveFileName = algorithm;
  std::string compOutputSavePath = outputDirPath + fmt::format("/{}_results.csv", saveFileName);;
  std::string compDetailedOutputSavePath = outputDirPath + fmt::format("/{}_results_detail.csv", saveFileName);

  // compress
  comp::CompResult *compStat = compressLines(compressor, loader);

  // print
  std::string workloadName;
  {
    std::vector<std::string> splitTracePath = strutil::split(tracePath, "/");
    workloadName = splitTracePath[splitTracePath.size() - 1];
    strutil::replace_all(workloadName, ".npy", "");
  }
  std::cout << fmt::format("comp.ratio: {}", compStat->CompRatio) << std::endl;

  if (algorithm == "VPC")
  {
    comp::VPCResult *stat = static_cast<comp::VPCResult*>(compStat);
    stat->Print(workloadName, compOutputSavePath);
    stat->PrintDetail(workloadName, compDetailedOutputSavePath);
  }
  else if (algorithm == "FPC")
  {
    comp::FPCResult *stat = static_cast<comp::FPCResult*>(compStat);
    stat->Print(workloadName, compOutputSavePath);
    stat->PrintDetail(workloadName, compDetailedOutputSavePath);
  }
  else if (algorithm == "BDI")
  {
    comp::BDIResult *stat = static_cast<comp::BDIResult*>(compStat);
    stat->Print(workloadName, compOutputSavePath);
    stat->PrintDetail(workloadName, compDetailedOutputSavePath);
  }
  else if (algorithm == "PATTERN")
  {
    comp::PatternResult *stat = static_cast<comp::PatternResult*>(compStat);
    stat->Print(workloadName, compOutputSavePath);
    stat->PrintDetail(workloadName, compDetailedOutputSavePath);
  }
  else 
  {
		comp::CompResult *stat = compStat;
    stat->Print(workloadName, compOutputSavePath);
    stat->PrintDetail(workloadName, compDetailedOutputSavePath);
  }

  delete loader;
  delete compressor;
  return 0;
}

comp::CompResult* compressLines(comp::Compressor *compressor, trace::Loader *loader)
{
  // check which loader is passed,
  // and init MemReq_t
  trace::MemReq_t *memReq;
  if (dynamic_cast<trace::gpgpusim::LoaderGPGPU*>(loader) != nullptr)
    memReq = new trace::gpgpusim::MemReqGPU_t;
  else if (dynamic_cast<trace::samsung::LoaderGPGPU*>(loader) != nullptr)
    memReq = new trace::samsung::MemReqGPU_t;
  else if (dynamic_cast<trace::LoaderNPY*>(loader) != nullptr)
    memReq = new trace::MemReq_t;

  // compress
  while (1)
  {
    memReq = loader->GetLine(memReq);
    if (memReq->isEnd) break;
    std::vector<uint8_t> &dataLine = memReq->data;
    compressor->CompressLine(dataLine);
  }

  comp::CompResult *compStat = compressor->GetResult();
  return compStat;
}
