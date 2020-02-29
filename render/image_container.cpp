#include "image_container.h"

#include "yavf.h"

namespace devils_engine {
  namespace render {
    image_container::image_pool::image_pool(yavf::Device* device, const image::extent_2d &img_size, const uint32_t &mips) {
      image = device->create(
        yavf::ImageCreateInfo::texture2D(
          {img_size.width, img_size.height}, 
          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
          VK_FORMAT_R8G8B8A8_UNORM, 
          size, 
          mips
        ), 
        VMA_MEMORY_USAGE_GPU_ONLY
      );
      
      image->createView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT);
      memset(container, 0, container_size*sizeof(size_t));
    }
    
    image::extent_2d image_container::image_pool::image_size() const {
      return {image->info().extent.width, image->info().extent.height};
    }
    
    uint32_t image_container::image_pool::mip_levels() const {
      return image->info().mipLevels;
    }
    
    size_t image_container::image_pool::used_size() const {
      size_t counter = 0;
      for (size_t i = 0; i < size; ++i) {
        counter += size_t(is_used(i));
      }
      return counter;
    }
    
    size_t image_container::image_pool::free_size() const {
      size_t counter = 0;
      for (size_t i = 0; i < size; ++i) {
        counter += size_t(!is_used(i));
      }
      return counter;
    }
    
    bool image_container::image_pool::full() const {
      return size == used_size();
    }
    
    bool image_container::image_pool::is_used(const size_t &index) const {
      const size_t idx = index / SIZE_WIDTH;
      const size_t bit = index % SIZE_WIDTH;
      const size_t mask = 1 << bit;
      return (container[idx] & mask) == mask;
    }
    
    void image_container::image_pool::set_used(const size_t &index, const bool value) {
      const size_t idx = index / SIZE_WIDTH;
      const size_t bit = index % SIZE_WIDTH;
      const size_t mask = 1 << bit;
      container[idx] = value ? container[idx] | mask : container[idx] & ~mask;
    }
    
    uint32_t image_container::image_pool::occupy_free_index() {
      for (uint32_t i = 0; i < size; ++i) {
        if (!is_used(i)) {
          set_used(i, true);
          return i;
        }
      }
      return UINT32_MAX;
    }
    
    void image_container::image_pool::release_index(const uint32_t &index) {
      if (index >= size) throw std::runtime_error("Releasing bad index");
      set_used(index, false);
    }
    
    image_container::image_container(const create_info &info) : device(info.device) {}
    image_container::~image_container() {
      for (auto &pool : slots) {
        device->destroy(pool.image);
      }
    }
    
    size_t image_container::pool_count() const {
      return slots.size();
    }
    
    const image_container::image_pool* image_container::get_pool(const size_t &index) const {
      if (index >= slots.size()) return nullptr;
      return &slots[index];
    }
    
    Image image_container::get_image(const size_t &pool_index) {
      if (pool_index >= slots.size()) return {UINT32_MAX, UINT32_MAX};
      const uint32_t free_index = slots[pool_index].occupy_free_index();
      if (free_index == UINT32_MAX) return {UINT32_MAX, UINT32_MAX};
      return {static_cast<uint32_t>(pool_index), slots[pool_index].occupy_free_index()};
    }
    
    void image_container::release_image(const Image &img) {
      if (img.index >= slots.size()) throw std::runtime_error("Trying to release bad image");
      slots[img.index].release_index(img.layer);
    }
    
    void image_container::create_pool(const image::extent_2d &img_size, const uint32_t &mips) {
      slots.emplace_back(device, img_size, mips);
    }
    
    void image_container::update_descriptor_data(yavf::DescriptorSet* set) {
      if (slots.size() >= set->size()) throw std::runtime_error("slots.size() >= set->size()");
      for (size_t i = 0; i < slots.size(); ++i) {
        set->at(i) = {VK_NULL_HANDLE, slots[i].image->view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, static_cast<uint32_t>(i), 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE};
      }
    }
  }
}
