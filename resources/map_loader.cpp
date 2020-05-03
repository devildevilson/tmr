#include "map_loader.h"

#include "Globals.h"
#include "EntityComponentSystem.h"
#include "entity_loader.h"
#include "state_loader.h"
#include "image_loader.h"
#include "Physics.h"
#include "CPUOctreeBroadphaseParallel.h"
#include "entity_creator_resources.h"
#include "global_components_indicies.h"
#include "shared_collision_constants.h"
#include <fstream>
#include <cstring>

#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "InputComponent.h"
// #include "SoundComponent.h"
#include "graphics_component.h"
#include "states_component.h"
#include "abilities_component.h"
#include "type_info_component.h"
// #include "UserDataComponent.h"
#include "ai_component.h"
#include "attributes_component.h"
#include "effects_component.h"
#include "inventory_component.h"
#include "global_components_indicies.h"
#include "movement_component.h"
#include "collision_property.h"
#include "vertex_component.h"
#define STATES_CONTAINER
#include "game_resources.h"
#include "graph.h"
#include "decals_container_component.h"

#include "scene_data.h"

#include <random>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include "yavf.h"

#define MAX_LINE_SIZE 256

enum class current_data {
  map,
  complex_obj,
  entity,
  entity_with_model,
  count
};

const std::vector<std::string> flags = {
  "baby",
  "easy",
  "medium",
  "hard",
  "nightmare",
  
};

enum obj_flags_enum {
  OBJ_FLAG_BABY,
  OBJ_FLAG_EASY,
  OBJ_FLAG_MEDIUM,
  OBJ_FLAG_HARD,
  OBJ_FLAG_NIGHTMARE,
  OBJ_FLAG_AMBUSH,
  OBJ_FLAG_ROTATION,
  
  OBJ_FLAG_COUNT
};

const char* flag_strings[] = {
  "baby",
  "easy",
  "medium",
  "hard",
  "nightmare",
  "ambush",
  nullptr
};

size_t find_first_not_of(const char *str, const char *cmp_str, const size_t &pos) {
  const size_t len = strcspn(str + pos, cmp_str);
  return len + pos;
}

size_t end_line(const char *line) {
  for (size_t i = 0; i < MAX_LINE_SIZE; ++i) {
    if (line[i] == '\n') return i;
  }
  
  return SIZE_MAX;
}

std::istream & safe_get_line(std::istream & is, char* s, const size_t &max_size) {
  // The characters in the stream are read one-by-one using a std::streambuf.
  // That is faster than reading them one-by-one using the std::istream.
  // Code that uses streambuf this way must be guarded by a sentry object.
  // The sentry object performs various tasks,
  // such as thread synchronization and updating the stream state.

  std::istream::sentry se(is, true);
  std::streambuf* sb = is.rdbuf();
  size_t current_index = 0;

  for(size_t i = 0; i < max_size; ++i) {
    int c = sb->sbumpc();
    s[current_index] = (char)c;
    ++current_index;
    switch (c) {
    case '\n': return is;
    case '\r':
      if(sb->sgetc() == '\n') sb->sbumpc();
      return is;
    case std::streambuf::traits_type::eof():
      // Also handle the case when the last line has no line ending
      if(current_index == 1) is.setstate(std::ios::eofbit);
      return is;
    }
  }

  return is;
}

namespace devils_engine {
  namespace map {
    load_data::object_flags apply_flag(const std::string &flag, const load_data::object_flags &old_flag);
  }
  
  bool parse_map(const std::string &path, const size_t &mark, map::load_data& info, utils::problem_container<resources::info::error> &errors, utils::problem_container<resources::info::warning> &warnings) {
    std::fstream file(path);
    if (!file) {
      errors.add(mark, resources::map_loader::ERROR_COULD_NOT_LOAD_MAP_FILE, "Could not load map file");
      return false;
    }
    
    bool has_map = false;
    current_data cur = current_data::count;
    utils::id next_name;
    
    size_t line_count = 0;
    char line[MAX_LINE_SIZE];
    //while (file.getline(line, MAX_LINE_SIZE)) {
    while (safe_get_line(file, line, MAX_LINE_SIZE)) {
      const size_t line_index = line_count++;
      const size_t line_size = end_line(line);
      if (file.eof()) break;
      if (line_size == 0) continue;
      assert(line_size != SIZE_MAX);

      const std::string line_str(line, line_size);
      //std::cout << "line " << line_index << ": " << line_str << '\n';
      const size_t index = line_str.find_first_not_of(" ", 0);
      switch (line[index]) {
        case 'o': {
          if (line_str.find_first_of("map") != std::string::npos) {
            cur = current_data::map;
            has_map = true;
            std::cout << "map" << "\n";
          } else {
            if (!has_map) {
              errors.add(mark, resources::map_loader::ERROR_MAP_INFO_MUST_START_WITH_WALLS_INFO, "\'map\' object must be first");
              return false;
            }
            
            const size_t name_start = line_str.find_first_not_of(" ", index+1);
            const size_t name_end = line_str.find_first_of(" ", name_start+1);
            const std::string name = line_str.substr(name_start, name_end-name_start);
            
            for (const auto &model : info.models) {
              if (model.id.name() == name) {
                errors.add(mark, resources::map_loader::ERROR_OBJECT_WITH_THIS_NAME_IS_ALREADY_EXIST, "object with name "+name+" is already exist");
                return false;
              }
            }
            
            next_name = utils::id::get(name);
            cur = current_data::complex_obj;
            info.models.emplace_back();
            info.models.back().id = next_name;
            info.models.back().points_start = info.vertices.size();
            info.models.back().tex_coords_start = info.tex_coords.size();
            std::cout << "obj name " << name << "\n";
          }
          break;
        }

        case 'e': {
          if (!has_map) {
            errors.add(mark, resources::map_loader::ERROR_MAP_INFO_MUST_START_WITH_WALLS_INFO, "\'map\' object must be first");
            return false;
          }
          
          const size_t name_start = line_str.find_first_not_of(" ", index+1);
          const size_t name_end = line_str.find_first_of(" ", name_start+1);
          const std::string name = line_str.substr(name_start, name_end-name_start);
          std::cout << "entity name " << name << "\n";
          
          bool found_complex = false;
          for (size_t i = 0; i < info.models.size(); ++i) {
            if (info.models[i].id.name() == name) {
              found_complex = true;
              cur = current_data::entity_with_model;
              next_name = utils::id::get(name);
              info.objects.emplace_back();
              info.objects.back().model_index = i;
            }
          }
          
          if (found_complex) break;
          
          cur = current_data::entity;
          next_name = utils::id::get(name);
          info.entities.emplace_back();
          info.entities.back().id = next_name;
          break;
        }

        case 'p': {
          float point[3];
          size_t num_start = line_str.find_first_not_of(" ", index+1);
          size_t num_end = line_str.find_first_of(" ", num_start+1);
          for (size_t i = 0; i < 3; ++i) {
            if (!isdigit(line_str[num_start])) {
              errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
              return false;
            }
            
            point[i] = atof(&line_str[num_start]);
            num_start = line_str.find_first_not_of(" ", num_end+1);
            num_end = line_str.find_first_of(" ", num_start+1);
          }

          std::cout << "point x: " << point[0] << " y: " << point[1] << " z: " << point[2] << "\n";

          switch (cur) {
            case current_data::map: 
            case current_data::complex_obj: {
              info.vertices.push_back(glm::vec3(point[0], point[1], point[2]));
              break;
            }
            
            case current_data::entity_with_model: {
              if (info.objects.back().valid()) {
                info.objects.emplace_back();
                info.objects.back() = info.objects[info.objects.size()-2];
                info.objects.back().tag = UINT32_MAX;
                memcpy(info.objects.back().pos, point, sizeof(float)*3);
              } else {
                memcpy(info.objects.back().pos, point, sizeof(float)*3);
              }
              
              break;
            }

            case current_data::entity: {
              if (info.entities.back().valid()) {
                info.entities.emplace_back();
                info.entities.back() = info.entities[info.entities.size()-2];
                info.entities.back().tag = UINT32_MAX;
                memcpy(info.entities.back().pos, point, sizeof(float)*3);
              } else {
                memcpy(info.entities.back().pos, point, sizeof(float)*3);
              }
              break;
            }

            case current_data::count: {
              //std::cout << "\'p\' is considered outside of obj or entity data at line " << line_index << '\n';
              errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'p\' is considered outside of obj or entity data at line "+std::to_string(line_index));
              return false;
            }
          }
          break;
        }

        case 'c': {
          float point[2];
          size_t num_start = line_str.find_first_not_of(" ", index+1);
          size_t num_end = line_str.find_first_of(" ", num_start+1);
          for (size_t i = 0; i < 2; ++i) {
            if (!isdigit(line_str[num_start])) {
              errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
              return false;
            }
            
            point[i] = atof(&line_str[num_start]);
            num_start = line_str.find_first_not_of(" ", num_end+1);
            num_end = line_str.find_first_of(" ", num_start+1);
          }
          
          if (cur != current_data::map && cur != current_data::complex_obj) {
            errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'c\' must be inside map or object data. Line "+std::to_string(line_index));
            return false;
          }

          std::cout << "tex coord u: " << point[0] << " v: " << point[1] << "\n";
          info.tex_coords.push_back(glm::vec2(point[0], point[1]));
          break;
        }

        case 'd': {
          size_t num_start = line_str.find_first_not_of(" ", index+1);
          size_t num_end = line_str.find_first_of(" ", num_start+1);
          
          size_t tmp_num_start = num_start;
          size_t tmp_num_end = num_end;
          size_t count = 0;
          while (tmp_num_start != std::string::npos) {
            count++;
            tmp_num_start = line_str.find_first_not_of(" ", tmp_num_end+1);
            tmp_num_end = line_str.find_first_of(" ", tmp_num_start+1);
          }
          
          if (count != 3 && count != 4) {
            errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
            return false;
          }
          
          float point[count];
          for (size_t i = 0; i < count; ++i) {
            if (!isdigit(line_str[num_start])) {
              errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
              return false;
            }
            
            point[i] = atof(&line_str[num_start]);
            num_start = line_str.find_first_not_of(" ", num_end+1);
            num_end = line_str.find_first_of(" ", num_start+1);
          }
          
          if (count == 3 && cur != current_data::entity) {
            errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'d\' with 3 element must be inside entity data. Line "+std::to_string(line_index));
            return false;
          } else {
            if (info.entities.empty()) {
              errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'d\' must be within entity data. Line "+std::to_string(line_index));
              //throw std::runtime_error("empty entities line "+std::to_string(line_index));
            }
            
            std::cout << "dir x: " << point[0] << " y: " << point[1] << " z: " << point[2] << "\n";
            ASSERT(count == 3 && cur == current_data::entity);
            memcpy(info.entities.back().rot, point, sizeof(float)*3);
          }
          
          if (count == 4 && cur != current_data::entity_with_model) {
            errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'d\' with 4 element must be inside complex entity data. Line "+std::to_string(line_index));
            return false;
          } else {
            if (info.objects.empty()) {
              errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'d\' must be within entity data. Line "+std::to_string(line_index));
              //throw std::runtime_error("empty complex entities line "+std::to_string(line_index));
            }
            
            std::cout << "rot w: " << point[0] << " x: " << point[1] << " y: " << point[2] << " z: " << point[3] << "\n";
            ASSERT(count == 4 && cur == current_data::entity_with_model);
            memcpy(info.objects.back().rot, point, sizeof(float)*4);
          }
          
          if (cur != current_data::entity) {
            errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'d\' must be inside entity data. Line "+std::to_string(line_index));
            return false;
          }
          
          // тут просто не нужно забывать создавать энтити в начале
          if (info.entities.empty()) {
            throw std::runtime_error("empty entities line "+std::to_string(line_index));
          }
          
          memcpy(info.entities.back().rot, point, sizeof(float)*3);
          break;
        }
        
        case 'r': {
//           float point[4];
//           size_t num_start = line_str.find_first_not_of(" ", index+1);
//           size_t num_end = line_str.find_first_of(" ", num_start+1);
//           for (size_t i = 0; i < 4; ++i) {
//             if (!isdigit(line_str[num_start])) {
//               errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
//               return false;
//             }
//             
//             point[i] = atof(&line_str[num_start]);
//             num_start = line_str.find_first_not_of(" ", num_end+1);
//             num_end = line_str.find_first_of(" ", num_start+1);
//           }
// 
//           std::cout << "rotation x: " << point[0] << " y: " << point[1] << " z: " << point[2] << " w: " << point[3] << "\n";
//           if (cur != current_data::entity_with_model) {
//             errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'r\' must be inside object entity data. Line "+std::to_string(line_index));
//             return false;
//           }
//           
//           memcpy(info.objects.back().rot, point, sizeof(float)*4);
          float arr[8];
          size_t num_start = line_str.find_first_not_of(" ", index+1);
          size_t num_end = line_str.find_first_of(" ", num_start+1);
          for (size_t i = 0; i < 8; ++i) {
            if (!isdigit(line_str[num_start])) {
              errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
              return false;
            }
            
            arr[i] = atof(&line_str[num_start]);
            num_start = line_str.find_first_not_of(" ", num_end+1);
            num_end = line_str.find_first_of(" ", num_start+1);
          }
          
          int32_t time = 0;
          if (num_start != std::string::npos) {
            if (!isdigit(line_str[num_start])) {
              errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
              return false;
            }
            
            time = atoi(&line_str[num_start]);
          }
          uint32_t final_time = time < 0 ? 0 : time;
          
          if (cur != current_data::entity_with_model) {
            errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'r\' must be inside complex entity data. Line "+std::to_string(line_index));
            return false;
          }
          
          std::cout << "pivot (" << arr[0] << ", " << arr[1] << ", " << arr[2] << ")\n";
          std::cout << "axis  (" << arr[3] << ", " << arr[4] << ", " << arr[5] << ")\n";
          std::cout << "start  " << arr[6] << "\n";
          std::cout << "end    " << arr[7] << "\n";
          std::cout << "time   " << final_time << "\n";
          
          memcpy(info.objects.back().pivot, &arr[0], sizeof(float)*3);
          memcpy(info.objects.back().axis, &arr[3], sizeof(float)*3);
          info.objects.back().start_angle = arr[6];
          info.objects.back().end_angle = arr[7];
          info.objects.back().time = final_time;
          info.objects.back().flags.set(OBJ_FLAG_ROTATION, true);
          break;
        }

        case 't': {
          const size_t num_start = line_str.find_first_not_of(" ", index+1);
          if (!isdigit(line_str[num_start])) {
            errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
            return false;
          }
          
          uint32_t tag = isdigit(line_str[num_start]) ? atol(&line_str[num_start]) : UINT32_MAX;
          std::cout << "tag " << tag << "\n";
          switch (cur) {
            case current_data::map: {
              info.walls.back().tag = tag;
              break;
            }
            
            case current_data::complex_obj: {
              info.models.back().sides.back().tag = tag;
              break;
            }
            
            case current_data::entity_with_model: {
              info.objects.back().tag = tag;
              break;
            }
            
            case current_data::entity: {
              info.entities.back().tag = tag;
              break;
            }
            
            case current_data::count: {
              errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'t\' is considered outside of obj or entity data at line "+std::to_string(line_index));
              return false;
            }
          }
          break;
        }

        // но вообще здесь можно и число считывать
        case 'f': {
          size_t flag_start = line_str.find_first_not_of(" ", index+1);
          size_t flag_end = line_str.find_first_of(" ", flag_start+1);
          map::load_data::object_flags flags;
          while (flag_start != std::string::npos) {
            const std::string str = line_str.substr(flag_start, flag_end-flag_start);
            // это по идее один из флагов
            // его нужно сравнить со списком
            flags = map::apply_flag(str, flags);
            std::cout << "flag " << str << "\n";
            
            if (flag_end == std::string::npos) break;
            flag_start = line_str.find_first_not_of(" ", flag_end+1);
            flag_end = line_str.find_first_of(" ", flag_start+1);
          }
          
          switch (cur) {
            case current_data::map: {
              info.walls.back().flags = flags;
              break;
            }
            
            case current_data::complex_obj: {
              info.models.back().sides.back().flags = flags;
              break;
            }
            
            case current_data::entity_with_model: {
              info.objects.back().flags = flags;
              break;
            }
            
            case current_data::entity: {
              info.entities.back().flags = flags;
              break;
            }
            
            case current_data::count: {
              errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'f\' is considered outside of obj or entity data at line "+std::to_string(line_index));
              return false;
            }
          }
          break;
        }

        case 'a': {
          const size_t num_start = line_str.find_first_not_of(" ", index+1);
          //const size_t num_end = line_str.find_first_of(" ", num_start+1);
          if (!isdigit(line_str[num_start])) {
            errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
            return false;
          }
          const float ambient = atof(&line_str[num_start]);
          std::cout << "ambient " << ambient << "\n";

          if (cur != current_data::map && cur != current_data::complex_obj) {
            errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'a\' must be inside map or object data. Line "+std::to_string(line_index));
            return false;
          }
          
          if (info.walls.empty()) {
            throw std::runtime_error("empty walls line "+std::to_string(line_index));
          }
          
          if (cur == current_data::map) info.walls.back().ambient = ambient;
          if (cur == current_data::complex_obj) info.models.back().sides.back().ambient = ambient;
          
          break;
        }

        case 's': {
          const size_t state_start = line_str.find_first_not_of(" ", index+1);
          const size_t state_end = line_str.find_first_of(" ", state_start+1);
          const std::string state = line_str.substr(state_start, state_end-state_start);
          const utils::id state_id = utils::id::get(state);
          std::cout << "state " << state << "\n";
          
          switch (cur) {
            case current_data::map: {
              info.walls.back().state_id = state_id;
              break;
            }
            
            case current_data::complex_obj: {
              info.models.back().sides.back().state_id = state_id;
              break;
            }
            
            case current_data::entity_with_model: 
            case current_data::entity: 
            case current_data::count: {
              errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'s\' must be inside map or object data. Line "+std::to_string(line_index));
              return false;
            }
          }
          break;
        }

        // вот опять же нужно ли делать задание текстурки
        // или зафорсить текстурки только из состояния?
        case 'i': {
          // картинка состоит из двух вещей: id картинки + номер
          const size_t name_start = line_str.find_first_not_of(" ", index+1);
          const size_t name_end = line_str.find_first_of(" ", name_start+1);
          const size_t num_start = line_str.find_first_not_of(" ", name_end+1);
          //const size_t num_end = line_str.find_first_of(" ", num_start+1);
          const std::string image_name = line_str.substr(name_start, name_end-name_start);
          if (!isdigit(line_str[num_start])) {
            errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
            return false;
          }
          const size_t index = atol(&line_str[num_start]);
          std::cout << "image " << image_name << " " << index << "\n";
          
          switch (cur) {
            case current_data::map: {
              info.walls.back().image = {utils::id::get(image_name), index};
              break;
            }
            
            case current_data::complex_obj: {
              info.models.back().sides.back().image = {utils::id::get(image_name), index};
              break;
            }
            
            case current_data::entity_with_model:
            case current_data::entity: 
            case current_data::count: {
              errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'i\' must be inside map or object data. Line "+std::to_string(line_index));
              return false;
            }
          }
          break;
        }
        
        case 'l': {
          // ребра в графе, по идее нам неплохо было бы запомнить фейковость
          // с другой стороны мы ее можем восстановить
          size_t index1_start = line_str.find_first_not_of(" ", index+1);
          size_t index1_end = line_str.find_first_of(" ", index1_start+1);
          size_t index2_start = line_str.find_first_not_of(" ", index1_end+1);
          size_t index2_end = line_str.find_first_of(" ", index2_start+1);
          size_t index_fake_start = index2_end == std::string::npos ? SIZE_MAX : line_str.find_first_of(" ", index2_end+1);
          if (!isdigit(line_str[index1_start])) {
            errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
            return false;
          }
          
          if (!isdigit(line_str[index2_start])) {
            errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
            return false;
          }
          
          const uint32_t first = atoi(&line_str[index1_start]);
          const uint32_t second = atoi(&line_str[index2_start]);
          
          bool fake = false;
          if (index_fake_start != SIZE_MAX) {
            fake = line_str[index_fake_start] == 'f';
          }
          
          if (fake) {
            std::cout << "edge between " << first << " and " << second << " fake" << "\n";
          } else {
            std::cout << "edge between " << first << " and " << second << "\n";
          }
          
          // как отделить ребра между сложными объектами и картой?
          // во первых будут ли ребра у сложных объектов? можно ли на них передвигаться?
          // могут ли монстры ими воспользоваться? крайне не желательно конечно чтобы монстры всем этим могли пользоваться
          // монстры должны иметь возможность этим воспользоваться
          // у дверей есть состояния (открытая, закрытая)
          // в одном из этих состояний мы должны блокировать одни ребра, а в другом другие
          
          break;
        }
        
        case 'x': {
          // плоскости карты или объекта (face)
          // несколько индексов
          size_t index_start = line_str.find_first_not_of(" ", index+1);
          size_t index_end = line_str.find_first_of(" ", index_start+1);
          if (cur == current_data::map) {
            info.walls.emplace_back();
            const auto &prev_face = info.walls[info.walls.size()-2];
            info.walls.back().state_id = prev_face.state_id;
            info.walls.back().image = prev_face.image;
            info.walls.back().ambient = prev_face.ambient;
            info.walls.back().tag = UINT32_MAX;
            info.walls.back().flags = prev_face.flags;
          }
          
          if (cur == current_data::complex_obj) {
            info.models.back().sides.emplace_back();
            const auto &prev_face = info.models.back().sides[info.models.back().sides.size()-2];
            info.models.back().sides.back().state_id = prev_face.state_id;
            info.models.back().sides.back().image = prev_face.image;
            info.models.back().sides.back().ambient = prev_face.ambient;
            info.models.back().sides.back().tag = UINT32_MAX;
            info.models.back().sides.back().flags = prev_face.flags;
          }
          
          std::cout << " face ";
          while (index_start != std::string::npos) {
            // данные вида "y/z"
            if (!isdigit(line_str[index_start])) {
              errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
              return false;
            }
            
            const size_t index_coord_start = line_str.find_first_of("/", index_start);
            if (index_coord_start != std::string::npos && !isdigit(line_str[index_coord_start+1])) {
              errors.add(mark, resources::map_loader::ERROR_BAD_NUMERIC_DATA, "Bad numeric data at line "+std::to_string(line_index));
              return false;
            }
            
            const map::load_data::faces::idx i{
              static_cast<uint32_t>(atol(&line_str[index_start])),
              index_coord_start != std::string::npos ? static_cast<uint32_t>(atol(&line_str[index_coord_start+1])) : UINT32_MAX
            };
            switch (cur) {
              case current_data::map: {
                info.walls.back().indices.push_back(i);
                break;
              }
              
              case current_data::complex_obj: {
                info.models.back().sides.back().indices.push_back(i);
                break;
              }
              
              case current_data::entity_with_model:
              case current_data::entity: 
              case current_data::count: {
                errors.add(mark, resources::map_loader::ERROR_BAD_DATA_IN_MAP, "\'x\' must be inside map or object data. Line "+std::to_string(line_index));
                return false;
              }
            }
            
            std::cout << " vert " << i.vertex << " tex " << i.texture_coordinates;
            
            if (index_end == std::string::npos) break;
            index_start = line_str.find_first_not_of(" ", index_end+1);
            index_end = line_str.find_first_of(" ", index_start+1);
          }
          std::cout << "\n";
          
          uint32_t faces_count = 0;
          if (cur == current_data::map) faces_count = info.walls.back().indices.size();
          if (cur == current_data::complex_obj) faces_count = info.models.back().sides.back().indices.size();
          if (faces_count < 3) {
            errors.add(mark, resources::map_loader::ERROR_BAD_FACE_DATA, "Bad face data at line "+std::to_string(line_index));
            return false;
          }
          
          break;
        }
        
        // переносим к энтити
//         case 'm': {
//           const size_t index_start = line_str.find_first_not_of(" ", index+1);
//           const size_t index_end = line_str.find_first_of(" ", index_start+1);
//           const std::string model_name = line_str.substr(index_start, index_end-index_start);
//           const utils::id model_id = utils::id::get(model_name);
//           cur = current_data::entity_with_model;
//           std::cout << "model name " << model_name << "\n";
//           
//           bool finded = false;
//           for (size_t i = 0; i < info.models.size(); ++i) {
//             if (info.models[i].id == model_id) {
//               info.objects.emplace_back();
//               info.objects.back().model_index = i;
//               finded = true;
//               break;
//             }
//           }
//           
//           if (!finded) {
//             errors.add(mark, resources::map_loader::ERROR_COULD_NOT_FIND_MODEL_NAME, "Could not find object name at line "+std::to_string(line_index));
//             return false;
//           }
//           
//           break;
//         }
        
        case '#': break;
        
        default: {
          warnings.add(mark, resources::map_loader::WARNING_UNKNOWN_MAP_INFO_LINE_START, "Unknown line start at line: "+std::to_string(line_index));
        }
      }
    }
    
    return true;
  }
  
  namespace map {
    load_data::object_flags apply_flag(const std::string &flag, const load_data::object_flags &old_flag) {
      load_data::object_flags new_flag = old_flag;
      for (size_t i = 0; i < flags.size(); ++i) {
        if (flag == flags[i]) {
          new_flag.set(i, true);
          break;
        }
      }
      
      return new_flag;
    }
    
    load_data::object_flags::object_flags() : container(0) {}
    bool load_data::object_flags::baby() const {
      const uint32_t mask = 1 << OBJ_FLAG_BABY;
      return (container & mask) == mask;
    }
    
    bool load_data::object_flags::easy() const {
      const uint32_t mask = 1 << OBJ_FLAG_EASY;
      return (container & mask) == mask;
    }
    
    bool load_data::object_flags::medium() const {
      const uint32_t mask = 1 << OBJ_FLAG_MEDIUM;
      return (container & mask) == mask;
    }
    
    bool load_data::object_flags::hard() const {
      const uint32_t mask = 1 << OBJ_FLAG_HARD;
      return (container & mask) == mask;
    }
    
    bool load_data::object_flags::nightmare() const {
      const uint32_t mask = 1 << OBJ_FLAG_NIGHTMARE;
      return (container & mask) == mask;
    }
    
    bool load_data::object_flags::static_obj() const {
      return false;
    }
    
    bool load_data::object_flags::ambush() const {
      const uint32_t mask = 1 << OBJ_FLAG_AMBUSH;
      return (container & mask) == mask;
    }
    
    bool load_data::object_flags::has_rotation() const {
      const uint32_t mask = 1 << OBJ_FLAG_ROTATION;
      return (container & mask) == mask;
    }
    
    void load_data::object_flags::set(const uint32_t &index, const bool value) {
      if (index >= 5) return;
      const uint32_t mask = 1 << index;
      container = value ? container | mask : container & ~mask;
    }
    
    bool load_data::object_flags::exist_on_current_diff(const game::difficulty &diff) const {
      const uint32_t mask = 1 << static_cast<uint32_t>(diff);
      return (container & mask) == mask;
    }
    
    load_data::entity_data::entity_data() : pos{glm::uintBitsToFloat(UINT32_MAX), glm::uintBitsToFloat(UINT32_MAX), glm::uintBitsToFloat(UINT32_MAX)}, tag(UINT32_MAX) {}
    bool load_data::entity_data::valid() const {
      return glm::floatBitsToUint(pos[0]) != UINT32_MAX;
    }
    
    load_data::faces::faces() : image{utils::id(), UINT32_MAX}, ambient(1.0f), tag(UINT32_MAX) {}
    load_data::model::model() {}
    load_data::complex_object::complex_object() : pos{glm::uintBitsToFloat(UINT32_MAX), glm::uintBitsToFloat(UINT32_MAX), glm::uintBitsToFloat(UINT32_MAX)}, tag(UINT32_MAX) {}
    bool load_data::complex_object::valid() const {
      return glm::floatBitsToUint(pos[0]) != UINT32_MAX;
    }
  }
  
  namespace resources {
    map_loader::map_loader(const create_info &info) : parser("maps"), world(info.world), player_ptr(nullptr), entities(info.entities), states(info.states), images(info.images), device(info.device), current_index(UINT32_MAX), container(info.container) {
      if (container->vertices == nullptr) {
        container->vertices = device->create(yavf::BufferCreateInfo::buffer(100, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      }
    }
    
    bool map_loader::validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      const size_t errors_count = errors.size();
      for (size_t i = 0; i < loading_data.size(); ++i) {
        for (const auto &entity : loading_data.at(i)->entities) {
          auto res = entities->resource(entity.id);
          if (res == nullptr) errors.add(mark(), ERROR_COULD_NOT_FIND_ENTITY, "Could not find entity "+entity.id.name());
        }
        
        for (const auto &wall : loading_data.at(i)->walls) {
          // тут либо стейт либо текстурка (несколько стейтов)
          if (wall.state_id.valid()) {
            auto res = states->resource(wall.state_id);
            if (res == nullptr) errors.add(mark(), ERROR_COULD_NOT_FIND_STATE, "Could not find entity "+wall.state_id.name());
          }
          
          if (wall.image.id.valid()) {
            auto res = images->resource(wall.image.id);
            if (res == nullptr) errors.add(mark(), ERROR_COULD_NOT_FIND_IMAGE, "Could not find entity "+wall.image.id.name());
          }
        }
        
        for (const auto &model : loading_data.at(i)->models) {
          // тут нужно проверить текстурки
          // но при этом стейты тоже будут
          // а у вершин текстурки
          for (const auto &face : model.sides) {
            if (face.state_id.valid()) {
              auto res = states->resource(face.state_id);
              if (res == nullptr) errors.add(mark(), ERROR_COULD_NOT_FIND_STATE, "Could not find entity "+face.state_id.name());
            }
            
            if (face.image.id.valid()) {
              auto res = images->resource(face.image.id);
              if (res == nullptr) errors.add(mark(), ERROR_COULD_NOT_FIND_IMAGE, "Could not find entity "+face.image.id.name());
            }
          }
        }
      }
      
      return errors_count == errors.size();
    }
    
    bool map_loader::load(const utils::id &id) {
      if (current == id) return true;
      
      const auto ptr = loading_data.get(id);
      if (ptr == nullptr) return false;
      
      // выгружаем все я так понимаю
      for (size_t i = 0; i < world->size(); ++i) {
        auto ent = world->get_entity(i);
        if (ent == player_ptr) continue;
        world->destroy_entity(ent);
      }
      
      container->tagged_entities.clear();
      container->current = ptr->id;
      container->next_map = ptr->next;
      container->next_secret_map = ptr->next_secret;
      
      // нужно перезагрузить физику
      // как это делать? удалить сначала все энтити
      // затем посчитать максимум минимум, посчитать центр и экстент
      // переделать броад фазу, кажется в физике ничего особенного ничего не надо делать
      {
        const auto idx = ptr->walls[0].indices[0];
        simd::vec4 max = simd::vec4(ptr->vertices[idx.vertex].x, ptr->vertices[idx.vertex].y, ptr->vertices[idx.vertex].z, 1.0f), min = max;
        for (size_t i = 0; i < ptr->walls.size(); ++i) {
          for (size_t j = 0; j < ptr->walls[i].indices.size(); ++j) {
            const auto idx = ptr->walls[i].indices[j];
            const auto vec = simd::vec4(ptr->vertices[idx.vertex].x, ptr->vertices[idx.vertex].y, ptr->vertices[idx.vertex].z, 1.0f);
            max = simd::max(max, vec);
            min = simd::min(min, vec);
          }
        }
        
        const simd::vec4 center =          (max + min) / 2.0f;
        const simd::vec4 extent = simd::abs(max - min) / 2.0f;
        float arr[4];
        extent.storeu(arr);
        
        // как посчитать уровень разбиения?
        // раньше он у меня был от балды (5)
        // это значило что размер 4 го уровня был примерно 6.25 (при размере 100)
        const float size = std::max(arr[0], std::max(arr[1], arr[2]));
        uint32_t depth = 0;
        float current_division = size;
        do {
          current_division /= 2.0f;
          ++depth;
        } while (current_division >= 6.0f);
        
        const CPUOctreeBroadphaseParallel::OctreeCreateInfo broad_info{
          center,
          simd::vec4(size, size, size, 0.0f),
          depth
        };
        Global::get<PhysicsEngine>()->remake_broadphase(&broad_info);
      }
      
      std::vector<Vertex> vertices;
      
      for (size_t i = 0; i < ptr->walls.size(); ++i) {
        // как лучше сделать непосредственно задать сложности в которых может появиться моб или максимальную сложность?
        // в думе кажется первое (но там понятное дело через флаги)
        if (!ptr->walls[i].flags.exist_on_current_diff(container->difficulty)) continue;
        
        // стены предполагают что мы создадим гпу буфер
        auto wall_ent = create_wall(i, ptr->walls[i], ptr->vertices, ptr->tex_coords, vertices);
        if (ptr->walls[i].tag == UINT32_MAX) continue;
        container->tagged_entities.push_back(std::make_pair(ptr->walls[i].tag, wall_ent));
      }
      
      // где то еще нужно выставить все ребра
      
//       for (const auto &obj : ptr->objects) {
//         // в случае этих объектов нам нужно создать faces + 1 объект
//         // где будет основной объект + дочерние объекты по каждой плоскости основного объекта
//         // здесь нужно скорее создать еще один компонент отрисовки
//         if (!obj.flags.exist_on_current_diff(container->difficulty)) continue;
//         
//         auto obj_ent = create_complex_obj(obj, ptr->models[obj.model_index]);
//         if (obj.tag == UINT32_MAX) continue;
//         container->tagged_entities.push_back(std::make_pair(obj.tag, obj_ent));
//       }
      
      // у сложного объекта тоже есть ребра
      
      {
        // тут нужно перекопировать verts и globalIndicies в буферы
        yavf::Buffer stagingVert(device, yavf::BufferCreateInfo::buffer(vertices.size()*sizeof(Vertex), 
                                                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), 
                                VMA_MEMORY_USAGE_CPU_ONLY);
//         yavf::Buffer stagingIndices(device, yavf::BufferCreateInfo::buffer(globalIndicies.size()*sizeof(uint32_t), 
//                                                                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), 
//                                     VMA_MEMORY_USAGE_CPU_ONLY);
        
        memcpy(stagingVert.ptr(), vertices.data(), vertices.size()*sizeof(Vertex));
//         memcpy(stagingIndices.ptr(), globalIndicies.data(), globalIndicies.size()*sizeof(uint32_t));
        
        container->vertices->recreate(vertices.size()*sizeof(Vertex));
        //indices->recreate(globalIndicies.size()*sizeof(uint32_t));
        
        yavf::TransferTask* task = device->allocateTransferTask();
        
        task->begin();
        task->copy(&stagingVert, container->vertices);
        //task->copy(&stagingIndices, indices);
        task->end();
        
        task->start();
        task->wait();
        
        device->deallocate(task);
      }
      
      for (const auto &ent_data : ptr->entities) {
        if (!ent_data.flags.exist_on_current_diff(container->difficulty)) continue;
        
        const simd::vec4 pos = simd::vec4(ent_data.pos[0], ent_data.pos[1], ent_data.pos[2], 1.0f);
        const simd::vec4 rot = simd::vec4(ent_data.rot[0], ent_data.rot[1], ent_data.rot[2], 0.0f);
        auto creator = Global::get<game::entity_creators_container>()->get(ent_data.id);
        if (creator == nullptr) throw std::runtime_error("Could not find entity creator "+ent_data.id.name());
        auto ent = creator->create(nullptr, nullptr, pos, rot, simd::vec4(0.0f));
        // флаги? тэг?
        // некоторые флаги должны учитываться непосредственно здесь
        // после создания энтити мы можем его добавить в массив с тэгами
        
        if (ent_data.tag == UINT32_MAX) continue;
        container->tagged_entities.push_back(std::make_pair(ent_data.tag, ent));
      }
      
      if (player_ptr != nullptr) {
        auto trans = player_ptr->at<TransformComponent>(game::entity::transform);
        auto phys = player_ptr->at<PhysicsComponent>(game::entity::physics);
        
        // ставим игрока в точку спавна
        trans->pos() = simd::vec4(1.0f, 5.0f, 2.0f, 1.0f);
        phys->setVelocity(simd::vec4(0.0f, 0.0f, 0.0f, 0.0f));
      } else {
        player_ptr = create_player();
      }
      
      return true;
    }
    
    bool map_loader::unload(const utils::id &id) {
      //if (current != id) return false;
      // тут что?
      return false;
    }
    
    void map_loader::end() {
      
    }
    
    void map_loader::clear() {
      // тут кстати можно чистить карту
    }
    
    bool map_loader::load_obj(const std::string &path) {
      // делаем примерно то же самое что и в хардкодед лодерсе
      static const Type shape = Type::get("boxShape");
      {
        const RegisterNewShapeInfo info{
          {},
          {simd::vec4(0.5f, 0.5f, 0.5f, 0.0f)}
        };
        
        Global::get<PhysicsEngine>()->registerShape(shape, BBOX_TYPE, info);
      }

      PhysicsComponent::CreateInfo physInfo{
        {
          {0.0f, 0.0f, 0.0f, 0.0f},
          7.0f, 80.0f, 0.0f, 0.0f
        },
        {
          PhysicsType(true, BBOX_TYPE, true, false, true, true),
          PLAYER_COLLISION_TYPE,     // collisionGroup
          player_collision_filter,     // collisionFilter
          player_trigger_filter,     // collisionTrigger

          0.5f,  // stairHeight
          //40.0f, // acceleration
          1.0f,  // overbounce
          4.0f,  // groundFriction

          0.0f,  // radius
          1.0f,

          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,

          //"boxShape"
          shape
        },
        nullptr
      };
      
      {
        player_ptr = create_player();
      }
      
      static const utils::id test_entity = utils::id::get("test_entity1");
      static const utils::id zombie = utils::id::get("zombie");
      static const utils::id test_item = utils::id::get("tmr_test_item_entity");
      const bool ret1 = entities->load(test_entity);
      if (!ret1) throw std::runtime_error("Could not load entity type test_entity1");
      const bool ret2 = entities->load(zombie);
      if (!ret2) throw std::runtime_error("Could not load entity type zombie");
      const bool ret3 = entities->load(test_item);
      if (!ret3) throw std::runtime_error("Could not load entity type zombie");
      
      PRINT("started entity creation")
      
      {
        const size_t objCount = 5000;
                  
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(-99,99);
        //std::uniform_real_distribution<> distY(0,99);
        for (size_t i = 0; i < objCount; ++i) {
          const simd::vec4 pos = simd::vec4(static_cast<float>(dist(gen)), static_cast<float>(dist(gen)), static_cast<float>(dist(gen)), 1.0f);
          const simd::vec4 rot = simd::vec4(0.0f, 1.0f, 0.0f, 0.0f);
          Global::get<game::entity_creators_container>()->get(test_entity)->create(nullptr, nullptr, pos, rot, simd::vec4(0.0f));
        }
        
        {
          const simd::vec4 pos = simd::vec4(0.0f, 1.0f, 0.0f, 1.0f);
          const simd::vec4 rot = simd::vec4(0.0f, 0.0f, 1.0f, 0.0f);
          auto zombie_ent = Global::get<game::entity_creators_container>()->get(test_entity)->create(nullptr, nullptr, pos, rot, simd::vec4(0.0f));
    //       auto g = zombie->get<GraphicComponent>();
    //       ASSERT(g.valid());
    //       ASSERT(zombie->get<AnimationComponent>().valid());
          //ASSERT(zombie->get<StateController>().valid());
          // логично что здесь будут UINT32_MAX значения, у меня не задана текстурка явно
    //       PRINT(std::to_string(g->getTexture().image.index)+" "+std::to_string(g->getTexture().image.layer))
        }
        
        {
          const simd::vec4 pos = simd::vec4(2.0f, 1.0f, 0.0f, 1.0f);
          const simd::vec4 rot = simd::vec4(0.0f, 0.0f, 1.0f, 0.0f);
          auto test_item_ent = Global::get<game::entity_creators_container>()->get(test_item)->create(nullptr, nullptr, pos, rot, simd::vec4(0.0f));
        }
      }
      
      //const std::string path = Global::getGameDir() + "models/box4.obj";
      
      tinyobj::attrib_t attrib;
      std::vector<tinyobj::shape_t> shapes;
      std::vector<tinyobj::material_t> materials;

      // в будущем тут у нас должен быть мой собственный загрузчик
      
      std::string err;
      std::string warn;
      const bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), nullptr, false);

      if (!err.empty()) { // `err` may contain warning message.
        std::cerr << err << std::endl;
      }

      if (!ret) {
        throw std::runtime_error("Could not load map");
      }
      
      physInfo.physInfo.collisionFilter = wall_collision_filter;
      physInfo.physInfo.collisionTrigger = wall_trigger_filter;
      physInfo.physInfo.collisionGroup = WALL_COLLISION_TYPE;
      physInfo.physInfo.type = PhysicsType(false, POLYGON_TYPE, true, false, true, true);
      
      std::vector<Vertex> verts;
      std::vector<uint32_t> globalIndicies;
      // Loop over shapes
      for (size_t s = 0; s < shapes.size(); ++s) {
        size_t index_offset = 0;
        
        std::cout << "Indices: " << shapes[s].mesh.indices.size() << "\n";
        std::cout << "Vertices: " << attrib.vertices.size() / 3 << "\n";
        std::cout << "Num face vertices: " << shapes[s].mesh.num_face_vertices.size() << "\n";
        
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
          const size_t fv = shapes[s].mesh.num_face_vertices[f];
    //       PRINT("started face "+std::to_string(f))
          
          RegisterNewShapeInfo shapeInfo{};
          
          if (fv < 3) {
            // является ли отсутствие нормального фейса критической ошибкой?
            // наверное нет, нужно их только пропускать
            Global::console()->printW("Bad face " + std::to_string(f) + " in the map");
            //throw std::runtime_error("Bad face in map");
            index_offset += fv;
            continue;
          }
          
          for (size_t i = 0; i < fv; ++i) {
            // собираем verts
            // + собираем индексы
            
            tinyobj::index_t idx = shapes[s].mesh.indices[index_offset+i];
            const simd::vec4 v = simd::vec4(attrib.vertices[3*idx.vertex_index+0], attrib.vertices[3*idx.vertex_index+1], attrib.vertices[3*idx.vertex_index+2], 1.0f);
            shapeInfo.points.push_back(v);
          }
          
          shapeInfo.faces.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
          // где то еще вычисляем нормаль
          for (size_t i = 0; i < shapeInfo.points.size(); i++) {
            const size_t j = (i+1) % shapeInfo.points.size();
            const size_t k = (i+2) % shapeInfo.points.size();

            const simd::vec4 p = shapeInfo.points[j] - shapeInfo.points[i];
            const simd::vec4 q = shapeInfo.points[k] - shapeInfo.points[i];
            const simd::vec4 normal = simd::normalize(simd::cross(p, q));
            
            float arr[4];
            normal.store(arr);
            if (!((arr[0] != arr[0] || arr[1] != arr[1] || arr[2] != arr[2]) ||
                  (arr[0] == 0.0f && arr[1] == 0.0f && arr[2] == 0.0f))) {
              shapeInfo.faces.back() = simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(0));
              break;
            }
          }
          
          simd::vec4 x;
          simd::vec4 y;
          
          const simd::vec4 normal = shapeInfo.faces.back();
          float normalArr[4];
          normal.storeu(normalArr);
            
          if (fast_fabsf(normalArr[0]) < EPSILON && fast_fabsf(normalArr[1]) < EPSILON) {
            x = simd::vec4(1.0f, 0.0f, 0.0f, 0.0f);
            y = simd::vec4(0.0f, 1.0f, 0.0f, 0.0f);
          } else {
            x = simd::normalize(simd::vec4(-normalArr[1], normalArr[0], 0.0f, 0.0f));
            y = simd::normalize(simd::vec4(-normalArr[0]*normalArr[2], -normalArr[1]*normalArr[2], normalArr[0]*normalArr[0] + normalArr[1]*normalArr[1], 0.0f));
          }
          
          const simd::vec4 v1 = shapeInfo.points[0];
          for (size_t i = 0; i < shapeInfo.points.size(); ++i) {
            if (i == 0) {
              const Vertex v{
                v1,
                simd::vec4(normalArr[0], normalArr[1], normalArr[2], glm::uintBitsToFloat(f)),
                glm::vec2(0.0f, 0.0f)
              };
              
              verts.push_back(v);
              globalIndicies.push_back(verts.size()-1);
            } else {
              const float a = simd::dot(x, shapeInfo.points[i]-v1);
              const float b = simd::dot(y, shapeInfo.points[i]-v1);
              
              const Vertex v{
                shapeInfo.points[i],
                simd::vec4(normalArr[0], normalArr[1], normalArr[2], glm::uintBitsToFloat(f)),
                glm::vec2(a, b)
              };
              
              verts.push_back(v);
              globalIndicies.push_back(verts.size()-1);
            }
            
            const size_t j = (i+1) % shapeInfo.points.size();

            const simd::vec4 firstPoint = shapeInfo.points[i];
            const simd::vec4 secondPoint = shapeInfo.points[j];

            const simd::vec4 side = simd::cross(secondPoint - firstPoint, shapeInfo.faces[0]);
            const simd::vec4 normSide = simd::normalize(side);
            float arr[4];
            normSide.store(arr);

            shapeInfo.faces.emplace_back(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i));
            // нужно еще написать примерно такой же код для объектов (например, двери)
          }
          
          simd::vec4 shapeMin = shapeInfo.points[0];
          simd::vec4 shapeMax = shapeInfo.points[0];
          simd::vec4 center;
          for (size_t i = 0; i < shapeInfo.points.size(); ++i) {
            center += shapeInfo.points[i];
            shapeMin = simd::min(shapeMin, shapeInfo.points[i]);
            shapeMax = simd::max(shapeMax, shapeInfo.points[i]);
          }
          center /= shapeInfo.points.size();
          
          const std::string shapeName = "WallShape " + std::to_string(f);
          const Type shapeType = Type::get(shapeName);
          Global::get<PhysicsEngine>()->registerShape(shapeType, POLYGON_TYPE, shapeInfo);
          
          static const utils::id state_id = utils::id::get("tmr_decor1_state");
          auto state = Global::get<game::states_container>()->get(state_id);
          ASSERT(state != nullptr);
          
          float center_arr[4];
          center.storeu(center_arr);
          
          const load_obj_wall_create_info info{
            shapeType,
            state,
            static_cast<uint32_t>(index_offset),
            static_cast<uint32_t>(fv),
            static_cast<uint32_t>(f),
            {center_arr[0], center_arr[1], center_arr[2], center_arr[3]},
            {normalArr[0], normalArr[1], normalArr[2], normalArr[3]}
          };
          
          auto wall_ent = create_wall(info);
          auto vertex = wall_ent->at<components::vertex>(game::wall::vertex);
          
          for (size_t i = 0; i < Global::get<yacs::world>()->count_components<components::vertex>(); ++i) {
            auto vert = Global::get<yacs::world>()->get_component<components::vertex>(i);
            auto vert_phys = vert->entity()->get<PhysicsComponent>();
            const uint32_t pointsCount = vert_phys->getObjectShapePointsSize();
            const simd::vec4* points = vert_phys->getObjectShapePoints();
            
            simd::vec4 edgePoints[2];
            uint8_t founded = 0;
            for (uint32_t j = 0; j < pointsCount; ++j) {
              for (uint32_t l = 0; l < shapeInfo.points.size(); ++l) {
                if (founded > 1) break;
                
                const size_t nextJ = (j+1)%pointsCount;
                const size_t nextL = (l+1)%shapeInfo.points.size();
                
                const bool config1 = (simd::distance2(points[j], shapeInfo.points[l]) < EPSILON) && (simd::distance2(points[nextJ], shapeInfo.points[nextL]) < EPSILON);
                const bool config2 = (simd::distance2(points[j], shapeInfo.points[nextL]) < EPSILON) && (simd::distance2(points[nextJ], shapeInfo.points[l]) < EPSILON);
                
                if (config1) {
                  edgePoints[0] = points[j];
                  edgePoints[1] = points[nextJ];
                  founded = 10;
                }
                
                if (config2) {
                  edgePoints[0] = points[j];
                  edgePoints[1] = points[nextJ];
                  founded = 10;
                }
              }
              
              if (founded > 1) break;
            }
            
            if (founded > 1) {
              glm::vec4 dir;
              simd::vec4 simd_dir = simd::normalize(edgePoints[1] - edgePoints[0]);
              simd_dir.storeu(&dir.x);
              
              glm::vec4 aP;
              glm::vec4 bP;
              edgePoints[0].storeu(&aP.x);
              edgePoints[1].storeu(&bP.x);
              
              auto edge = Global::get<graph::container>()->create(vertex.get(), vert.get(), edgePoints[0], edgePoints[1]);
              edge->length = simd::distance(vertex->center(), vert->center());
              edge->angle = getAngle(vertex->normal(), vert->normal());
              edge->height = 0.0f;
              vertex->add_edge(edge);
              vert->add_edge(edge);
            }
          }
          
          index_offset += fv;
        }
      }
      
      // нужно создать сложный объект
      ASSERT(shapes[0].mesh.num_face_vertices.size() > 0);
      create_test_complex_obj(shapes[0].mesh.num_face_vertices.size(), verts);
      
      std::cout << "Created " << Global::get<yacs::world>()->count_components<components::vertex>() << " vertices" << "\n";
      const size_t edges = Global::get<graph::container>()->edges.size();
      std::cout << "Created " << edges << " edges" << "\n";
      
      // мы еще должны найти fake edges
      // fake edge работают неверно (хотя возможно они создаются неверно)
      for (size_t i = 0; i < Global::get<yacs::world>()->count_components<components::vertex>(); ++i) {
        auto vert1 = Global::get<yacs::world>()->get_component<components::vertex>(i).get();

        for (size_t j = 0; j < vert1->degree(); ++j) {
          size_t edge1_index = j;
          auto edge = vert1->next_edge(edge1_index);

          if (edge->angle < PASSABLE_ANGLE) continue;

          auto vert2 = edge->vertices.first == vert1 ? edge->vertices.second : edge->vertices.first;

          for (size_t e = 0; e < vert2->degree(); ++e) {
            size_t edge2_index = e;
            auto secondEdge = vert2->next_edge(edge2_index);

            if (secondEdge->is_fake()) continue;
            if (secondEdge->vertices.first == vert1 || secondEdge->vertices.second == vert1) continue;

            auto another = secondEdge->vertices.first == vert1 ? secondEdge->vertices.second : secondEdge->vertices.first;
            if (vert1->has_edge(another) != SIZE_MAX) continue;

            const float angle = getAngle(vert1->normal(), another->normal());
            if (angle >= PASSABLE_ANGLE) continue;

            simd::vec4 left1, right1, left2, right2;
            edge->seg.left_right(vert1->center(), vert1->normal(), left1, right1);
            secondEdge->seg.left_right(vert1->center(), vert1->normal(), left2, right2);
            float d1 = simd::distance(left1, left2);
            float d2 = simd::distance(right1, right2);

            const float width1 = edge->seg.distance(); // simd::distance(edge->getSegment().a, edge->getSegment().b);
            const float width2 = secondEdge->seg.distance(); // simd::distance(secondEdge->getSegment().a, secondEdge->getSegment().b);
    
            auto new_edge = Global::get<devils_engine::graph::container>()->create(vert1, another, d1 > d2 ? right1 : left1, d1 > d2 ? left1 : right1);
            new_edge->length = simd::distance(vert1->center(), edge->seg.closest_point(vert1->center())) + simd::distance(another->center(), secondEdge->seg.closest_point(another->center()));
            new_edge->angle = secondEdge->angle;
            new_edge->height = d1 > d2 ? d2 : d1;
            vert1->add_edge(new_edge);
            another->add_edge(new_edge);
            
            // вообще мне наверное пригодится перерасчитывать расстояние для фейковых ребер
          }
        }
      }
      
      std::cout << "Created " << (Global::get<devils_engine::graph::container>()->edges.size() - edges) << " fake edges" << "\n";
      
      // тут нужно перекопировать verts и globalIndicies в буферы
      yavf::Buffer stagingVert(device, yavf::BufferCreateInfo::buffer(verts.size()*sizeof(Vertex), 
                                                                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), 
                              VMA_MEMORY_USAGE_CPU_ONLY);
      yavf::Buffer stagingIndices(device, yavf::BufferCreateInfo::buffer(globalIndicies.size()*sizeof(uint32_t), 
                                                                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), 
                                  VMA_MEMORY_USAGE_CPU_ONLY);
      
      memcpy(stagingVert.ptr(), verts.data(), verts.size()*sizeof(Vertex));
      memcpy(stagingIndices.ptr(), globalIndicies.data(), globalIndicies.size()*sizeof(uint32_t));
      
      container->vertices->recreate(verts.size()*sizeof(Vertex));
      //indices->recreate(globalIndicies.size()*sizeof(uint32_t));
      
      yavf::TransferTask* task = device->allocateTransferTask();
      
      task->begin();
      task->copy(&stagingVert, container->vertices);
      //task->copy(&stagingIndices, indices);
      task->end();
      
      task->start();
      task->wait();
      
      device->deallocate(task);
      // это все по идее
      return true;
    }
    
    // между уровнями у нас будет выводиться стата - это особый уровень, затем он будет вызывать эту функцию
    bool map_loader::change_map(const utils::id &episod, const utils::id &map_id, const game::difficulty &diff) {
      if (current == map_id) return true;
      
      container->episod = episod;
      container->current = map_id;
      container->difficulty = diff;
      return load(map_id);
    }
    
    uint32_t get_tag(const game::map_data_container* map_data, const yacs::entity* ent) {
      for (size_t i = 0; i < map_data->tagged_entities.size(); ++i) {
        if (map_data->tagged_entities[i].second == ent) return map_data->tagged_entities[i].first;
      }
      
      return UINT32_MAX;
    }
    
    void map_loader::save_map(const std::string &path) {
      // создаем текстовый файл с описанием карты
      // нам нужно передать описание, как?
      // это по идее обратная функция от load
      // то есть обходим все энтити, запоминаем у них позиции
      const auto wall_id = utils::id::get("wall");
      const auto obj_id = utils::id::get("complex_object");
      
      // нужно взять графические вершины (то есть вершины + текстурные координаты)
      yavf::Buffer stagingVert(device, yavf::BufferCreateInfo::buffer(container->vertices->info().size,
                                                                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), 
                              VMA_MEMORY_USAGE_CPU_ONLY);
      
      {
        yavf::TransferTask* task = device->allocateTransferTask();
      
        task->begin();
        task->copy(container->vertices, &stagingVert);
        //task->copy(&stagingIndices, indices);
        task->end();
        
        task->start();
        task->wait();
        
        device->deallocate(task);
      }
      
      std::fstream file(path);
      file << "o map" << "\n";
      
      const Vertex* vert_ptr = reinterpret_cast<const Vertex*>(stagingVert.ptr());
      // заполняем стены
      for (size_t i = 0; i < world->size(); ++i) {
        auto ent = world->get_entity(i);
        auto type_info = ent->at<components::type_info>(game::entity::type_info);
        if (type_info->id != wall_id) continue;
        //const uint32_t tag = get_tag(container, ent);
        
        auto graphics = ent->at<components::indexed_graphics>(game::entity::graphics);
        for (uint32_t i = 0; i < graphics->count; ++i) {
          const uint32_t index = i + graphics->offset;
          const auto &vert = vert_ptr[index];
          file << "p " << vert.pos.arr[0] << " " << vert.pos.arr[1] << " " << vert.pos.arr[2] << "\n";
          file << "c " << vert.texCoord.x << " " << vert.texCoord.y << "\n";
        }
      }
      
      for (size_t i = 0; i < world->size(); ++i) {
        auto ent = world->get_entity(i);
        auto type_info = ent->at<components::type_info>(game::entity::type_info);
        if (type_info->id != wall_id) continue;
        const uint32_t tag = get_tag(container, ent);
        
        file << "x ";
        auto graphics = ent->at<components::indexed_graphics>(game::entity::graphics);
        for (uint32_t i = 0; i < graphics->count; ++i) {
          const uint32_t index = i + graphics->offset;
          file << index << "/" << index << " ";
        }
        file << "\n";
        if (tag != UINT32_MAX) file << "t " << tag << "\n";
        //const float ambient = graphics->
        // флаги
      }
      
      file << "\n";
      
      // сложные объекты
      for (size_t i = 0; i < world->size(); ++i) {
        auto ent = world->get_entity(i);
        auto type_info = ent->at<components::type_info>(game::entity::type_info);
        if (type_info->id != obj_id) continue;
//         auto trans = ent->at<TransformComponent>(game::entity::transform);
        auto graphics = ent->at<components::complex_indices_graphics>(game::entity::graphics);
        
        //file << "o " << << "\n";
        for (const auto model : graphics->model_faces) {
          for (uint32_t i = 0; i < model.count; ++i) {
            const uint32_t index = model.offset + i;
            const auto &vert = vert_ptr[index];
            file << "p " << vert.pos.arr[0] << " " << vert.pos.arr[1] << " " << vert.pos.arr[2] << "\n";
            file << "c " << vert.texCoord.x << " " << vert.texCoord.y << "\n";
          }
        }
        
        for (const auto model : graphics->model_faces) {
          file << "x ";
          for (uint32_t i = 0; i < model.count; ++i) {
            const uint32_t index = model.offset + i;
            file << index << "/" << index << " ";
          }
          file << "\n";
        }
        
        // эмбиент
        
        file << "\n";
      }
      
      for (size_t i = 0; i < world->size(); ++i) {
        auto ent = world->get_entity(i);
        auto type_info = ent->at<components::type_info>(game::entity::type_info);
        if (type_info->id != obj_id) continue; // нужно изменить проверку, type_info->id может быть другим
        auto trans = ent->at<TransformComponent>(game::entity::transform);
//         auto graphics = ent->at<components::complex_indices_graphics>(game::entity::graphics);
        
        float pos[4];
        trans->pos().storeu(pos);
        
        const uint32_t tag = get_tag(container, ent);
        
        auto states = ent->at<components::states>(game::entity::states);
        
        //auto rotation = ent->at<components::rotation>();
        
        file << "e " << type_info->id.name() << "\n";
//         file << "d " << rot[0] << " " << rot[1] << " " << rot[2] << "\n";
        if (tag != UINT32_MAX) file << "t " << tag << "\n";
        file << "s " << states->current->id.name() << "\n";
        //file << "f " << << "\n"; // где это дело хранить + при редактировании нужно грузить все
        file << "p " << pos[0] << " " << pos[1] << " " << pos[2] << "\n";
        //if (rotation.valid()) file << "r " << << "\n";
        file << "\n";
      }
      
      //file << "\n";
      
      // монстры и декор
      for (size_t i = 0; i < world->size(); ++i) {
        auto ent = world->get_entity(i);
        auto type_info = ent->at<components::type_info>(game::entity::type_info);
        if (type_info->id == wall_id || type_info->id == obj_id) continue;
        auto trans = ent->at<TransformComponent>(game::entity::transform);
        float rot[4];
        float pos[4];
        trans->rot().storeu(rot);
        trans->pos().storeu(pos);
        
        const uint32_t tag = get_tag(container, ent);
        
        auto states = ent->at<components::states>(game::entity::states);
        
        // нет данных о сложности, с другой стороны это не должны быть основные объекты
        // а должны быть спавн точки
        file << "e " << type_info->id.name() << "\n";
        file << "d " << rot[0] << " " << rot[1] << " " << rot[2] << "\n";
        if (tag != UINT32_MAX) file << "t " << tag << "\n";
        file << "s " << states->current->id.name() << "\n";
        //file << "f " << << "\n"; // где это дело хранить + при редактировании нужно грузить все
        file << "p " << pos[0] << " " << pos[1] << " " << pos[2] << "\n";
        file << "\n";
      }
      
      // нужно разделить спаунер от непосредственно монстров и декора
      // неплохо было бы рисовать спаунер полупрозрачным
      // но на самом деле это не то чтобы обязательно
      // нужно просто разделить клиент игры от редактора
    }
    
    utils::id map_loader::current_map() const {
      return current;
    }
    
    yacs::entity* map_loader::player() const {
      return player_ptr;
    }
    
    utils::id map_loader::check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, map::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      bool has_id = false, has_path = false;
      // тут мы скорее всего парсим карту сразу
      //
      
      const size_t errors_count = errors.size();
      for (auto itr = data.begin(); itr != data.end(); ++itr) {
        if (itr.value().is_string() && itr.key() == "id") {
          has_id = true;
          info.id = utils::id::get(itr.value().get<std::string>());
          continue;
        }
        
        if (itr.value().is_string() && itr.key() == "path") {
          has_path = true;
          const std::string path = itr.value().get<std::string>();
          info.path = path;
          const bool ret = parse_map(path_prefix+path, mark, info, errors, warnings);
          if (!ret) return utils::id();
          continue;
        }
        
        if (itr.value().is_string() && itr.key() == "next_map") {
          info.next = utils::id::get(itr.value().get<std::string>());
          continue;
        }
        
        if (itr.value().is_string() && itr.key() == "next_map_secret") {
          info.next_secret = utils::id::get(itr.value().get<std::string>());
          continue;
        }
      }
      
      if (!has_id) {
        errors.add(mark, ERROR_COULD_NOT_FIND_ID, "Could not find map id");
        return utils::id();
      }
      
      if (!has_path) {
        errors.add(mark, ERROR_COULD_NOT_PATH, "Could not find map "+info.id.name()+" path");
      }
      
      return errors_count == errors.size() ? info.id : utils::id();
    }
    
    // вершины для графики!
    yacs::entity* map_loader::create_wall(const size_t &index, const map::load_data::faces &face, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec2> &tex_coords, std::vector<Vertex> &verts) {
      RegisterNewShapeInfo shapeInfo{};
      for (size_t i = 0; i < face.indices.size(); ++i) {
        const auto vert = vertices[face.indices[i].vertex];
        shapeInfo.points.push_back(simd::vec4(vert.x, vert.y, vert.z, 1.0f));
      }
      
      shapeInfo.faces.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
      for (size_t i = 0; i < shapeInfo.points.size(); i++) {
        const size_t j = (i+1) % shapeInfo.points.size();
        const size_t k = (i+2) % shapeInfo.points.size();
        
//         PRINT_VEC4("i", shapeInfo.points[i])
//         PRINT_VEC4("k", shapeInfo.points[k])
//         PRINT_VEC4("j", shapeInfo.points[j])

        const simd::vec4 p = shapeInfo.points[j] - shapeInfo.points[i];
        const simd::vec4 q = shapeInfo.points[k] - shapeInfo.points[i];
        const simd::vec4 normal = simd::normalize(simd::cross(p, q));
        
        float arr[4];
        normal.store(arr);
        if (!((arr[0] != arr[0] || arr[1] != arr[1] || arr[2] != arr[2]) ||
              (arr[0] == 0.0f && arr[1] == 0.0f && arr[2] == 0.0f))) {
          shapeInfo.faces.back() = simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(0));
          break;
        }
      }
      
      const auto norm = shapeInfo.faces[0];
      for (size_t i = 0; i < face.indices.size(); ++i) {
        const auto vert = vertices[face.indices[i].vertex];
        const auto coord = tex_coords[face.indices[i].texture_coordinates];
        verts.push_back({
          basic_vec4(vert.x, vert.y, vert.z, 1.0f),
          norm,
          coord
        });
      }
      
      for (size_t i = 0; i < shapeInfo.points.size(); ++i) {
        const size_t j = (i+1) % shapeInfo.points.size();

        const simd::vec4 firstPoint = shapeInfo.points[i];
        const simd::vec4 secondPoint = shapeInfo.points[j];

        const simd::vec4 side = simd::cross(secondPoint - firstPoint, shapeInfo.faces[0]);
        const simd::vec4 normSide = simd::normalize(side);
        float arr[4];
        normSide.store(arr);

        shapeInfo.faces.emplace_back(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i));
      }
      
      simd::vec4 shapeMin = shapeInfo.points[0];
      simd::vec4 shapeMax = shapeInfo.points[0];
      simd::vec4 center;
      for (size_t i = 0; i < shapeInfo.points.size(); ++i) {
        center += shapeInfo.points[i];
        shapeMin = simd::min(shapeMin, shapeInfo.points[i]);
        shapeMax = simd::max(shapeMax, shapeInfo.points[i]);
      }
      center /= shapeInfo.points.size();
      
      const std::string shapeName = "wall_shape_" + std::to_string(index);
      const Type shapeType = Type::get(shapeName);
      Global::get<PhysicsEngine>()->registerShape(shapeType, POLYGON_TYPE, shapeInfo);
      
      const PhysicsComponent::CreateInfo phys_info {
        {},
        {
          PhysicsType(false, POLYGON_TYPE, true, false, true, true),
          WALL_COLLISION_TYPE,
          wall_collision_filter,
          wall_trigger_filter,
          0.0f,
          1.0f,
          4.0f,
          0.0f,
          1.0f,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          shapeType
        },
        nullptr
      };
      
      auto default_state = Global::get<game::states_container>()->get(face.state_id);
      if (default_state == nullptr) throw std::runtime_error("default_state == nullptr");
      
      auto ent = world->create_entity();
      
      //auto info = create_info(ent, utils::id::get(shapeName), nullptr);
      auto info = create_type_info(ent, utils::id::get("wall"), nullptr); // лучше чтоб id у таких объектов был одинаковым
      //auto user_data = create_usr_data(ent);
      ent->set(yacs::component_handle<TransformComponent>(nullptr));
      ent->set(yacs::component_handle<InputComponent>(nullptr));
      auto phys = create_physics(ent, &phys_info);
      phys->setUserData(ent);
      auto graphics = create_graphics(ent, face.indices[0].vertex, face.indices.size(), index); // ???
      auto states = create_states(ent, default_state);
      auto vertex = create_vertex(ent, center, shapeInfo.faces[0]);
      
      (void)info;
      (void)graphics;
      (void)states;
      (void)vertex;
      
      return ent;
    }
    
    yacs::entity* map_loader::create_wall(const load_obj_wall_create_info &create_info) {
      const PhysicsComponent::CreateInfo physInfo{
        {
          {0.0f, 0.0f, 0.0f, 0.0f},
          7.0f, 80.0f, 0.0f, 0.0f
        },
        {
          PhysicsType(false, POLYGON_TYPE, true, false, true, true),
          WALL_COLLISION_TYPE,     // collisionGroup
          wall_collision_filter,     // collisionFilter
          wall_trigger_filter,     // collisionTrigger

          0.0f,  // stairHeight
          //40.0f, // acceleration
          1.0f,  // overbounce
          4.0f,  // groundFriction

          0.0f,  // radius
          1.0f,

          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,

          //"boxShape"
          create_info.shape_type
        },
        nullptr
      };
      
      static const utils::id wall_id = utils::id::get("wall");
      // будем видимо создавать энтити прямо здесь
      auto wall_ent = Global::get<yacs::world>()->create_entity();
      auto info = wall_ent->add<components::type_info>();
      info->created_ability = nullptr;
      info->ent = wall_ent;
      info->id = wall_id;
      info->parent = nullptr;
      // состояния для стен я не могу явно указать, точнее могу но для каждой стены по отдельности в информации о карте
      // но тут тип такой что я вряд ли смогу правильно удалить
      // в будущем нужно будет сделать тип для стен?
      info->states = nullptr;
      info->states_count = 0;
      
      ASSERT(wall_ent->at<components::type_info>(game::entity::type_info).valid());
      
      wall_ent->set(yacs::component_handle<TransformComponent>(nullptr));
      wall_ent->set(yacs::component_handle<InputComponent>(nullptr));
      
//           auto usr_data = wall_ent->add<UserDataComponent>();
//           usr_data->entity = wall_ent;
      auto wall_phys = wall_ent->add<PhysicsComponent>(physInfo);
      wall_phys->setUserData(wall_ent);
      
      ASSERT(wall_ent->at<PhysicsComponent>(game::entity::physics).valid());
      
      auto graphics = wall_ent->add<components::indexed_graphics>();
      graphics->ent = wall_ent;
      graphics->offset = create_info.offset;
      graphics->count = create_info.count;
      graphics->index = create_info.index;
      
      ASSERT(wall_ent->at<components::indexed_graphics>(game::entity::graphics).valid());
      
      auto states = wall_ent->add<components::states>(); // состояние по умолчанию?
      states->current = create_info.state; // тут нужно что нибудь указать
      states->current_time = SIZE_MAX;
      states->ent = wall_ent;
      // звук скорее всего будет просто компонентом, при всех взаимодействиях будем вызывать функцию из компонента, 
      // так как мы явно определяем все функции взаимодействий, то материал нам может и не пригодиться
//       wall_ent->set(yacs::component_handle<SoundComponent>(nullptr));
      auto vertex = wall_ent->add<components::vertex>(components::vertex::create_info{wall_ent, simd::vec4(create_info.center), simd::vec4(create_info.normal)});
      ASSERT(wall_ent->at<components::vertex>(game::wall::vertex).valid());
      (void)vertex;
      
      auto decal_container = wall_ent->add<components::decals_container>(wall_ent);
      ASSERT(wall_ent->at<components::decals_container>(game::wall::decal_container).valid());
      (void)decal_container;
      
      return wall_ent;
    }
    
    yacs::entity* map_loader::create_complex_obj(const map::load_data::complex_object &obj, const map::load_data::model &model, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec2> &tex_coords, std::vector<Vertex> &verts) {
      
    }
    
    yacs::entity* map_loader::create_player() {
      auto ent = world->create_entity();
      auto type_info = ent->add<components::type_info>();
      type_info->ent = ent;
      type_info->id = utils::id::get("player"); // потом нужно заменить на id персонажа игрока
      type_info->parent = nullptr;
      type_info->created_ability = nullptr;
      type_info->states = nullptr;
      type_info->states_count = 0;
      type_info->bit_container.set_player(true);
      
      auto trans = ent->add<TransformComponent>(simd::vec4(1.0f, 5.0f, 2.0f, 1.0f),
                                                simd::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                                                simd::vec4(1.0f, 1.0f, 1.0f, 0.0f)).get();
      
      auto input = ent->add<UserInputComponent>(UserInputComponent::CreateInfo{trans}).get();

      const Type shape = Type::get("boxShape");
      PhysicsComponent::CreateInfo physInfo{
        {
          {0.0f, 0.0f, 0.0f, 0.0f},
          7.0f, 80.0f, 0.0f, 0.0f
        },
        {
          PhysicsType(true, BBOX_TYPE, true, false, true, true),
          PLAYER_COLLISION_TYPE,     // collisionGroup
          player_collision_filter,     // collisionFilter
          player_trigger_filter,

          0.5f,  // stairHeight
          //40.0f, // acceleration
          1.0f,  // overbounce
          4.0f,  // groundFriction

          0.0f,  // radius
          1.0f,

          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,

          //"boxShape"
          shape
        },
        nullptr
      };
      
      physInfo.physInfo.inputIndex = input->inputIndex;
      physInfo.physInfo.transformIndex = trans->index();
      auto phys = ent->add<PhysicsComponent>(physInfo);
      phys->setUserData(ent);
      
      ent->set(yacs::component_handle<components::sprite_graphics>(nullptr));
      ent->set(yacs::component_handle<components::states>(nullptr));
      ent->set(yacs::component_handle<components::point_light>(nullptr));
      ent->set(yacs::component_handle<components::attributes>(nullptr));
      ent->set(yacs::component_handle<components::effects>(nullptr));
      ent->set(yacs::component_handle<components::inventory>(nullptr));
      ent->set(yacs::component_handle<components::weapons>(nullptr));
      ent->add<components::player_interface>(ent);
      
      Global g;
      g.set_player(ent);
      
      return ent;
    }
    
    components::type_info* map_loader::create_type_info(yacs::entity* ent, const utils::id &id, yacs::entity* parent) {
      auto inf = ent->add<components::type_info>();
      inf->ent = ent;
      inf->parent = parent;
      inf->id = id;
      inf->states_count = 0;
      inf->states = nullptr;
      inf->created_ability = nullptr;
      return inf.get();
    }
    
//     UserDataComponent* map_loader::create_usr_data(yacs::entity* ent) {
//       auto user_data = ent->add<UserDataComponent>();
//       user_data->entity = ent;
//       return user_data.get();
//     }
    
    PhysicsComponent* map_loader::create_physics(yacs::entity* ent, const void* info) {
      auto info_phys = reinterpret_cast<const PhysicsComponent::CreateInfo*>(info);
      auto phys = ent->add<PhysicsComponent>(*info_phys);
      return phys.get();
    }
    
    components::indexed_graphics* map_loader::create_graphics(yacs::entity* ent, const uint32_t &offset, const uint32_t &count, const uint32_t &index) {
      auto graphics = ent->add<components::indexed_graphics>();
      graphics->ent = ent;
      graphics->offset = offset;
      graphics->count = count;
      graphics->index = index;
      return graphics.get();
    }
    
    components::states* map_loader::create_states(yacs::entity* ent, const core::state_t* state) {
      auto states = ent->add<components::states>();
      states->current = state;
      states->current_time = SIZE_MAX;
      states->ent = ent;
      return states.get();
    }
    
    components::vertex* map_loader::create_vertex(yacs::entity* ent, const simd::vec4 &center, const simd::vec4 &normal) {
      auto vertex = ent->add<components::vertex>(components::vertex::create_info{ent, center, normal});
      return vertex.get();
    }
    
    yacs::entity* map_loader::create_test_complex_obj(const uint32_t &index, std::vector<Vertex> &vertices) {
      const utils::id id = utils::id::get("complex_box");
      auto ent = world->create_entity();
      
      auto inf = ent->add<components::type_info>();
      inf->ent = ent;
      inf->parent = nullptr;
      inf->id = id;
      inf->states_count = 0;
      inf->states = nullptr;
      inf->created_ability = nullptr;
      
      auto trans = ent->add<TransformComponent>(simd::vec4(4.0f, 20.0f, 2.0f, 1.0f),
                                                simd::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                                                simd::vec4(1.0f, 1.0f, 1.0f, 0.0f)).get();
                                                
      //ent->set(yacs::component_handle<InputComponent>(nullptr));
      auto input = ent->add<InputComponent>();
      
      RegisterNewShapeInfo shapeInfo{};
      
      shapeInfo.points.push_back(simd::vec4(-0.5f,  0.5f,  0.5f, 1.0f)); // front | 0
      shapeInfo.points.push_back(simd::vec4( 0.5f,  0.5f,  0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4( 0.5f, -0.5f,  0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4(-0.5f, -0.5f,  0.5f, 1.0f));
      
      shapeInfo.faces.push_back (simd::vec4( 0.0f, 0.0f, 1.0f, glm::uintBitsToFloat(0)));
      
      shapeInfo.points.push_back(simd::vec4(-0.5f,  0.5f, -0.5f, 1.0f)); // back | 4
      shapeInfo.points.push_back(simd::vec4( 0.5f,  0.5f, -0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4( 0.5f, -0.5f, -0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4(-0.5f, -0.5f, -0.5f, 1.0f));
      
      shapeInfo.faces.push_back (simd::vec4( 0.0f, 0.0f,-1.0f, glm::uintBitsToFloat(4)));
      
      shapeInfo.points.push_back(simd::vec4(-0.5f,  0.5f, -0.5f, 1.0f)); // left | 8
      shapeInfo.points.push_back(simd::vec4(-0.5f,  0.5f,  0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4(-0.5f, -0.5f,  0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4(-0.5f, -0.5f, -0.5f, 1.0f));
      
      shapeInfo.faces.push_back (simd::vec4(-1.0f, 0.0f, 0.0f, glm::uintBitsToFloat(8)));
      
      shapeInfo.points.push_back(simd::vec4( 0.5f,  0.5f, -0.5f, 1.0f)); // right | 12
      shapeInfo.points.push_back(simd::vec4( 0.5f,  0.5f,  0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4( 0.5f, -0.5f,  0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4( 0.5f, -0.5f, -0.5f, 1.0f));
      
      shapeInfo.faces.push_back (simd::vec4( 1.0f, 0.0f, 0.0f, glm::uintBitsToFloat(12)));
      
      shapeInfo.points.push_back(simd::vec4(-0.5f,  0.5f,  0.5f, 1.0f)); // top | 16
      shapeInfo.points.push_back(simd::vec4( 0.5f,  0.5f,  0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4( 0.5f,  0.5f, -0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4(-0.5f,  0.5f, -0.5f, 1.0f));
      
      shapeInfo.faces.push_back (simd::vec4( 0.0f, 1.0f, 0.0f, glm::uintBitsToFloat(16)));
      
      shapeInfo.points.push_back(simd::vec4(-0.5f, -0.5f,  0.5f, 1.0f)); // bottom | 20
      shapeInfo.points.push_back(simd::vec4( 0.5f, -0.5f,  0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4( 0.5f, -0.5f, -0.5f, 1.0f));
      shapeInfo.points.push_back(simd::vec4(-0.5f, -0.5f, -0.5f, 1.0f));
      
      shapeInfo.faces.push_back (simd::vec4( 0.0f,-1.0f, 0.0f, glm::uintBitsToFloat(20)));
      
      const Type type = Type::get("complex_box");
      Global::get<PhysicsEngine>()->registerShape(type, POLYGON_TYPE, shapeInfo);
      
      PhysicsComponent::CreateInfo physInfo{
        {
          {0.0f, 0.0f, 0.0f, 0.0f},
          7.0f, 80.0f, 0.0f, 0.0f
        },
        {
          PhysicsType(true, POLYGON_TYPE, true, false, true, true),
          MONSTER_COLLISION_TYPE,     // collisionGroup
          monster_collision_filter,     // collisionFilter
          monster_trigger_filter,

          0.5f,  // stairHeight
          //40.0f, // acceleration
          1.0f,  // overbounce
          4.0f,  // groundFriction

          0.0f,  // radius
          1.0f,

          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,

          type
        },
        nullptr
      };
      
      physInfo.physInfo.transformIndex = trans->index();
      physInfo.physInfo.inputIndex = input->inputIndex;
      auto phys = ent->add<PhysicsComponent>(physInfo);
      phys->setUserData(ent);
      
      PRINT_VAR("box transform index", trans->index())
      PRINT_VAR("complex box   index", phys->getIndexContainer().objectDataIndex)
      
      // нужно сюда передать массив текстур, нет не нужно, лучше стейт
      static const utils::id state_id = utils::id::get("tmr_decor1_state");
      auto state = Global::get<game::states_container>()->get(state_id);
      ASSERT(state != nullptr);
      
      const glm::vec2 tex_coords[] = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
      };
      
      auto graphics = ent->add<components::complex_indices_graphics>();
      graphics->ent = ent;
      const uint32_t count = 4;
      for (size_t i = 0; i < 6; ++i) {
        const uint32_t offset = vertices.size();
        const uint32_t face_index = index + i;
        
        const components::complex_indices_graphics::indices idx{
          offset,
          count,
          face_index,
          state
        };
        graphics->model_faces.push_back(idx);
        
        const simd::vec4 normal = shapeInfo.faces[i] * simd::vec4(1.0f, 1.0f, 1.0f, 0.0);
        float arr[4];
        normal.storeu(arr);
        
        for (uint32_t j = 0; j < count; ++j) {
          vertices.push_back({
            shapeInfo.points[i*count+j],
            basic_vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(face_index)),
            tex_coords[j]
          });
        }
      }
      
      auto states = ent->add<components::states>(ent, state);
      
      // с вершинами пока не понятно, их должно быть по количеству фейсов
      //auto vertex = ent->add<components::vertex>(components::vertex::create_info{ent, simd::vec4(create_info.center), simd::vec4(create_info.normal)});
      //ASSERT(ent->at<components::vertex>(game::wall::vertex).valid());
      
      (void)states;
      // по идее это все что нужно для тестового объекта
      return ent;
    }
  }
}
