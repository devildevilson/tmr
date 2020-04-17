#ifndef TARGETS_H
#define TARGETS_H

#include "target.h"
#include "RenderTarget.h"
#include "simd.h"

namespace devils_engine {
  namespace render {
    struct matrices;
    
    class deffered : public target, public yavf::RenderTarget {
    public:
      struct data {
        yavf::Image* color;
        yavf::Image* normal;
        yavf::Image* depth;
        yavf::DescriptorSet* desc;
        yavf::Framebuffer wall_buffer;
        yavf::Framebuffer next_buffer;
      };
      
      struct create_info {
        yavf::Device* device;
        uint32_t count;
        uint32_t width;
        uint32_t height;
      };
      deffered(const create_info &info);
      ~deffered();
      
      std::vector<VkClearValue> clearValues() const override;
      VkRect2D size() const override;
      yavf::RenderPass renderPass() const override;
      VkViewport viewport() const override;
      VkRect2D scissor() const override;
      yavf::Framebuffer framebuffer() const override;
      
      void recreate(const uint32_t &width, const uint32_t &height) override;
      
      void change_pass();
      void next_frame();
      
      yavf::RenderPass wall_renderpass() const;
      yavf::RenderPass next_renderpass() const;
      
      const data* current_frame_data() const;
      
      yavf::DescriptorSetLayout layout() const;
    private:
      void create_render_passes();
      
      yavf::Device* device;
      uint32_t width, height;
      uint32_t current_frame;
      uint32_t current_pass;
      yavf::RenderPass wall_pass;
      yavf::RenderPass next_pass;
      yavf::DescriptorSetLayout set_layout;
      std::vector<data> images;
    };
    
    struct images : public target {
      yavf::Device* device;
      yavf::Image light_output;
      yavf::Image tone_mapping_output;
      
      images(yavf::Device* device);
      ~images();
      void recreate(const uint32_t &width, const uint32_t &height) override;
    };
    
    struct buffers : public target {
      bool perspective;
      matrices* mat;
      yavf::Buffer uniform;
      yavf::Buffer matrix;
      
      buffers(yavf::Device* device);
      ~buffers();
      void recreate(const uint32_t &width, const uint32_t &height) override;
      
      void toggle_projection();
      bool projection_perspective() const;
      void set_persp(const simd::mat4 &proj);
      void set_ortho(const simd::mat4 &proj);
      void set_view(const simd::mat4 &view);
      void set_camera(const simd::vec4 &pos, const simd::vec4 &rot);
      void set_camera_dim(const uint32_t &width, const uint32_t &height);
      simd::mat4 get_view_proj() const;
      void update_data();
    };
  }
}

#endif
