#include "utils.h"

bool isFileExists(const std::string &filePath)
{
  std::ifstream f(filePath);
  return f.good();
}

