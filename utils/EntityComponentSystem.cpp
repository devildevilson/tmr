#define YACS_DEFINE_EVENT_TYPE
#include "EntityComponentSystem.h"

// std::ostream& operator<<(std::ostream& stream, const glm::vec3 &vec) {
//   stream << " " << vec.x << " " <<  vec.y << " " << vec.z << "\n";
//   return stream;
// }

// void YACS::print(glm::vec3 arg) {
//   std::cout << " " << arg.x << " " <<  arg.y << " " << arg.z << "\n";
// }

std::unordered_map<std::string, uint64_t> ComponentType::stringToType;
