#ifndef LOGGING_H
#define LOGGING_H

#include <chrono>
#include <string>
#include <iostream>

#include "shared_time_constant.h"

class RegionLog {
public:
#ifdef _DEBUG
  inline RegionLog(const std::string &name, const bool printStart = false) : tp(std::chrono::steady_clock::now()), name(name) {
    if (printStart) std::cout << name << " start" << "\n";
  }
  inline ~RegionLog() {
    const auto end = std::chrono::steady_clock::now() - tp;
    const auto mcs = std::chrono::duration_cast<CHRONO_TIME_TYPE>(end).count();

    std::cout << name << " takes " << mcs << " " << TIME_STRING << "\n";
  }

  std::chrono::steady_clock::time_point tp;
  std::string name;
#else
  inline RegionLog(const std::string &name, const bool printStart = false) {
    (void)name;
    (void)printStart;
  }
#endif
};

#endif
