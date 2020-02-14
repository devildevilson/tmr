#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <vector>
#include <atomic>

namespace devils_engine {
  namespace utils {
    class id {
    public:
      static id get(const std::string &name);
      
      id();
      
      std::string name() const;
      
      bool operator==(const id &other) const;
      bool operator!=(const id &other) const;
      bool operator>(const id &other) const;
      bool operator<(const id &other) const;
      bool operator>=(const id &other) const;
      bool operator<=(const id &other) const;
    private:
      size_t m_id;
      
      //static std::atomic<size_t> current_id;
      static std::vector<std::string> names;
    };
  }
}

#endif
