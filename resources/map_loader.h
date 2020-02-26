#ifndef MAP_LOADER_H
#define MAP_LOADER_H

#include "id.h"
#include "resource.h"
#include "resource_parser.h"
#include "resource_container.h"
#include "RenderStructures.h"
#include "map_data.h"
#include "state.h"

// карта это такой ресурс, который создает другие ресурсы
// и ничего особо после себя не оставляет (разве что номер эпизода и айди карты)
// карта определяется набором энтити которые на ней находятся
// энтити определяются тремя типами: энтити определенные в json, стены и сложные объекты (двери)
// короче я тут примерно понял что должны представлять из себя двери: 
// это главный энтити + несколько энтити "стен"

class TransformComponent;
class PhysicsComponent;
class InputComponent;
struct UserDataComponent;
class SoundComponent;

namespace yacs {
  class world;
  class entity;
}

namespace yavf {
  class Device;
  class Buffer;
}

namespace devils_engine {
  namespace components {
    struct type_info;
    struct states;
    struct input;
    struct sprite_graphics;
    struct indexed_graphics;
    struct point_light;
    struct basic_ai;
    struct tree_ai;
    struct func_ai;
    class attributes;
    class effects;
    class inventory;
    class movement;
    class abilities;
    class vertex;
  }
  
  namespace map {
    struct load_data : public core::resource {
      struct texture {
        utils::id id;
        size_t index;
      };
      
      struct object_flags {
        uint32_t container;
        
        object_flags();
        bool baby() const;
        bool easy() const;
        bool medium() const;
        bool hard() const;
        bool nightmare() const;
        bool ambush() const;
        bool static_obj() const;
        
        void set(const uint32_t &index, const bool value);
        bool exist_on_current_diff(const game::difficulty &diff) const;
      };
      
      struct entity_data {
        utils::id id;
        float pos[3];
        float rot[3];
        uint32_t tag;
        object_flags flags;
        
        entity_data();
        bool valid() const;
      };
      
      struct faces {
        struct idx {
          uint32_t vertex;
          uint32_t texture_coordinates;
        };
        
        std::vector<idx> indices;
        utils::id state_id; // или текстурка?
        texture image;
        float ambient;
        uint32_t tag;
        object_flags flags; // 32 бита?
        
        faces();
      };
      
//       struct complex_objects {
//         utils::id name;
//         float pos[4];
//         float rot[4];
//         uint32_t tag;
//         uint32_t flags; // тут кстати даже флаг тип статик не статик
//         std::vector<faces> sides; // указатель?
//         uint32_t points_start; // это + faces::idx::vertex получается индекс точки
//         uint32_t tex_coords_start;
//       };
      
      struct model {
        utils::id id;
        std::vector<faces> sides;
        uint32_t points_start;
        uint32_t tex_coords_start;
        
        model();
      };
      
      struct complex_object {
        size_t model_index;
        //utils::id name; // ??
        float pos[3];
        float rot[4];
        uint32_t tag;
        object_flags flags;
        
        complex_object();
        bool valid() const;
      };
      
      // эти векторы только при загрузке нужны
      std::vector<glm::vec3> vertices;
      std::vector<glm::vec2> tex_coords;
      
      // сначала должна быть карта
      std::vector<faces> walls;
      std::vector<entity_data> entities;
      std::vector<model> models;
      std::vector<complex_object> objects; // скорее нужно разбить на два: модель + данные энтити
      std::string path;
      utils::id next;
      utils::id next_secret;
    };
  }
  
  namespace resources {
    class entity_loader;
    class state_loader;
    class image_loader;
    
    class map_loader : public parser<map::load_data, 10>, public validator, public loader {
    public:
      enum errors {
        ERROR_COULD_NOT_FIND_ID,
        ERROR_COULD_NOT_PATH,
        ERROR_COULD_NOT_LOAD_MAP_FILE,
        ERROR_BAD_DATA_IN_MAP,
        ERROR_MAP_INFO_MUST_START_WITH_WALLS_INFO,
        ERROR_BAD_NUMERIC_DATA,
        ERROR_BAD_FACE_DATA,
        ERROR_COULD_NOT_FIND_MODEL_NAME,
        ERROR_COULD_NOT_FIND_ENTITY,
        ERROR_COULD_NOT_FIND_IMAGE,
        ERROR_COULD_NOT_FIND_STATE
      };
      
      enum warnings {
        WARNING_UNKNOWN_MAP_INFO_LINE_START,
        
      };
      
      struct create_info {
        yacs::world* world;
        entity_loader* entities;
        state_loader* states;
        image_loader* images;
        yavf::Device* device;
        game::map_data_container_load* container;
      };
      map_loader(const create_info &info);
      
      bool validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
      
      bool load(const utils::id &id) override;
      bool unload(const utils::id &id) override;
      void end() override;
      void clear() override;
    
      bool load_obj(const std::string &path);
      bool change_map(const utils::id &episod, const utils::id &map_id, const game::difficulty &diff);
      void save_map(const std::string &path); // при редактировании мне пригодится
      
      utils::id current_map() const;
      yacs::entity* player() const;
    private:
      yacs::world* world;
      yacs::entity* player_ptr;
      entity_loader* entities;
      state_loader* states;
      image_loader* images;
      yavf::Device* device;
      //yavf::Buffer* vertices;
      //yavf::Buffer* indices;
      utils::id current;
      uint32_t current_index;
      game::map_data_container_load* container;
      
      utils::id check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, map::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
      
      yacs::entity* create_wall(const size_t &index, const map::load_data::faces &face, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec2> &tex_coords, std::vector<Vertex> &verts);
      yacs::entity* create_complex_obj(const map::load_data::complex_object &obj, const map::load_data::model &model, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec2> &tex_coords, std::vector<Vertex> &verts);
      yacs::entity* create_player(); // добавится потом id (маг, воин, лучник)
      
      components::type_info* create_type_info(yacs::entity* ent, const utils::id &id, yacs::entity* parent);
//       UserDataComponent* create_usr_data(yacs::entity* ent);
      PhysicsComponent* create_physics(yacs::entity* ent, const void* info);
      components::indexed_graphics* create_graphics(yacs::entity* ent, const uint32_t &offset, const uint32_t &count, const uint32_t &index);
      components::states* create_states(yacs::entity* ent, const core::state_t* state);
      components::vertex* create_vertex(yacs::entity* ent, const simd::vec4 &center, const simd::vec4 &normal);
    };
  }
}

#endif
