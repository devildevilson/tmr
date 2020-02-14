#ifndef SYSTEM_H
#define SYSTEM_H

#include "Engine.h"
#include "Globals.h"
#include "EntityComponentSystem.h"
#include "ThreadPool.h"

namespace devils_engine {
  namespace utils {
    template <typename T>
    class system : public Engine {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      system(const create_info &info) : pool(info.pool) {}
      void update(const size_t &time) override {
        const size_t count = Global::get<yacs::world>()->count_components<T>();
        const size_t one_jobs = std::ceil(float(count)/float(pool->size()+1));
        size_t start = 0;
        for (size_t i = 0; i < pool->size()+1; ++i) {
          const size_t job_count = std::min(one_jobs, count-start);
          if (job_count == 0) break;
          
          pool->submitbase([start, job_count, time] () {
            for (size_t i = start; i < start+job_count; ++i) {
              auto states = Global::get<yacs::world>()->get_component<T>(i);
              states->update(time);
            }
          });
          
          start += job_count;
        }
        
        pool->compute();
        pool->wait();
      }
    private:
      dt::thread_pool* pool;
    };
    
    template <typename T>
    void update(dt::thread_pool* pool, const size_t &time) {
      const size_t count = Global::get<yacs::world>()->count_components<T>();
      const size_t one_jobs = std::ceil(float(count)/float(pool->size()+1));
      size_t start = 0;
      for (size_t i = 0; i < pool->size()+1; ++i) {
        const size_t job_count = std::min(one_jobs, count-start);
        if (job_count == 0) break;
        
        pool->submitbase([start, job_count, time] () {
          for (size_t i = start; i < start+job_count; ++i) {
            auto comp = Global::get<yacs::world>()->get_component<T>(i);
            comp->update(time);
          }
        });
        
        start += job_count;
      }
      
      pool->compute();
      pool->wait();
    }
    
    template <typename T>
    void update(dt::thread_pool* pool) {
      const size_t count = Global::get<yacs::world>()->count_components<T>();
      const size_t one_jobs = std::ceil(float(count)/float(pool->size()+1));
      size_t start = 0;
      for (size_t i = 0; i < pool->size()+1; ++i) {
        const size_t job_count = std::min(one_jobs, count-start);
        if (job_count == 0) break;
        
        pool->submitbase([start, job_count] () {
          for (size_t i = start; i < start+job_count; ++i) {
            auto comp = Global::get<yacs::world>()->get_component<T>(i);
            comp->update();
          }
        });
        
        start += job_count;
      }
      
      pool->compute();
      pool->wait();
    }
  }
}

#endif
