#ifndef SHARED_COLLISION_CONSTANTS_H
#define SHARED_COLLISION_CONSTANTS_H

#include <cstdint>

// я тут подумал, а можно ли сделать с помощью этого уникальные пары?
// все статики с нулевым фильтром, у всех динамиков фильтр определяем
// от динамика к динамику количество фильтров снижается потому что мы не указываем повторы
// и по идее это должно привести к тому что у нас пары будут повторяться только между сходными типами коллизии
// но тем не менее все равно пары не уникальны =( 

enum CollisionType : uint32_t {
  PLAYER_COLLISION_TYPE           = (1<<0),
  WALL_COLLISION_TYPE             = (1<<1),
  MONSTER_COLLISION_TYPE          = (1<<2),
  ITEM_COLLISION_TYPE             = (1<<3),
  SMALL_DECORATION_COLLISION_TYPE = (1<<4),
  BIG_DECORATION_COLLISION_TYPE   = (1<<5),
  DOOR_COLLISION_TYPE             = (1<<6),
  INTERACTION_COLLISION_TYPE      = (1<<7),
  GHOST_COLLISION_TYPE            = (1<<8),
  NOCLIP_COLLISION_TYPE           = (1<<9),
  EVERYTHING                      = UINT32_MAX
};

const uint32_t player_collision_filter           = PLAYER_COLLISION_TYPE | WALL_COLLISION_TYPE | MONSTER_COLLISION_TYPE | BIG_DECORATION_COLLISION_TYPE | DOOR_COLLISION_TYPE;
const uint32_t wall_collision_filter             = PLAYER_COLLISION_TYPE |                       MONSTER_COLLISION_TYPE |                                 DOOR_COLLISION_TYPE | ITEM_COLLISION_TYPE | GHOST_COLLISION_TYPE;
const uint32_t monster_collision_filter          = PLAYER_COLLISION_TYPE | WALL_COLLISION_TYPE | MONSTER_COLLISION_TYPE | BIG_DECORATION_COLLISION_TYPE | DOOR_COLLISION_TYPE;
const uint32_t item_collision_filter             =                         WALL_COLLISION_TYPE |                          BIG_DECORATION_COLLISION_TYPE; // дверь?
const uint32_t small_decoration_collision_filter =                         WALL_COLLISION_TYPE |                          BIG_DECORATION_COLLISION_TYPE | DOOR_COLLISION_TYPE;
const uint32_t big_decoration_collision_filter   = PLAYER_COLLISION_TYPE | WALL_COLLISION_TYPE | MONSTER_COLLISION_TYPE |                                 DOOR_COLLISION_TYPE;
const uint32_t door_collision_filter             = PLAYER_COLLISION_TYPE | WALL_COLLISION_TYPE | MONSTER_COLLISION_TYPE | BIG_DECORATION_COLLISION_TYPE | DOOR_COLLISION_TYPE;
const uint32_t interaction_collision_filter      = 0; //PLAYER_COLLISION_TYPE | WALL_COLLISION_TYPE | MONSTER_COLLISION_TYPE | SMALL_DECORATION_COLLISION_TYPE | BIG_DECORATION_COLLISION_TYPE | DOOR_COLLISION_TYPE
const uint32_t ghost_collision_filter            = WALL_COLLISION_TYPE;
const uint32_t noclip_collision_filter           = 0;

const uint32_t player_trigger_filter           = SMALL_DECORATION_COLLISION_TYPE | INTERACTION_COLLISION_TYPE | ITEM_COLLISION_TYPE;
const uint32_t wall_trigger_filter             = INTERACTION_COLLISION_TYPE; // PLAYER_COLLISION_TYPE | MONSTER_COLLISION_TYPE | INTERACTION_COLLISION_TYPE
const uint32_t monster_trigger_filter          = SMALL_DECORATION_COLLISION_TYPE | INTERACTION_COLLISION_TYPE;
const uint32_t item_trigger_filter             = PLAYER_COLLISION_TYPE;
const uint32_t small_decoration_trigger_filter = PLAYER_COLLISION_TYPE | MONSTER_COLLISION_TYPE | INTERACTION_COLLISION_TYPE;
const uint32_t big_decoration_trigger_filter   = INTERACTION_COLLISION_TYPE;
const uint32_t door_trigger_filter             = INTERACTION_COLLISION_TYPE;
const uint32_t interaction_trigger_filter      = PLAYER_COLLISION_TYPE | MONSTER_COLLISION_TYPE | WALL_COLLISION_TYPE | SMALL_DECORATION_COLLISION_TYPE | BIG_DECORATION_COLLISION_TYPE | DOOR_COLLISION_TYPE;
const uint32_t ghost_trigger_filter            = 0;
const uint32_t noclip_trigger_filter           = 0;

#endif
