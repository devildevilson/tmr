#include "image_loader.h"

#include "image_container.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "yavf.h"

namespace devils_engine {
  namespace game {
    image_resources_t::image_resources_t(yavf::Device* device) : images(0), samplers(2), samplers_mem(new char[sizeof(yavf::Sampler)*samplers]), set(nullptr), layout(VK_NULL_HANDLE), pool(VK_NULL_HANDLE), device(device) {
      auto samplers = reinterpret_cast<yavf::Sampler*>(samplers_mem);
      
      {
        yavf::SamplerMaker sm(device);
        
        samplers[0] = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT)
                        .anisotropy(VK_TRUE, 16.0f)
                        .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK)
                        .compareOp(VK_FALSE, VK_COMPARE_OP_MAX_ENUM)
                        .filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
                        .lod(0, 1000.0f)
                        .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
                        .unnormalizedCoordinates(VK_FALSE)
                        .create("default_sampler");

        samplers[1] = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
                        .anisotropy(VK_TRUE, 16.0f)
                        .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK)
                        .compareOp(VK_FALSE, VK_COMPARE_OP_MAX_ENUM)
                        .filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
                        .lod(0, 1000.0f)
                        .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
                        .unnormalizedCoordinates(VK_FALSE)
                        .create("clamp_to_border_sampler");
      }
      
      {
        yavf::DescriptorPoolMaker dpm(device);
        auto pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1).create("skybox_pool");
        
        yavf::DescriptorSetLayout skybox_set_layout = device->setLayout(SKYBOX_TEXTURE_LAYOUT_NAME);
        if (skybox_set_layout == VK_NULL_HANDLE) {
          yavf::DescriptorLayoutMaker dlm(device);
          skybox_set_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).create(SKYBOX_TEXTURE_LAYOUT_NAME);
        }
        
        yavf::DescriptorMaker dm(device);
        skybox_set = dm.layout(skybox_set_layout).create(pool)[0];
      }
      
      cube_image = nullptr;
    }
    
    image_resources_t::~image_resources_t() {
      auto s = reinterpret_cast<yavf::Sampler*>(samplers_mem);
      for (size_t i = 0; i < samplers; ++i) {
        device->destroy(s[i]);
      }
      
      delete [] samplers_mem;
      
      if (cube_image != nullptr) device->destroy(cube_image);
      
      if (pool != VK_NULL_HANDLE) {
        device->destroy(pool);
        pool = VK_NULL_HANDLE;
      }
      
      if (layout != VK_NULL_HANDLE) {
        device->destroy(layout);
        layout = VK_NULL_HANDLE;
      }
    }
  }
  
  namespace resources {
    bool VHpartValid(const size_t &indexFrom, const size_t &lastIndex, const std::string &str) {
      const size_t count = std::min(str.length(), lastIndex) - indexFrom;
      if (count > 2) return false;
      if (count == 2) return (str[indexFrom] == 'h' && str[indexFrom+1] == 'v') || (str[indexFrom] == 'v' && str[indexFrom+1] == 'h');
      return str[indexFrom] == 'h' || str[indexFrom] == 'v';
    }

    bool indexPartValid(const size_t &indexFrom, const size_t &indexLast, const std::string &str) {
      bool digitOk = true;
      for (size_t i = indexFrom; i < std::min(indexLast, str.size()); ++i) {
        digitOk = digitOk && isdigit(str[i]);
      }

      return digitOk;
    }
    
    void image_loader::parse_image_data_string(const std::string &str, utils::id &image_id, size_t &image_index, bool &flip_u, bool &flip_v) {
      image_index = 0;
      image_id = utils::id();

      size_t lastIndex = str.size();
      size_t nextPart = str.find_last_of('.');
      if (nextPart == std::string::npos) {
        image_id = utils::id::get(str);
        image_index = 0;
        flip_u = false;
        flip_v = false;
        return;
      }

      const size_t dotCount = std::count(str.begin(), str.end(), '.');

      flip_u = false;
      flip_v = false;
      bool vhPart = false;
      bool indexPart = false;

      for (uint32_t i = 0; i < std::min(size_t(2), dotCount); ++i) {
        {
          const bool tmp = VHpartValid(nextPart+1, lastIndex, str);
          if (!vhPart && tmp) {
            vhPart = tmp;

            flip_u = str[nextPart+1] == 'h' || str[nextPart+2] == 'h';
            flip_v = str[nextPart+1] == 'v' || str[nextPart+2] == 'v';

            lastIndex = nextPart;
            for (nextPart = lastIndex-1; nextPart != SIZE_MAX; --nextPart) {
              if (str[nextPart] == '.') break;

              //std::cout << "char: " << str[nextPart] << '\n';
            }

            continue;
          }
        }

        {
          const bool tmp = indexPartValid(nextPart+1, lastIndex, str);
          if (tmp) {
            indexPart = tmp;
            const std::string str122 = str.substr(nextPart+1, lastIndex);
            // std::cout << str122 << '\n';
            image_index = stoi(str122);

            lastIndex = nextPart;
            for (nextPart = lastIndex-1; nextPart != SIZE_MAX; --nextPart) {
              if (str[nextPart] == '.') break;
            }

            continue;
          }
        }
        
        if (i == 0 && !(vhPart || indexPart)) {
          image_id = utils::id::get(str);
          image_index = 0;
          flip_u = false;
          flip_v = false;
          return;
        }
      }

      image_id = utils::id::get(str.substr(0, lastIndex));
    }
    
    constexpr VkExtent2D to_vk(const render::utils::extent_2d &size) {
      return {size.width, size.height};
    }
    
    constexpr VkExtent2D to_vk(const render::utils::extent_3d &size) {
      return {size.width, size.height};
    }
    
    constexpr VkExtent3D to_vk3(const render::utils::extent_3d &size) {
      return {size.width, size.height, size.depth};
    }
    
    image_loader::image_loader(const create_info &info) : parser("textures"), device(info.device), container(info.container), data_container(info.data_container), additional_data(info.additional_data) {
//       additional_data->samplers = 2;
//       additional_data->samplers_mem = new char[sizeof(yavf::Sampler)*additional_data->samplers];
//       auto samplers = reinterpret_cast<yavf::Sampler*>(additional_data->samplers_mem);
//       
//       {
//         yavf::SamplerMaker sm(device);
//         
//         samplers[0] = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT)
//                         .anisotropy(VK_TRUE, 16.0f)
//                         .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK)
//                         .compareOp(VK_FALSE, VK_COMPARE_OP_MAX_ENUM)
//                         .filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
//                         .lod(0, 1000.0f)
//                         .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
//                         .unnormalizedCoordinates(VK_FALSE)
//                         .create("default_sampler");
// 
//         samplers[1] = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
//                         .anisotropy(VK_TRUE, 16.0f)
//                         .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK)
//                         .compareOp(VK_FALSE, VK_COMPARE_OP_MAX_ENUM)
//                         .filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
//                         .lod(0, 1000.0f)
//                         .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
//                         .unnormalizedCoordinates(VK_FALSE)
//                         .create("clamp_to_border_sampler");
//       }
    }
    
    image_loader::~image_loader() {}
    
    bool image_loader::validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      (void)errors;
      (void)warnings;
      return true;
    }
    
    bool image_loader::load(const utils::id &id) {
      {
        // проверка некоего контейнера в который мы должны загрузить информацию о изображениях
        // id + массив Image
        auto data = data_container->get(id);
        if (data != nullptr) return true;
      }
      
      auto ptr = loading_data.get(id);
      if (ptr == nullptr) return false;
      
      // сначало выделяем память
      size_t index = SIZE_MAX;
      for (size_t i = 0; i < container->pool_count(); ++i) {
        if (container->get_pool(i)->image_size().width == ptr->size.width && container->get_pool(i)->image_size().height == ptr->size.height && container->get_pool(i)->mip_levels() == ptr->mip_levels) {
          index = i;
          break;
        }
      }
      
//       if (index == SIZE_MAX) {
//         container->create_pool({ptr->size.width, ptr->size.height}, ptr->mip_levels);
//         index = container->pool_count()-1;
//       }
      
      auto data_cont = data_container->create(ptr->id, ptr->count, render::utils::extent_2d{ptr->size.width, ptr->size.height}, ptr->mip_levels);
      
      // мы должны подготовить изображения к копированию
      size_t count = 0;
      while (count < ptr->count) {
        if (index == SIZE_MAX) {
          container->create_pool({ptr->size.width, ptr->size.height}, ptr->mip_levels);
          index = container->pool_count()-1;
        }
        
        const size_t image_count = std::min(ptr->count - count, container->get_pool(index)->free_size());
        copies.push_back({ptr, count, image_count, static_cast<uint32_t>(index), {}});
        for (size_t i = count; i < image_count; ++i) {
          const render::image img = container->get_image(index);
          if (img.container == UINT32_MAX) throw std::runtime_error("img.container == UINT32_MAX");
          data_cont->images[i] = img;
          copies.back().indices.push_back(render::get_image_layer(img));
        }
        count += image_count;
        
        index = SIZE_MAX;
      }
      
      return true;
    }
    
    bool image_loader::unload(const utils::id &id) {
      auto data = data_container->get(id);
      if (data == nullptr) return false;
      
      for (size_t i = 0; i < data->count; ++i) {
        container->release_image(data->images[i]);
      }
      
      return data_container->destroy(id);
    }
    
    void vk_copy_data(const image::load_data* data, const size_t &start, const std::vector<uint32_t> &indices, std::vector<VkImageCopy> &copies) {
      uint32_t counter = 0;
      //uint32_t width = 0;
      //uint32_t height = 0;
      
      for (size_t i = 0 ; i < data->rows; ++i) {
        uint32_t height = data->size.height * i;
        //width = 0;
        
        for (size_t j = 0; j < data->columns; ++j) {
          size_t index = i * data->rows + j;
          if (index < start) continue;
          uint32_t width = data->size.width * j;
          
          const VkImageCopy copy{
            {
              VK_IMAGE_ASPECT_COLOR_BIT,
              0, 0, 1
            },
            {int32_t(width), int32_t(height), 0},
            {
              VK_IMAGE_ASPECT_COLOR_BIT,
              0, indices[counter], 1 //(counter-1) + startingLayer
            },
            {0, 0, 0},
            to_vk3(data->size)
          };
          ++counter;

          copies.push_back(copy);
        }
      }
    }
    
    void blit(const uint32_t &mipLevels, const uint32_t &layersCount, const VkExtent3D &size, std::vector<VkImageBlit> &blits) {
#ifdef _DEBUG
      const uint32_t mipLevelsCheck = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
      //  эта формула - это макисальное количество мип уровней? похоже на то
      // по такой формуле посчитаются мип уровни вплоть до 1х1 по идее
      // дольше в них нет необходимости
      
      ASSERT(mipLevelsCheck == mipLevels);
#endif

      int32_t mipWidth = size.width;
      int32_t mipHeight = size.height;
      for (uint32_t i = 1; i < mipLevels; ++i) {
        const VkImageBlit b{
          {
            VK_IMAGE_ASPECT_COLOR_BIT,
            i-1, 0, layersCount
          },
          {
            {0, 0, 0}, {mipWidth, mipHeight, 1}
          },
          {
            VK_IMAGE_ASPECT_COLOR_BIT,
            i, 0, layersCount
          },
          {
            {0, 0, 0}, {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}
          }
        };

        blits[i-1] = b;

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
      }
    }
    
    void image_loader::end() {
      if (copies.empty()) return;
      
      // собираем VkImageCopy и собственно копируем
      struct final_copy {
        yavf::Image* src;
        yavf::Image* dst;
        std::vector<VkImageCopy> copies;
        std::vector<VkImageBlit> blits;
      };
      
      std::vector<final_copy> finals;
      for (size_t i = 0; i < copies.size(); ++i) {
        auto dst = container->get_pool(copies[i].pool_index)->image;
        auto mips = container->get_pool(copies[i].pool_index)->mip_levels();
        
        int texWidth, texHeight, texChannels;
        uint8_t* pixels = stbi_load(copies[i].load_data->path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        const VkDeviceSize imageBytes = texWidth * texHeight * 4;
        
        if (pixels == nullptr) {
          throw std::runtime_error("failed to load image " + copies[i].load_data->id.name() + " path: " + copies[i].load_data->path);
        }
        
        auto staging_img = device->create(yavf::ImageCreateInfo::texture2DStaging({uint32_t(texWidth), uint32_t(texHeight)}), VMA_MEMORY_USAGE_CPU_ONLY);
        memcpy(staging_img->ptr(), pixels, imageBytes);
        stbi_image_free(pixels);
        
        std::vector<VkImageCopy> c;
        vk_copy_data(copies[i].load_data, copies[i].start, copies[i].indices, c);
        std::vector<VkImageBlit> blits(mips-1);
        if (mips > 1) blit(mips, render::image_container::image_pool::size, to_vk3(copies[i].load_data->size), blits);
        finals.push_back({staging_img, dst, c, blits});
      }
      
      auto task = device->allocateGraphicTask();
      task->begin();
      for (auto &copy : finals) {
        task->setBarrier(copy.src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        task->setBarrier(copy.dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        task->copy(copy.src, copy.dst, copy.copies);
    //    task->setBarrier(i.dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        for (uint32_t j = 1; j < copy.dst->info().mipLevels; ++j) {
          const VkImageSubresourceRange range{
            VK_IMAGE_ASPECT_COLOR_BIT,
            j-1, 1, 0, copy.dst->info().arrayLayers
          };

          task->setBarrier(copy.dst->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range);
          task->copyBlit(copy.dst, copy.dst, copy.blits[j-1], VK_FILTER_NEAREST); //VK_FILTER_NEAREST
        }
        
        if (copy.dst->info().mipLevels > 1) {
          const VkImageSubresourceRange range{
            VK_IMAGE_ASPECT_COLOR_BIT,
            copy.dst->info().mipLevels-1, 1, 0, copy.dst->info().arrayLayers
          };
          task->setBarrier(copy.dst->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range);
        }

        copy.dst->info().initialLayout = copy.dst->info().mipLevels > 1 ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : copy.dst->info().initialLayout;
        task->setBarrier(copy.dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      }

      task->end();

      task->start();
      task->wait();
      device->deallocate(task);
      
      for (auto &copy : finals) {
        device->destroy(copy.src);
      }
      
      // обновить дескриптор пул
      // основная проблема в том что мне нужно положить дескриптор пул примерно туда же где лежит data_container
      // так как требуется обновлять пайплайны после обновлений дескрипторов
      additional_data->images = container->pool_count();
      recreate_descriptor_pool();
      copies.clear();
    }
    
    void image_loader::clear() {
      
    }
    
    utils::id image_loader::check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, image::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      bool has_id = false, valid = false, hasCount = false, hasWidth = false, hasHeight = false;
      uint32_t count = 1, rows = 1, columns = 1, width = 0, height = 0;
      std::string texturePath;
      const std::string path = path_prefix+file;
      
      int32_t loadedWidth, loadedHeight;
      info.mem_size = 0;
      info.parser_mark = mark;
      info.mip_levels = 1;
      info.size.depth = 1;
      info.size.height = 1;
      info.size.width = 1;
      
      for (auto concreteTIt = data.begin(); concreteTIt != data.end(); ++concreteTIt) {
        if (concreteTIt.value().is_string() && concreteTIt.key() == "id") {
          has_id = true;
          info.id = utils::id::get(concreteTIt.value().get<std::string>());
          continue;
        }
        
        if (concreteTIt.value().is_string() && concreteTIt.key() == "path") {
          texturePath = concreteTIt.value().get<std::string>();
          
          int32_t comp;
          int ret = stbi_info((path_prefix + texturePath).c_str(), &loadedWidth, &loadedHeight, &comp);
          info.gpu_size = loadedWidth * loadedHeight * 4;
          if (ret) info.path = path_prefix + texturePath;
          else {
            errors.add(mark, ERROR_COULD_NOT_FIND_TEXTURE_FILE, "Could not find texture file "+path_prefix + texturePath);
            return utils::id();
          }
          
    //       if (!bool(ret)) throw std::runtime_error("dqsdqwfgqfwfqffqfqfwfavsavaw " + pathPrefix + texturePath);
          
          continue;
        }
        
        if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "count") {
          count = concreteTIt.value().get<size_t>();
          hasCount = count > 1;
          continue;
        }
        
        if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "rows") {
          rows = concreteTIt.value().get<size_t>();
    //       hasRows = rows > 1;
          continue;
        }
        
        if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "columns") {
          columns = concreteTIt.value().get<size_t>();
    //       hasColumns = columns > 1;
          continue;
        }
        
        if (concreteTIt.value().is_object() && concreteTIt.key() == "size") {
          for (auto sizeIt = concreteTIt.value().begin(); sizeIt != concreteTIt.value().end(); ++sizeIt) {
            if (sizeIt.value().is_number_unsigned() && sizeIt.key() == "width") {
              width = sizeIt.value().get<uint32_t>();
              hasWidth = width > 0;
              continue;
            }
            
            if (sizeIt.value().is_number_unsigned() && sizeIt.key() == "height") {
              height = sizeIt.value().get<uint32_t>();
              hasHeight = height > 0;
              continue;
            }
          }
        }
      }
      
      const bool hasSize = hasWidth && hasHeight;

      if (!texturePath.empty() && has_id) valid = true;
      
      if (!has_id) {
        errors.add(mark, MISSED_TEXTURE_ID, "Image must have an id"); // " File: " + path
        return utils::id();
      }
      
      if (!valid) {
        errors.add(mark, MISSED_TEXTURE_PATH, "Missing image "+info.id.name()+" path");
        return utils::id();
      }
      
      if (hasCount && count > rows * columns) {
        errors.add(mark, TOO_MUCH_TEXTURE_COUNT, "Images count " + std::to_string(count) + " > rows*columns " + std::to_string(rows * columns));
        return utils::id();
      }
      
      if (hasCount && !hasSize) {
        errors.add(mark, MISSED_TEXTURE_SIZE, "Missing image "+info.id.name()+" size");
        return utils::id();
      }
      
      info.columns = columns;
      info.rows = rows;
      info.count = count;
      info.size.width = hasCount && hasSize ? width : loadedWidth;
      info.size.height = hasCount && hasSize ? height : loadedHeight;
      info.mip_levels = std::floor(std::log2(std::max(info.size.width, info.size.height)))+1;
      //info.mip_levels = 1;
      
      (void)warnings;
      
      return info.id;
    }
    
    void image_loader::recreate_descriptor_pool() {
      if (additional_data->pool != VK_NULL_HANDLE) {
        device->destroy(additional_data->pool);
        additional_data->pool = VK_NULL_HANDLE;
      }
      
      if (additional_data->layout != VK_NULL_HANDLE) {
        device->destroy(additional_data->layout);
        additional_data->layout = VK_NULL_HANDLE;
      }
      
      {
        yavf::DescriptorPoolMaker dpm(device);

        additional_data->pool = dpm.flags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                                   .poolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, additional_data->imagesCount())
                                   .poolSize(VK_DESCRIPTOR_TYPE_SAMPLER, additional_data->samplersCount())
                                   .create("texture_descriptor_pool");
      }
      
      {
        yavf::DescriptorLayoutMaker dlm(device);
        additional_data->layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, additional_data->imagesCount())
                                     .binding(1, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, additional_data->samplersCount())
                                     .create("texture_data_array_layout");
      }
    
      {
        yavf::DescriptorMaker dm(device);
        
        additional_data->set = dm.layout(additional_data->layout).create(additional_data->pool)[0];
        additional_data->set->resize(additional_data->imagesCount() + additional_data->samplersCount());
        container->update_descriptor_data(additional_data->set);
        
        auto samplers = reinterpret_cast<yavf::Sampler*>(additional_data->samplers_mem);
        for (size_t i = 0; i < additional_data->samplersCount(); ++i) {
          additional_data->set->at(i + additional_data->imagesCount()) = {samplers[i], nullptr, VK_IMAGE_LAYOUT_MAX_ENUM, static_cast<uint32_t>(i), 1, VK_DESCRIPTOR_TYPE_SAMPLER};
        }

        additional_data->set->update();
      }
    }
  }
}
