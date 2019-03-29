#ifndef FILETOOLS_H
#define FILETOOLS_H

#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <sstream>

//#include "Structures.h"

enum StringType {
  INFO,
  WARNING,
  ERROR
};

struct ConsoleLine {
  StringType type;
  std::string string;
};

namespace FileTools {
  std::vector<char> readFile(const std::string& filename);
  std::string getAppDir();
  void createLogFile(std::vector<ConsoleLine> history);
}

#endif
