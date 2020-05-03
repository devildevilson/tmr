#ifndef DECAL_SYSTEM_H
#define DECAL_SYSTEM_H

#include "Utility.h"
#include <vector>
#include "ArrayInterface.h"
#include "PhysicsTemporary.h"
#include "shared_structures.h"
#include "Physics.h"

// короче существует подходящий алгоритм для декалей
// который используют по сей день
// https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm
// осталось только понять как сделать uv координаты
// 3.05.2020 декали сделаны почти полностью (их уже можно использовать вполне)
// скорее всего проблемы возникнут на стыках нескольких стен
// и на рампах
// systems::decals не многопоточный!

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace core {
    struct state_t;
  }
  
  namespace systems {
    class decals {
    public:
      static Container<simd::mat4>* matrix;
      static Container<Transform>* transforms;
      
      struct pending_data {
        basic_vec4 pos;
        basic_mat4 orn; // может лучше передавать просто нормаль?
        float scale;
        float size;
        
        //render::image image;
        const core::state_t* state;
      };
      
      decals();
      ~decals();
      
      void update(const size_t &time, const size_t &update_delta);
      
      yacs::entity* add(const pending_data &data);
      void remove(yacs::entity* decal);
    private:
      struct computed_decal {
        yacs::entity* decal;
        uint32_t matrix;
      };
      
      struct pending_decal {
        pending_data data;
        PhysicsIndexContainer container;
        yacs::entity* decal;
        uint32_t matrix;
        uint32_t transform_index;
      };
      
      size_t current_time;
      std::vector<computed_decal> computed_decals;
      std::vector<pending_decal> pending_decals;
      
      void clip(const std::vector<simd::vec4> &inVerts, std::vector<simd::vec4> &outVerts, const simd::vec4 &normal, const float &dist);
      void clip(std::vector<simd::vec4> &inVerts, std::vector<simd::vec4> &outVerts, const std::vector<std::pair<simd::vec4, simd::vec4>> &clip_poly);
      
      void compute_polygon(const PhysicsIndexContainer* obj_ptr, const uint32_t &points_count, const simd::vec4* points_arr, const simd::vec4 &normal, const simd::vec4 &center, pending_decal &decal);
      void compute_complex(const PhysicsIndexContainer* obj_ptr, pending_decal &decal);
    };
  }
}

#endif
