#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include "id.h"
#include "resource.h"
#include "resource_parser.h"
#include "resource_container.h"
#include "shared_structures.h"
//#include "image_container.h"
#include "image_data.h"

namespace yavf {
  class Image;
  class Device;
}

namespace devils_engine {
  namespace render {
    class image_container;
  }
  
  namespace resources {
    namespace image {
      struct load_data : public core::resource {
        uint32_t rows;
        uint32_t columns;
        uint32_t count;
        uint32_t mip_levels;
        render::utils::extent_3d size;
        std::string path;
      };
      
      struct data {
        size_t count;
        render::image* images;
        render::utils::extent_2d size;
        uint32_t mip_levels;
      };
    }
    
    class image_loader : public parser<image::load_data, 50>, public validator, public loader {
    public:
      static void parse_image_data_string(const std::string &str, utils::id &image_id, size_t &image_index, bool &flip_u, bool &flip_v);
      
      enum errors {
        MISSED_TEXTURE_PATH,
        ERROR_COULD_NOT_FIND_TEXTURE_FILE,
        TOO_MUCH_TEXTURE_COUNT,
        MISSED_TEXTURE_SIZE,
        MISSED_TEXTURE_ID,
        TEXTURE_ERRORS_COUNT
      };
      
      struct create_info {
        yavf::Device* device;
        render::image_container* container;
        game::image_data_container_load* data_container;
        game::image_resources_load* additional_data;
      };
      image_loader(const create_info &info);
      ~image_loader();
      
      bool validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
      
      bool load(const utils::id &id) override;
      bool unload(const utils::id &id) override;
      void end() override;
      void clear() override;
    private:
      // по одному?
      struct copy_data {
        const image::load_data* load_data;
        //Image img;
        size_t start;
        size_t count;
        uint32_t pool_index;
        std::vector<uint32_t> indices;
      };
      
      yavf::Device* device;
      render::image_container* container;
      game::image_data_container_load* data_container;
      game::image_resources_load* additional_data;
      std::vector<copy_data> copies;
      
      utils::id check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, image::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
      void recreate_descriptor_pool();
    };
  }
}

#endif
