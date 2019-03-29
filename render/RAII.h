#ifndef RAII_H
#define RAII_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Internal.h"

namespace yavf {
  class Device;
  
  namespace raii {
  #define FUNC_NAME_ENUM(func) e##func
  #define FUNC_POINTER(func) reinterpret_cast<void*>(func)
  #define GET_FUNC_POINTER(type, func) (reinterpret_cast<type>(func))
    
    enum VulkanFuncEnum {
      FUNC_NAME_ENUM(vkCreateShaderModule),
      FUNC_NAME_ENUM(vkCreateBufferView),
      FUNC_NAME_ENUM(vkCreateCommandPool),
      FUNC_NAME_ENUM(vkCreateComputePipelines),
      FUNC_NAME_ENUM(vkCreateDescriptorPool),
      FUNC_NAME_ENUM(vkCreateDescriptorSetLayout),
      FUNC_NAME_ENUM(vkCreateDescriptorUpdateTemplate),
      FUNC_NAME_ENUM(vkCreateDevice),
  //     FUNC_NAME_ENUM(vkCreateDisplayModeKHR),
      FUNC_NAME_ENUM(vkCreateDisplayPlaneSurfaceKHR),
      FUNC_NAME_ENUM(vkCreateEvent),
      FUNC_NAME_ENUM(vkCreateFence),
      FUNC_NAME_ENUM(vkCreateFramebuffer),
      FUNC_NAME_ENUM(vkCreateGraphicsPipelines),
      FUNC_NAME_ENUM(vkCreateImageView),
  //     FUNC_NAME_ENUM(vkCreateIndirectCommandsLayoutNVX),
      FUNC_NAME_ENUM(vkCreateInstance),
  //     FUNC_NAME_ENUM(vkCreateObjectTableNVX),
      FUNC_NAME_ENUM(vkCreatePipelineCache),
      FUNC_NAME_ENUM(vkCreatePipelineLayout),
      FUNC_NAME_ENUM(vkCreateQueryPool),
  //     FUNC_NAME_ENUM(vkCreateRayTracingPipelinesNV),
      FUNC_NAME_ENUM(vkCreateRenderPass),
//       FUNC_NAME_ENUM(vkCreateRenderPass2KHR),
      FUNC_NAME_ENUM(vkCreateSampler),
      FUNC_NAME_ENUM(vkCreateSamplerYcbcrConversion),
      FUNC_NAME_ENUM(vkCreateSemaphore),
  //     FUNC_NAME_ENUM(vkCreateSharedSwapchainsKHR),
      FUNC_NAME_ENUM(vkCreateSwapchainKHR),
  //     FUNC_NAME_ENUM(vkCreateValidationCacheEXT),     // вроде как для EXT надо загрузить указатели на функцию
  //     FUNC_NAME_ENUM(vkCreateDebugReportCallbackEXT), // и поэтому здесь это не подходит
  //     FUNC_NAME_ENUM(vkCreateDebugUtilsMessengerEXT),
      FUNC_NAME_ENUM(vmaCreateBuffer),
      FUNC_NAME_ENUM(vmaCreateImage),
      VULKAN_FUNC_ENUM_COUNT
    };
    
    struct FuncPointers {void* create; void* destroy;};
    static const FuncPointers vulkanFuncPtr[] = {
      {FUNC_POINTER(vkCreateShaderModule), FUNC_POINTER(vkDestroyShaderModule)},
      {FUNC_POINTER(vkCreateBufferView), FUNC_POINTER(vkDestroyBufferView)},
      {FUNC_POINTER(vkCreateCommandPool), FUNC_POINTER(vkDestroyCommandPool)},
      {FUNC_POINTER(vkCreateComputePipelines), FUNC_POINTER(vkDestroyPipeline)},
      {FUNC_POINTER(vkCreateDescriptorPool), FUNC_POINTER(vkDestroyDescriptorPool)},
      {FUNC_POINTER(vkCreateDescriptorSetLayout), FUNC_POINTER(vkDestroyDescriptorSetLayout)},
      {FUNC_POINTER(vkCreateDescriptorUpdateTemplate), FUNC_POINTER(vkDestroyDescriptorUpdateTemplate)},
      {FUNC_POINTER(vkCreateDevice), FUNC_POINTER(vkDestroyDevice)},
  //     {FUNC_POINTER(vkCreateDisplayModeKHR), FUNC_POINTER(vkDestroyDisplayModeKHR)},
      {FUNC_POINTER(vkCreateDisplayPlaneSurfaceKHR), FUNC_POINTER(vkDestroySurfaceKHR)},
      {FUNC_POINTER(vkCreateEvent), FUNC_POINTER(vkDestroyEvent)},
      {FUNC_POINTER(vkCreateFence), FUNC_POINTER(vkDestroyFence)},
      {FUNC_POINTER(vkCreateFramebuffer), FUNC_POINTER(vkDestroyFramebuffer)},
      {FUNC_POINTER(vkCreateGraphicsPipelines), FUNC_POINTER(vkDestroyPipeline)},
      {FUNC_POINTER(vkCreateImageView), FUNC_POINTER(vkDestroyImageView)},
      {FUNC_POINTER(vkCreateInstance), FUNC_POINTER(vkDestroyInstance)},
      {FUNC_POINTER(vkCreatePipelineCache), FUNC_POINTER(vkDestroyPipelineCache)},
      {FUNC_POINTER(vkCreatePipelineLayout), FUNC_POINTER(vkDestroyPipelineLayout)},
      {FUNC_POINTER(vkCreateQueryPool), FUNC_POINTER(vkDestroyQueryPool)},
      {FUNC_POINTER(vkCreateRenderPass), FUNC_POINTER(vkDestroyRenderPass)},
//       {FUNC_POINTER(vkCreateRenderPass2KHR), FUNC_POINTER(vkDestroyRenderPass)},
      {FUNC_POINTER(vkCreateSampler), FUNC_POINTER(vkDestroySampler)},
      {FUNC_POINTER(vkCreateSamplerYcbcrConversion), FUNC_POINTER(vkDestroySamplerYcbcrConversion)},
      {FUNC_POINTER(vkCreateSemaphore), FUNC_POINTER(vkDestroySemaphore)},
  //     {FUNC_POINTER(vkCreateSharedSwapchainsKHR), FUNC_POINTER(vkDestroySwapchainKHR)},
      {FUNC_POINTER(vkCreateSwapchainKHR), FUNC_POINTER(vkDestroySwapchainKHR)},
      {FUNC_POINTER(vmaCreateBuffer), FUNC_POINTER(vmaDestroyBuffer)},
      {FUNC_POINTER(vmaCreateImage), FUNC_POINTER(vmaDestroyImage)}
    };
    
    typedef VkResult (*vmaCreateImageType)(VmaAllocator, const VkImageCreateInfo*, const VmaAllocationCreateInfo*, VkImage*, VmaAllocation*, VmaAllocationInfo*);
    typedef void (*vmaDestroyImageType)(VmaAllocator, VkImage, VmaAllocation);
    typedef VkResult (*vmaCreateBufferType)(VmaAllocator, const VkBufferCreateInfo*, const VmaAllocationCreateInfo*, VkBuffer*, VmaAllocation*, VmaAllocationInfo*);
    typedef void (*vmaDestroyBufferType)(VmaAllocator, VkBuffer, VmaAllocation);
    
    template <typename ObjType>
    class Copyable {
    public:
      Copyable(ObjType obj) : obj(obj) {}
      Copyable(const Copyable<ObjType> &copy) : obj(copy.obj) {}
      ~Copyable() {}
      
      Copyable & operator=(const Copyable<ObjType> &copy) {
        obj = copy.obj;
        return *this;
      }
      
      operator ObjType() const {
        return obj;
      }
      
      bool operator==(const Copyable<ObjType> &copy) const {
        return obj == copy.obj;
      }
      
      bool operator!=(const Copyable<ObjType> &copy) const {
        return obj != copy.obj;
      }
    protected:
      ObjType obj;
    };
    
    template <typename ObjType, typename ObjCreateInfo, typename CreateFuncType, typename DestroyFuncType>
    class RAIIType1 {
    public:
      RAIIType1(const VulkanFuncEnum &e) : e(e), d(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) {}
      RAIIType1(VkDevice d, const VulkanFuncEnum &e) : e(e), d(d), obj(VK_NULL_HANDLE) {}
      
      RAIIType1(VkDevice d, const ObjCreateInfo &info, const char* funcName, VulkanFuncEnum e) 
      : e(e), d(d), obj(VK_NULL_HANDLE) { 
        vkCheckError(funcName, nullptr, 
        GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(d, &info, nullptr, &obj));
      }
      
      RAIIType1(const RAIIType1 &another) = delete;
      RAIIType1(RAIIType1 &&another) {
        e = another.e;
        obj = another.obj;
        d = another.d;
        another.obj = VK_NULL_HANDLE;
      }
      
      ~RAIIType1() {
        if (obj != VK_NULL_HANDLE) GET_FUNC_POINTER(DestroyFuncType, vulkanFuncPtr[e].destroy)(d, obj, nullptr);
      }
      
      RAIIType1 & operator=(const RAIIType1 &another) = delete;
      RAIIType1 & operator=(RAIIType1 &&another) {
        if (this == &another) return *this;
        
        e = another.e;
        obj = another.obj;
        d = another.d;
        another.obj = VK_NULL_HANDLE;
        return *this;
      }
      
      VkResult construct(VkDevice d, const ObjCreateInfo &info) {
        this->d = d;
        return GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(d, &info, nullptr, &obj);
      }
      
      VkResult construct(const ObjCreateInfo &info) {
        return GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(d, &info, nullptr, &obj);
      }
      
      operator ObjType() const {
        return obj;
      }
    private:
      VulkanFuncEnum e;
    protected:
      VkDevice d;
      ObjType obj;
    };
    
    template <typename ObjType, typename ObjCreateInfo, typename CreateFuncType, typename DestroyFuncType>
    class RAIIType2 {
    public:
      RAIIType2(const VulkanFuncEnum &e) : e(e), d(VK_NULL_HANDLE), cache(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) {}
      RAIIType2(VkDevice d, const VulkanFuncEnum &e) : e(e), d(d), cache(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) {}
      RAIIType2(VkDevice d, VkPipelineCache cache, const VulkanFuncEnum &e) : e(e), d(d), cache(cache), obj(VK_NULL_HANDLE) {}
      
      RAIIType2(VkDevice d, VkPipelineCache cache, const ObjCreateInfo &info, const char* funcName, VulkanFuncEnum e) 
      : e(e), d(d), cache(cache), obj(VK_NULL_HANDLE) { //, createFuncPtr(createFuncPtr), destroyFuncPtr(destroyFuncPtr) {
        vkCheckError(funcName, nullptr, 
        GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(d, cache, 1, &info, nullptr, &obj));
      }
      
      RAIIType2(const RAIIType2 &another) = delete;
      RAIIType2(RAIIType2 &&another) {
        e = another.e;
        obj = another.obj;
        cache = another.cache;
        d = another.d;
        another.obj = VK_NULL_HANDLE;
      }
      
      ~RAIIType2() {
        if (obj != VK_NULL_HANDLE) GET_FUNC_POINTER(DestroyFuncType, vulkanFuncPtr[e].destroy)(d, obj, nullptr);
      }
      
      RAIIType2 & operator=(const RAIIType2 &another) = delete;
      RAIIType2 & operator=(RAIIType2 &&another) {
        if (this == &another) return *this;
        
        e = another.e;
        obj = another.obj;
        cache = another.cache;
        d = another.d;
        another.obj = VK_NULL_HANDLE;
        return *this;
      }
      
      VkResult construct(VkDevice d, const ObjCreateInfo &info) {
        this->d = d;
        return GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(d, cache, 1, &info, nullptr, &obj);
      }
      
      VkResult construct(VkDevice d, VkPipelineCache cache, const ObjCreateInfo &info) {
        this->d = d;
        this->cache = cache;
        return GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(d, cache, 1, &info, nullptr, &obj);
      }
      
      VkResult construct(const ObjCreateInfo &info) {
        return GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(d, cache, 1, &info, nullptr, &obj);
      }
      
      operator ObjType() const {
        return obj;
      }
    private:
      VulkanFuncEnum e;
    protected:
      VkDevice d;
      VkPipelineCache cache;
      ObjType obj;
    };
    
    template <typename ObjType, typename ObjCreateInfo, typename CreateFuncType, typename DestroyFuncType>
    class RAIIType3 {
    public:
      RAIIType3(const VulkanFuncEnum &e) : e(e), obj(VK_NULL_HANDLE) {}
      
      RAIIType3(const ObjCreateInfo &info, const char* funcName, VulkanFuncEnum e) 
      : e(e), obj(VK_NULL_HANDLE) { //, createFuncPtr(createFuncPtr), destroyFuncPtr(destroyFuncPtr) {
        vkCheckError(funcName, nullptr, 
        GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(&info, nullptr, &obj));
      }
      
      RAIIType3(const RAIIType3 &another) = delete;
      RAIIType3(RAIIType3 &&another) {
        e = another.e;
        obj = another.obj;
        another.obj = VK_NULL_HANDLE;
      }
      
      ~RAIIType3() {
        if (obj != VK_NULL_HANDLE) GET_FUNC_POINTER(DestroyFuncType, vulkanFuncPtr[e].destroy)(obj, nullptr);
      }
      
      RAIIType3 & operator=(const RAIIType3 &another) = delete;
      RAIIType3 & operator=(RAIIType3 &&another) {
        if (this == &another) return *this;
        
        e = another.e;
        obj = another.obj;
        another.obj = VK_NULL_HANDLE;
        return *this;
      }
      
      VkResult construct(const ObjCreateInfo &info) {
        return GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(&info, nullptr, &obj);
      }
      
      operator ObjType() const {
        return obj;
      }
    private:
      VulkanFuncEnum e;
    protected:
      ObjType obj;
    };
    
    template <typename ObjType, typename ObjCreateInfo, typename CreateFuncType, typename DestroyFuncType>
    class RAIIType4 {
    public:
      RAIIType4(const VulkanFuncEnum &e) : e(e), allocator(VK_NULL_HANDLE), allocation(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) {}
      RAIIType4(VmaAllocator allocator, const VulkanFuncEnum &e) : e(e), allocator(allocator), allocation(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) {}
      
      RAIIType4(VmaAllocator allocator, const ObjCreateInfo &info, const VmaAllocationCreateInfo &pAllocationCreateInfo, const char* funcName, VulkanFuncEnum e) 
      : e(e), allocator(allocator), allocation(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) { //, createFuncPtr(createFuncPtr), destroyFuncPtr(destroyFuncPtr) {
        vkCheckError(funcName, nullptr, 
        GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(allocator,  &info, &pAllocationCreateInfo, &obj, &allocation, nullptr));
      }
      
      RAIIType4(const RAIIType4 &another) = delete;
      RAIIType4(RAIIType4 &&another) {
        e = another.e;
        obj = another.obj;
        allocation = another.allocation;
        allocator = another.allocator;
        another.obj = VK_NULL_HANDLE;
      }
      
      ~RAIIType4() {
        if (obj != VK_NULL_HANDLE) GET_FUNC_POINTER(DestroyFuncType, vulkanFuncPtr[e].destroy)(allocator, obj, allocation);
      }
      
      RAIIType4 & operator=(const RAIIType4 &another) = delete;
      RAIIType4 & operator=(RAIIType4 &&another) {
        if (this == &another) return *this;
        
        e = another.e;
        obj = another.obj;
        allocation = another.allocation;
        allocator = another.allocator;
        another.obj = VK_NULL_HANDLE;
        return *this;
      }
      
      VkResult construct(VmaAllocator allocator, const ObjCreateInfo &info, const VmaAllocationCreateInfo &pAllocationCreateInfo, VmaAllocationInfo *pAllocationInfo) {
        this->allocator = allocator;
        return GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(allocator,  &info, &pAllocationCreateInfo, &obj, &allocation, pAllocationInfo);
      }
      
      VkResult construct(const ObjCreateInfo &info, const VmaAllocationCreateInfo &pAllocationCreateInfo, VmaAllocationInfo *pAllocationInfo) {
        return GET_FUNC_POINTER(CreateFuncType, vulkanFuncPtr[e].create)(allocator,  &info, &pAllocationCreateInfo, &obj, &allocation, pAllocationInfo);
      }
      
      operator ObjType() const {
        return obj;
      }
    private:
      VulkanFuncEnum e;
    protected:
      VmaAllocator allocator;
      VmaAllocation allocation;
      ObjType obj;
    };
    
    template <typename ObjType, typename DestroyFuncType>
    class RAIIType5 {
    public:
      RAIIType5(VkDevice d, ObjType obj, VulkanFuncEnum e) : e(e), d(d), obj(obj) {}
      RAIIType5(const RAIIType5 &another) = delete;
      RAIIType5(RAIIType5 &&another) {
        e = another.e;
        obj = another.obj;
        another.obj = VK_NULL_HANDLE;
      }
      
      ~RAIIType5() {
        if (obj != VK_NULL_HANDLE) GET_FUNC_POINTER(DestroyFuncType, vulkanFuncPtr[e].destroy)(d, obj, nullptr);
      }
      
      RAIIType5 & operator=(const RAIIType5 &another) = delete;
      RAIIType5 & operator=(RAIIType5 &&another) {
        if (this == &another) return *this;
        
        e = another.e;
        obj = another.obj;
        d = another.d;
        another.obj = VK_NULL_HANDLE;
        return *this;
      }
      
      operator ObjType() const {
        return obj;
      }
    private:
      VulkanFuncEnum e;
    protected:
      VkDevice d;
      ObjType obj;
    };
    
    class ShaderModule : public RAIIType1<VkShaderModule, VkShaderModuleCreateInfo, PFN_vkCreateShaderModule, PFN_vkDestroyShaderModule> {
    public:
      ShaderModule();
      ShaderModule(Device* d, const VkShaderModuleCreateInfo &info);
      ShaderModule(Device* d, const char* path);
    };
    
//     class Pipeline2 : public RAIIType5<VkPipeline, PFN_vkDestroyPipeline> {
//     public:
//       Pipeline2(VkDevice d, VkPipeline obj) : RAIIType5(d, obj, FUNC_NAME_ENUM(vkCreateComputePipelines)) {}
//     };
//     
//     class Image2 : public RAIIType4<VkImage, VkImageCreateInfo, vmaCreateImageType, vmaDestroyImageType> {
//     public:
//       Image2(VmaAllocator a) : RAIIType4(a, FUNC_NAME_ENUM(vmaCreateImage)) {}
//     };
  }
}
  
#endif
