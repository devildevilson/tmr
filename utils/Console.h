#ifndef CONSOLE_H
#define CONSOLE_H

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdarg.h>

#include "fmt/printf.h"

//#include "Structures.h"
#include "FileTools.h"

#define NOTIFY_TIME 500000
#define INPUT_CURSOR_TIME 500000

std::string trim(const std::string& str);

// struct Notify {
//   std::string string;
//   uint32_t    time  = 0;
//   Notify*     next  = nullptr;
  
//   float       x     = 0.0f;
//   float       y     = 0.0f;
//   TextAlign   align = TextAlign::LEFT;
// };

class Console {
public:
           Console();
           ~Console();
  
  void print(const std::stringstream &ss);
  void print(const std::string &s);
  
  void printW(const std::stringstream &ss);
  void printW(const std::string &s);
  
  void printE(const std::stringstream &ss);
  void printE(const std::string &s);
  
  template <typename... Args>
  void printf(const char* formatMessage, const Args & ... args) {
    ConsoleLine cl = {};
    cl.type = INFO;
    cl.string = fmt::sprintf(formatMessage, args...);
    cl.string = trim(cl.string);
    
    items.push_back(cl);
    fmt::print(cl.string);
    if (cl.string.at(cl.string.length()-1) != '\n') fmt::print("\n");
  }
  
  template <typename... Args>
  void printfW(const char* formatMessage, const Args & ... args) {
    ConsoleLine cl = {};
    cl.type = WARNING;
    cl.string = fmt::sprintf(formatMessage, args...);
    cl.string = trim(cl.string);
    
    items.push_back(cl);
    fmt::print(cl.string);
    if (cl.string.at(cl.string.length()-1) != '\n') fmt::print("\n");
  }
  
  template <typename... Args>
  void printfE(const char* formatMessage, const Args & ... args) {
    ConsoleLine cl = {};
    cl.type = ERROR;
    cl.string = fmt::sprintf(formatMessage, args...);
    cl.string = trim(cl.string);
    
    items.push_back(cl);
    fmt::print(cl.string);
    if (cl.string.at(cl.string.length()-1) != '\n') fmt::print("\n");
  }
  //void     centerPrintf(float textHeight, const char *formatMessage, ...);
  
  void addToHistory(const std::string &s);
  
  void     logWriteToFile();
  std::vector<ConsoleLine> getHistory();
  size_t getHistorySize();
  
  //  int textEditCallback(ImGuiTextEditCallbackData* data);
private:
  std::vector<std::string> history;
  std::vector<ConsoleLine> items;
  bool scrollToBottom;
  int32_t historyPos;    // -1: new line, 0..History.Size-1 browsing history.
};

#endif // CONSOLE_H
