#ifndef IMAGE_CONTAINER_H
#define IMAGE_CONTAINER_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include "shared_structures.h"
#include "image_data.h"

#define TEXTURE_MAX_LAYER_COUNT 2048

namespace yavf {
  class Device;
  class Image;
  class DescriptorSet;
}

namespace devils_engine {
  namespace render {
    class image_container {
    public:
      struct image_pool {
        static const size_t size = UINT8_MAX+1;
        static const size_t container_size = size/SIZE_WIDTH;
        static_assert(size % SIZE_WIDTH == 0, "Better if image pool size would be devided by SIZE_WIDTH");
        static_assert(size < TEXTURE_MAX_LAYER_COUNT, "Image pool size must be less than TEXTURE_MAX_LAYER_COUNT");
        
        size_t container[container_size];
        yavf::Image* image;

        image_pool(yavf::Device* device, const utils::extent_2d &img_size, const uint32_t &mips);
        utils::extent_2d image_size() const;
        uint32_t mip_levels() const;
        size_t used_size() const;
        size_t free_size() const;
        bool full() const;
        
        bool is_used(const size_t &index) const;
        void set_used(const size_t &index, const bool value);
        
        uint32_t occupy_free_index();
        void release_index(const uint32_t &index);
      };
      
      struct create_info {
        yavf::Device* device;
      };
      image_container(const create_info &info);
      ~image_container();
      
      size_t pool_count() const;
      const image_pool* get_pool(const size_t &index) const;
      image get_image(const size_t &pool_index);
      void release_image(const image &img);
      void create_pool(const utils::extent_2d &img_size, const uint32_t &mips);
      
      void update_descriptor_data(yavf::DescriptorSet* set);
    private:
      yavf::Device* device;
      std::vector<image_pool> slots;
    };
  }
}

#endif
