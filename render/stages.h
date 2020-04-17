#ifndef STAGES_H
#define STAGES_H

#include <cstddef>

#include "stage.h"
#include "yavf.h"
#include "ArrayInterface.h"
#include "GPUArray.h"
#include "Utility.h"
#include "PhysicsTemporary.h"
#include "shared_structures.h"

namespace yavf {
  class Device;
}

namespace devils_engine {
  namespace render {
    class deffered;
    struct images;
    struct window;
    struct particles;
    
    class window_next_frame : public stage {
    public:
      struct create_info {
        window* w;
      };
      window_next_frame(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      window* w;
    };
    
//     class update_buffers : public stage {
//     public:
//       struct create_info {
//         yavf::Buffer* uniform;
//         yavf::Buffer* matrix;
//       };
//       update_buffers(const create_info &info);
//       ~update_buffers();
//       void begin() override;
//       void proccess(context* ctx) override;
//       void clear() override;
//     private:
//       bool perspective;
//       matrices* mat;
//       yavf::Buffer* uniform;
//       yavf::Buffer* matrix;
//     };
    
    class task_begin : public stage {
    public:
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    };
    
    class task_end : public stage {
    public:
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    };
    
    class gbuffer_begin : public stage {
    public:
      struct create_info {
        deffered* target;
      };
      gbuffer_begin(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      deffered* target;
    };
    
    class gbuffer_end : public stage {
    public:
      struct create_info {
        deffered* target;
      };
      gbuffer_end(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      deffered* target;
    };
    
    class post_begin : public stage {
    public:
      struct create_info {
        window* w;
      };
      post_begin(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      window* w;
    };
    
    class post_end : public stage {
    public:
      struct create_info {
        window* w;
      };
      post_end(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      window* w;
    };
    
    class monster_optimizer : public stage {
    public:
      static const uint32_t workgroup_size = 256;
      
      struct instance_data {
        glm::mat4 mat;
        image_data textureData;
        uint32_t dummy[1];
      };
      
      struct object_indices {
        uint32_t transform;
        uint32_t matrix;
        uint32_t rotation;
        uint32_t texture;
        float scale;
      };
      
      struct create_info {
        yavf::Device* device;
        yavf::Buffer* uniform;
        
        ArrayInterface<Transform>* transforms;
        ArrayInterface<simd::mat4>* matrices;
        ArrayInterface<image_data>* textures;
//         ArrayInterface<instance_data>* inst_datas_array;
      };
      monster_optimizer(const create_info &info);
      ~monster_optimizer();
      
      void add(const object_indices &idx);
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      
      uint32_t instances_count() const;
      GPUArray<instance_data>* instance_datas();
    private:
      yavf::Device* device;
      yavf::Pipeline pipe;
      yavf::Buffer* uniform;
      yavf::Buffer* obj_count;
      
      ArrayInterface<Transform>* transforms;
      ArrayInterface<simd::mat4>* matrices;
      ArrayInterface<image_data>* textures;
      
      GPUArray<instance_data> inst_datas_array;
      GPUArray<object_indices> indices;
      
      std::mutex mutex;
    };
    
    class geometry_optimizer : public stage {
    public:
      static const uint32_t workgroup_size = 256;
      
      struct instance_data {
    //     glm::mat4 mat; // добавится обязательно
        image_data textureData;
      };
      
      struct object_indices {
        uint32_t transform;
        uint32_t matrix;
        uint32_t rotation;
        uint32_t texture;
        
        uint32_t vertexOffset;
        uint32_t vertexCount;
        uint32_t faceIndex;
        uint32_t dummy;
      };
      
      struct create_info {
        yavf::Device* device;
        yavf::Buffer* uniform;
        
        ArrayInterface<Transform>* transforms;
        ArrayInterface<simd::mat4>* matrices;
        ArrayInterface<RotationData>* rotation_datas;
        ArrayInterface<image_data>* textures;
//         ArrayInterface<uint32_t>* indices_array;
//         ArrayInterface<instance_data>* instance_datas;
      };
      geometry_optimizer(const create_info &info);
      ~geometry_optimizer();
      
      void add(const object_indices &idx);
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      
      GPUArray<uint32_t>* indices();
      GPUArray<instance_data>* instances();
      uint32_t get_indices_count() const;
    private:
      yavf::Device* device;
      yavf::Pipeline pipe;
      
      yavf::Buffer* uniform;
      yavf::Buffer* obj_count;
      
      uint32_t face_count;
      uint32_t indices_count;
      
      ArrayInterface<Transform>* transforms;
      ArrayInterface<simd::mat4>* matrices;
      ArrayInterface<RotationData>* rotation_datas;
      ArrayInterface<image_data>* textures;
      
      GPUArray<uint32_t> indices_array;
      GPUArray<instance_data> instance_datas;
      GPUArray<object_indices> objs;
      
      std::mutex mutex;
    };
    
    class compute_particles : public stage {
    public:
      static const uint32_t workgroup_size = 256;
      
      struct create_info {
        yavf::Device* device;
        yavf::Buffer* uniform;
        yavf::Buffer* matrices;
        deffered* target;
        struct particles* particles;
      };
      compute_particles(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      yavf::Device* device;
      yavf::Buffer* uniform;
      yavf::Buffer* matrices;
      deffered* target;
      struct particles* particles;
      yavf::Pipeline pipe;
    };
    
    class monster_gbuffer : public stage, public pipeline_stage {
    public:
      struct create_info {
        yavf::Device* device;
        yavf::Buffer* uniform;
//         GPUArray<monster_optimizer::instance_data>* instance_datas;
        monster_optimizer* opt;
        deffered* target;
      };
      monster_gbuffer(const create_info &info);
      ~monster_gbuffer();
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      
      void recreate_pipelines(const game::image_resources_t* resource) override;
    private:
      yavf::Device* device;
      yavf::Pipeline pipe;
      yavf::Buffer* monster_default;
      yavf::Buffer* uniform;
      
      yavf::DescriptorSet* images_set;
      
//       GPUArray<monster_optimizer::instance_data>* instance_datas;
      monster_optimizer* opt;
      deffered* target;
    };
    
    class geometry_gbuffer : public stage, public pipeline_stage {
    public:
      struct create_info {
        yavf::Device* device;
        yavf::Buffer* uniform;
        yavf::Buffer* world_map_vertex;
        geometry_optimizer* opt;
        deffered* target;
      };
      geometry_gbuffer(const create_info &info);
      ~geometry_gbuffer();
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      
      void recreate_pipelines(const game::image_resources_t* resource) override;
    private:
      yavf::Device* device;
      yavf::Buffer* uniform;
      yavf::Buffer* world_map_vertex;
      yavf::Pipeline pipe;
      yavf::DescriptorSet* images_set;
      geometry_optimizer* opt;
      deffered* target;
    };
    
    class particles_gbuffer : public stage, public pipeline_stage {
    public:
      struct create_info {
        yavf::Device* device;
        yavf::Buffer* uniform;
        deffered* target;
        struct particles* particles;
      };
      particles_gbuffer(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      void recreate_pipelines(const game::image_resources_t* resource) override;
    private:
      yavf::Device* device;
      yavf::Buffer* uniform;
      deffered* target;
      struct particles* particles;
      yavf::Pipeline pipe;
      yavf::DescriptorSet* images_set;
    };
    
    // может ли быть совмещено с lights_deffered? скорее всего да
    class lights_optimizer : public stage {
    public:
      static const uint32_t workgroup_size = 16;
      
      struct light_data {
        render::light_data light_data;
        uint32_t transform_index;
      };
      
      struct create_info {
        yavf::Device* device;
        yavf::Buffer* uniform;
        yavf::Buffer* matrix;
        ArrayInterface<Transform>* transforms;
        images* data;
        deffered* target;
      };
      lights_optimizer(const create_info &info);
      ~lights_optimizer();
      void add(const light_data &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      yavf::Device* device;
      yavf::Buffer* uniform;
      yavf::Buffer* matrix;
      
      ArrayInterface<Transform>* transforms;
      GPUArray<render::light_data> light_array;
      
      yavf::Pipeline pipe;
      images* data;
      deffered* target;
      
      std::mutex mutex;
    };
    
    class tone_mapping : public stage {
    public:
      static const uint32_t workgroup_size = 16;
      
      struct create_info {
        yavf::Device* device;
        images* data;
      };
      tone_mapping(const create_info &info);
      ~tone_mapping();
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      yavf::Device* device;
      yavf::Pipeline pipe;
      images* data;
    };
    
    class copy : public stage {
    public:
      struct create_info {
        images* data;
        deffered* target;
        window* w;
        yavf::Image* early_screenshot_img;
      };
      copy(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      void do_screenshot();
    private:
      bool screenshot;
      images* data;
      deffered* target;
      window* w;
      yavf::Image* early_screenshot_img;
    };
    
    // для гуи отдельный begin, end? да
    class gui : public stage, public pipeline_stage {
    public:
      struct create_info {
        yavf::Device* device;
        window* w;
        yavf::DescriptorSet* image_set;
      };
      gui(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      
      void recreate_pipelines(const game::image_resources_t* resource) override;
    private:
      yavf::Device* device;
      window* w;
      yavf::Pipeline pipe;
      yavf::Buffer vertex_gui;
      yavf::Buffer index_gui;
      yavf::Buffer matrix;
      yavf::DescriptorSet* image_set;
    };
    
    class debug_monster : public stage {
    public:
      
    };
    
    class debug_geometry : public stage {
    public:
      
    };
    
    class task_start : public stage {
    public:
      task_start(yavf::Device* device);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      void wait();
    private:
      yavf::Device* device;
      yavf::Internal::Queue wait_fence;
    };
    
    class window_present : public stage {
    public:
      struct create_info {
        window* w;
      };
      window_present(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      window* w;
    };
  }
}

#endif
