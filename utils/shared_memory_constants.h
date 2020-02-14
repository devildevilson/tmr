#ifndef SHARED_MEMORY_CONSTANTS_H
#define SHARED_MEMORY_CONSTANTS_H

// с помощью констант можно задать статические массивы практически во всех компонентах
// во всех, и не только в компонентах

// const size_t power_of_two[] = {
//   1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576
// };

// const size_t power_of_four[] = {
//   1 << 0, 1 << (1 << 1), 1 << (1 << 2), 1 << (1 << 3), 1 << (1 << 4), static_cast<size_t>(1 << (1 << 5)), 1 << (1 << 6), 1 << (1 << 7), 1 << (1 << 8),
//   1 << (1 << 9), 1 << (1 << 10), 1 << (1 << 11), 1 << (1 << 12), 1 << (1 << 13), 1 << (1 << 14), 1 << (1 << 15), 1 << (1 << 16),
// };

constexpr size_t power_of_two(const uint32_t &power) { return 1 << power; }
constexpr size_t power_of_four(const uint32_t &power) { return 1 << (power*2); }

#define BYTE_SIZE power_of_two(0)
#define KB_SIZE   power_of_two(10)
#define MB_SIZE   power_of_two(20)
#define GB_SIZE   power_of_two(30)

#define FLOAT_ATTRIBUTES_MAX_COUNT power_of_two(4)
#define INT_ATTRIBUTES_MAX_COUNT power_of_two(4)
#define EFFECT_BASE_VALUES_MAX_COUNT power_of_two(4)
#define IMPACT_EFFECT_MAX_COUNT power_of_two(4)
#define ABILITY_MEMORY_MAX_COUNT 3
#define ATTRIBUTE_CHANGE_MEMORY_SIZE 20
#define STIMULUS_BUFFER_SIZE 20

// ограничить контейнер? в принципе идея норм
//#define EFFECT_MAX_BONUSES 16

// #define FLOAT_ATTRIBUTES_MAX_COUNT 16
// #define INT_ATTRIBUTES_MAX_COUNT 16

#endif
