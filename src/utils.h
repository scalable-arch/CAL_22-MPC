#ifndef __UTILS_H__
#define __UTILS_H__

#include <fstream>
#include <string>

#include <strutil.h>
#include <fmt/core.h>

bool isFileExists(const std::string &filePath);
std::string parseConfig(const std::string &configPath);

#endif  // __UTILS_H__
