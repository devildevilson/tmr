#include "targets.h"

#include "yavf.h"
#include "shared_structures.h"

namespace devils_engine {
  namespace render {
    enum current_render_pass {
      RENDER_PASS_WALL,
      RENDER_PASS_OBJECTS,
      RENDER_PASS_COUNT
    };
    
    deffered::deffered(const create_info &info) : device(info.device), width(info.width), height(info.height), current_frame(0), current_pass(RENDER_PASS_WALL), images(info.count, {nullptr, nullptr, nullptr, nullptr, VK_NULL_HANDLE, VK_NULL_HANDLE}) {
      VkFormat depth = yavf::findSupportedFormat(device->physicalHandle(),
                                                 {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                                 VK_IMAGE_TILING_OPTIMAL,
                                                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
      
      {
        yavf::DescriptorLayoutMaker dlm(device);
        
        set_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
                        .binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
                        .binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT).create("compute_set_layout");
      }
      
      yavf::DescriptorMaker dm(device);
      VkDescriptorPool pool = device->descriptorPool("default_descriptor_pool");
    //   yavf::DescriptorUpdater du(device);
      
      yavf::SamplerMaker sm(device);
      yavf::Sampler s = sm.filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
                          .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                          .anisotropy(VK_FALSE)
                          .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
                          .create("deferred_sampler_name");
                          
      for (uint32_t i = 0; i < images.size(); ++i) {
        images[i].desc = dm.layout(set_layout).create(pool)[0];
        
        {
          images[i].color = device->create(yavf::ImageCreateInfo::texture2D({width, height}, 
                                                                                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), 
                                                      VMA_MEMORY_USAGE_GPU_ONLY);
          yavf::ImageView* view = images[i].color->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
          images[i].desc->add({s, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
        }
        
        {
          images[i].normal = device->create(yavf::ImageCreateInfo::texture2D({width, height}, 
                                                                                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                                                      VK_FORMAT_R16G16_UNORM), 
                                                      VMA_MEMORY_USAGE_GPU_ONLY);
          yavf::ImageView* view = images[i].normal->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
          images[i].desc->add({s, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
        }
        
        {
          images[i].depth = device->create(yavf::ImageCreateInfo::texture2D({width, height}, 
                                                                                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                                                      depth), 
                                                      VMA_MEMORY_USAGE_GPU_ONLY);
          yavf::ImageView* view = images[i].depth->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});
          images[i].desc->add({s, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
        }

        images[i].desc->update();
      }
      
      create_render_passes();
      
      for (uint32_t i = 0; i < images.size(); ++i) {
        const std::vector<VkImageView> views = {
          images[i].color->view()->handle(),
          images[i].normal->view()->handle(),
          images[i].depth->view()->handle()
        };
        images[i].wall_buffer = device->create(yavf::FramebufferCreateInfo::framebuffer(wall_pass, views.size(), views.data(), width, height), "deferred_wall_framebuffer_"+std::to_string(i));
        images[i].next_buffer = device->create(yavf::FramebufferCreateInfo::framebuffer(next_pass, views.size(), views.data(), width, height), "deferred_next_framebuffer_"+std::to_string(i));
      }
    }
    
    deffered::~deffered() {
      for (auto &img : images) {
        device->destroy(img.wall_buffer);
        device->destroy(img.next_buffer);
        device->destroy(img.color);
        device->destroy(img.depth);
        device->destroy(img.normal);
      }
      
      device->destroy(set_layout);
      device->destroy(wall_pass);
      device->destroy(next_pass);
    }
    
    std::vector<VkClearValue> deffered::clearValues() const {
      return {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0}};
    }
    
    VkRect2D deffered::size() const {
      const VkRect2D r{
        {0, 0},
        {width, height}
      };
      
      return r;
    }
    
    yavf::RenderPass deffered::renderPass() const {
      if (current_pass == RENDER_PASS_WALL) return wall_pass;
      return next_pass;
    }
    
    VkViewport deffered::viewport() const {
      const VkViewport port{
        0.0f, 0.0f, 
        static_cast<float>(width),
        static_cast<float>(height),
        0.0f, 1.0f
      };
      
      return port;
    }
    
    VkRect2D deffered::scissor() const {
      const VkRect2D r{
        {0, 0},
        {width, height}
      };
      
      return r;
    }
    
    yavf::Framebuffer deffered::framebuffer() const {
      if (current_pass == RENDER_PASS_WALL) return images[current_frame].wall_buffer;
      return images[current_frame].next_buffer;
    }
    
    void deffered::recreate(const uint32_t &width, const uint32_t &height) {
      if (this->width == width && this->height == height) return;
      this->width = width;
      this->height = height;
      
      for (uint32_t i = 0; i < images.size(); ++i) {
        images[i].color->recreate({width, height, 1});
        images[i].normal->recreate({width, height, 1});
        images[i].depth->recreate({width, height, 1});
        
        images[i].desc->update();
        
        device->destroy(images[i].wall_buffer);
        device->destroy(images[i].next_buffer);
        
        const std::vector<VkImageView> views = {
          images[i].color->view()->handle(),
          images[i].normal->view()->handle(),
          images[i].depth->view()->handle()
        };
        images[i].wall_buffer = device->create(yavf::FramebufferCreateInfo::framebuffer(wall_pass, views.size(), views.data(), width, height), "deferred_wall_framebuffer_"+std::to_string(i));
        images[i].next_buffer = device->create(yavf::FramebufferCreateInfo::framebuffer(next_pass, views.size(), views.data(), width, height), "deferred_next_framebuffer_"+std::to_string(i));
      }
    }
    
    void deffered::change_pass() {
      current_pass = (current_pass+1)%RENDER_PASS_COUNT;
    }
    
    void deffered::next_frame() {
      current_frame = (current_frame+1)%images.size();
    }
    
    yavf::RenderPass deffered::wall_renderpass() const {
      return wall_pass;
    }
    
    yavf::RenderPass deffered::next_renderpass() const {
      return next_pass;
    }
    
    const deffered::data* deffered::current_frame_data() const {
      return &images[current_frame];
    }
    
    yavf::DescriptorSetLayout deffered::layout() const {
      return set_layout;
    }
    
    void deffered::create_render_passes() {
      yavf::RenderPassMaker rpm(device);
  
      wall_pass = rpm.attachmentBegin(images[0].color->info().format)
                      .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                      .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                      .attachmentInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                      .attachmentFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                      //.attachmentSamples(VK_SAMPLE_COUNT_8_BIT)
                    .attachmentBegin(images[0].normal->info().format)
                      .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                      .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                      .attachmentInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                      .attachmentFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                      //.attachmentSamples(VK_SAMPLE_COUNT_1_BIT)
                    .attachmentBegin(images[0].depth->info().format)
                      .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                      .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                      .attachmentInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                      .attachmentFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                      //.attachmentSamples(VK_SAMPLE_COUNT_1_BIT)
                    .subpassBegin(VK_PIPELINE_BIND_POINT_GRAPHICS)
                      .subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0)
                      .subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1)
                      .subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 2)
                    .dependencyBegin(VK_SUBPASS_EXTERNAL, 0)
                      .dependencySrcStageMask(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
                      .dependencyDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                      .dependencySrcAccessMask(VK_ACCESS_MEMORY_READ_BIT)
                      .dependencyDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                    .dependencyBegin(0, VK_SUBPASS_EXTERNAL)
                      .dependencySrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                      .dependencyDstStageMask(VK_PIPELINE_BIND_POINT_COMPUTE)
                      .dependencySrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                      .dependencyDstAccessMask(VK_ACCESS_MEMORY_READ_BIT)
                    .create("deferred_wall_render_pass");
                    
       next_pass = rpm.attachmentBegin(images[0].color->info().format)
                      .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
                      .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                      .attachmentInitialLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                      .attachmentFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                      //.attachmentSamples(VK_SAMPLE_COUNT_8_BIT)
                    .attachmentBegin(images[0].normal->info().format)
                      .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
                      .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                      .attachmentInitialLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                      .attachmentFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                      //.attachmentSamples(VK_SAMPLE_COUNT_1_BIT)
                    .attachmentBegin(images[0].depth->info().format)
                      .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
                      .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                      .attachmentInitialLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                      .attachmentFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                      //.attachmentSamples(VK_SAMPLE_COUNT_1_BIT)
                    .subpassBegin(VK_PIPELINE_BIND_POINT_GRAPHICS)
                      .subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0)
                      .subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1)
                      .subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 2)
                    .dependencyBegin(VK_SUBPASS_EXTERNAL, 0)
                      .dependencySrcStageMask(VK_PIPELINE_BIND_POINT_COMPUTE)
                      .dependencyDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                      .dependencySrcAccessMask(VK_ACCESS_MEMORY_READ_BIT)
                      .dependencyDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                    .dependencyBegin(0, VK_SUBPASS_EXTERNAL)
                      .dependencySrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                      .dependencyDstStageMask(VK_PIPELINE_BIND_POINT_COMPUTE)
                      .dependencySrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                      .dependencyDstAccessMask(VK_ACCESS_MEMORY_READ_BIT)
                    .create("deferred_next_render_pass");
                    
    }
    
    images::images(yavf::Device* device) : 
      device(device), 
      light_output(device, yavf::ImageCreateInfo::texture2D({1280, 720}, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_GPU_ONLY), 
      tone_mapping_output(device, yavf::ImageCreateInfo::texture2D({1280, 720}, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_GPU_ONLY) 
    {
      yavf::DescriptorSetLayout storage_image_layout = device->setLayout("storage_image_layout");
      {
        yavf::DescriptorLayoutMaker dlm(device);
        
        if (storage_image_layout == VK_NULL_HANDLE) {
          storage_image_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT).create("storage_image_layout");
        }
      }
      
      yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
      {
        yavf::DescriptorMaker dm(device);
        
        yavf::ImageView* view = light_output.createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
        yavf::DescriptorSet* d2 = dm.layout(storage_image_layout).create(pool)[0];
        const size_t i2 = d2->add({VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_GENERAL, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE});
        view->setDescriptor(d2, i2);
      }
      
      {
        yavf::DescriptorMaker dm(device);
        
        yavf::ImageView* view = tone_mapping_output.createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
        yavf::DescriptorSet* d2 = dm.layout(storage_image_layout).create(pool)[0];
        const size_t i2 = d2->add({VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_GENERAL, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE});
        view->setDescriptor(d2, i2);
      }
    }
    
    images::~images() {}
    void images::recreate(const uint32_t &width, const uint32_t &height) {
      if (light_output.info().extent.width == width && light_output.info().extent.height == height) return;
      light_output.recreate({width, height, 1});
      tone_mapping_output.recreate({width, height, 1});
    }
    
    buffers::buffers(yavf::Device* device) : perspective(true), mat(new matrices),
      uniform(device, yavf::BufferCreateInfo::buffer(sizeof(camera_data), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY),
      matrix(device, yavf::BufferCreateInfo::buffer(sizeof(matrices_data), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY)
    {
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      yavf::DescriptorPool defaultPool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
      {
        yavf::DescriptorMaker dm(device);
        
        yavf::DescriptorSet* d2 = dm.layout(uniform_layout).create(defaultPool)[0];
        const size_t i2 = d2->add({&uniform, 0, uniform.info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
        uniform.setDescriptor(d2, i2);
      }
      
      {
        yavf::DescriptorMaker dm(device);
        
        yavf::DescriptorSet* d2 = dm.layout(uniform_layout).create(defaultPool)[0];
        const size_t i2 = d2->add({&matrix, 0, matrix.info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
        matrix.setDescriptor(d2, i2);
      }
      
      (void)storage_layout;
    }
    
    buffers::~buffers() { delete mat; }
    
    void buffers::recreate(const uint32_t &width, const uint32_t &height) { (void)width; (void)height; }
    void buffers::toggle_projection() { perspective = !perspective; }
    bool buffers::projection_perspective() const { return perspective; }
    void buffers::set_persp(const simd::mat4 &proj) { mat->persp = proj; }
    void buffers::set_ortho(const simd::mat4 &proj) { mat->ortho = proj; }
    void buffers::set_view(const simd::mat4 &view) { mat->view = view; }
    void buffers::set_camera(const simd::vec4 &pos, const simd::vec4 &rot) { mat->camera.pos = pos; mat->camera.dir = rot; }
    void buffers::set_camera_dim(const uint32_t &width, const uint32_t &height) { mat->camera.width = width; mat->camera.height = height; }
    simd::mat4 buffers::get_view_proj() const { return mat->camera.viewproj.get_simd(); }
    void buffers::update_data() {
      if (perspective) {
        mat->matrixes.proj = mat->persp;
      } else {
        mat->matrixes.proj = mat->ortho;
      }

      mat->camera.viewproj = mat->matrixes.proj.get_simd() * mat->view.get_simd();
      mat->camera.view = mat->view;
      mat->matrixes.view = mat->view;
      mat->matrixes.invView = simd::inverse(mat->view.get_simd());
      mat->matrixes.invProj = simd::inverse(mat->matrixes.proj.get_simd());
      mat->matrixes.invViewProj = simd::inverse(mat->camera.viewproj.get_simd());

      memcpy(uniform.ptr(), &mat->camera, sizeof(camera_data));
      memcpy(matrix.ptr(), &mat->matrixes, sizeof(matrices_data));
    }
  }
}
