#include "utils.h"

bool isFileExists(const std::string &filePath)
{
  std::ifstream f(filePath);
  return f.good();
}

std::string parseConfig(const std::string &configPath)
{
  std::vector<std::string> parsedBySlash = strutil::split(configPath, "/");

  std::string cfgFileName = parsedBySlash[parsedBySlash.size() - 1];
  if (!strutil::replace_all(cfgFileName, ".json", ""))
  {
    fmt::print("String parsing have failed!\n");
    exit(1);
  }

  return cfgFileName;
}

