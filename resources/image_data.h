#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <cstdint>
#include "shared_structures.h"
#include "id.h"
#include "resource_container.h"
#include "ImageResourceContainer.h"

namespace devils_engine {
  namespace render {
    namespace utils {
      struct extent_2d {
        uint32_t width;
        uint32_t height;
      };
      
      struct extent_3d {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
      };
      
      struct offset_2d {
        uint32_t x;
        uint32_t y;
      };
    }
  }
  
  namespace core {
    struct image_data {
      inline image_data(const size_t &count, const render::utils::extent_2d &size, const uint32_t &mip_levels) : count(count), images(new render::image[count]), size(size), mip_levels(mip_levels) {}
      inline ~image_data() { delete [] images; }
      
      size_t count;
      render::image* images;
      render::utils::extent_2d size;
      uint32_t mip_levels;
    };
  }
  
  namespace game {
    using image_data_container = const utils::resource_container_array<utils::id, core::image_data, 50>;
    using image_data_container_load = utils::resource_container_array<utils::id, core::image_data, 50>;
    
    struct image_resources_t : public ImageResourceContainer {
      uint32_t images;
      uint32_t samplers;
      char* samplers_mem;
      yavf::DescriptorSet* set;
      yavf::DescriptorSetLayout layout;
      yavf::DescriptorPool pool;
      yavf::Device* device;
      
      image_resources_t(yavf::Device* device);
      ~image_resources_t();
      uint32_t imagesCount() const override { return images; }
      uint32_t samplersCount() const override { return samplers; }
      yavf::DescriptorSet* resourceDescriptor() const override { return set; }
      yavf::DescriptorSetLayout resourceLayout() const override { return layout; }
    };
    
    using image_resources = const image_resources_t;
    using image_resources_load = image_resources_t;
  }
}

#endif
