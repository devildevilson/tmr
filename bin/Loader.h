#ifndef LOADER_H
#define LOADER_H

#include <string>

class Loader {
public:
  virtual ~Loader() {}
  
  virtual bool load(const std::string &name) = 0;
  virtual bool unload(const std::string &name) = 0;
  virtual void end() = 0;
  
  virtual void clear() = 0;
  
  virtual size_t overallState() const = 0;
  virtual size_t loadingState() const = 0;
  virtual std::string hint() const = 0;
};

#endif
