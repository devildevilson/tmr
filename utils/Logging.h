#ifndef LOGGING_H
#define LOGGING_H

#include <chrono>
#include <string>
#include <iostream>

class RegionLog {
public:
#ifdef _DEBUG
  inline RegionLog(const std::string &name, const bool printStart = false) : tp(std::chrono::steady_clock::now()), name(name) {
    if (printStart) std::cout << name << " start" << "\n";
  }
  inline ~RegionLog() {
    auto end = std::chrono::steady_clock::now() - tp;
    auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();

    std::cout << name << " takes " << mcs << " mcs" << "\n";
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
