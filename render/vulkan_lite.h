#ifndef VULKAN_LITE_H
#define VULKAN_LITE_H

#include <cstdint>

#ifndef VK_DEFINE_HANDLE
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#endif

#if !defined(VK_DEFINE_NON_DISPATCHABLE_HANDLE)
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__) ) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif
#endif

#define VK_NULL_HANDLE 0

typedef uint64_t VkDeviceSize;

typedef struct VkOffset2D {
  int32_t    x;
  int32_t    y;
} VkOffset2D;

typedef struct VkExtent2D {
  uint32_t    width;
  uint32_t    height;
} VkExtent2D;

typedef struct VkRect2D {
  VkOffset2D    offset;
  VkExtent2D    extent;
} VkRect2D;

typedef struct VkExtent3D {
  uint32_t    width;
  uint32_t    height;
  uint32_t    depth;
} VkExtent3D;

#endif //VULKAN_LITE_H
