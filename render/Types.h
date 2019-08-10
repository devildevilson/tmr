#ifndef YAVF_TYPES_H
#define YAVF_TYPES_H

#include <vulkan/vulkan.h>
//#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
//#define VMA_DEBUG_MARGIN 16
//#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_RECORDING_ENABLED 0
#include <vk_mem_alloc.h>

#include "Internal.h"
#include "RAII.h"

#define YAVF_DEFAULT_SWIZZLE {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A}

namespace yavf {
  class Device;
  
  typedef VkRenderPass RenderPass;
  typedef VkPipelineLayout PipelineLayout;
  typedef VkDescriptorSetLayout DescriptorSetLayout;
  typedef VkDescriptorPool DescriptorPool;
  typedef VkCommandPool CommandPool;
  typedef VkSemaphore Semaphore;
//   typedef VkFramebuffer Framebuffer;
  typedef VkFence Fence;
  
//   struct ImageCreateInfo {  
//     VkImageCreateFlags flags;
//     VkImageType imageType;
//     VkFormat format;
//     VkExtent3D extent;
//     uint32_t mipLevels;
//     uint32_t arrayLayers;
//     VkSampleCountFlagBits samples;
//     VkImageTiling tiling;
//     VkImageUsageFlags usage;
//     
//     VkImageAspectFlags aspect;
//     
//     VmaMemoryUsage memoryUsage;
//   };
  
  class Swapchain;
  
//   struct SwapchainCreateInfo {
//     VkSwapchainCreateInfoKHR _info;
//     
//     static SwapchainCreateInfo swapchain(const VkSurfaceKHR &surface,
//                                          const uint32_t &minImageCount,
//                                          const VkExtent2D &imageExtent,
//                                          const VkImageUsageFlags &imageUsage,
//                                          const VkPresentModeKHR &presentMode,
//                                          const Swapchain &oldSwapchain,
//                                          
//     );
//   };
  
  struct FramebufferCreateInfo {
    VkFramebufferCreateInfo _info;
    
    static FramebufferCreateInfo framebuffer(const RenderPass &renderPass,
                                             const uint32_t &attachmentCount,
                                             const VkImageView* pAttachments,
                                             const uint32_t &width,
                                             const uint32_t &height,
                                             const uint32_t &layers = 1);
    
    FramebufferCreateInfo();
    FramebufferCreateInfo(const VkFramebufferCreateInfo &info);
    
    FramebufferCreateInfo & operator=(const VkFramebufferCreateInfo &info);
    FramebufferCreateInfo & operator=(const FramebufferCreateInfo &info);
    
    operator VkFramebufferCreateInfo() const;
  };
  
  struct ImageCreateInfo {
    VkImageCreateInfo _info;
    
    static ImageCreateInfo texture2D(const VkExtent2D &size,
                                     const VkImageUsageFlags &usage,
                                     const VkFormat &format = VK_FORMAT_R8G8B8A8_UNORM,
                                     const uint32_t &arrayLayers = 1,
                                     const uint32_t &mipLevels = 1,
                                     const VkSampleCountFlagBits &samples = VK_SAMPLE_COUNT_1_BIT,
                                     const VkImageCreateFlags &flags = 0);
    
    static ImageCreateInfo texture2DStaging(const VkExtent2D &size,
                                            const VkImageUsageFlags &usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                            const VkFormat &format = VK_FORMAT_R8G8B8A8_UNORM,
                                            const VkImageCreateFlags &flags = 0);
    
    ImageCreateInfo();
    ImageCreateInfo(const VkImageCreateInfo &info);
    
    ImageCreateInfo & operator=(const VkImageCreateInfo &info);
    ImageCreateInfo & operator=(const ImageCreateInfo &info);
    
    operator VkImageCreateInfo() const;
    // тут можно потом добавить всего прочего
  };

  struct BufferCreateInfo {
    VkBufferCreateInfo _info;
    
    static BufferCreateInfo buffer(const VkDeviceSize &size, const VkBufferUsageFlags &usage, const VkBufferCreateFlags &flags = 0);
    
    BufferCreateInfo();
    BufferCreateInfo(const VkBufferCreateInfo &info);
    
    BufferCreateInfo & operator=(const VkBufferCreateInfo &info);
    BufferCreateInfo & operator=(const BufferCreateInfo &info);
    
    operator VkBufferCreateInfo() const;
  };
  
//   typedef VkDescriptorSet Descriptor;
  
//   struct DescriptorUpdate {
//     VkDescriptorType type;
//     uint32_t arrayElement;
//     uint32_t bindingNum;
//     Descriptor handle;
//   };
  
//   struct BufferView {
//     VkFormat format = VK_FORMAT_UNDEFINED;
//     VkBufferView handle = VK_NULL_HANDLE;
//   };

//   struct ImageView {
//     VkImageViewType type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
//     VkFormat format = VK_FORMAT_UNDEFINED;
//     VkComponentMapping components = YAVF_DEFAULT_SWIZZLE;
//     VkImageAspectFlags aspect = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
//     VkImageView handle = VK_NULL_HANDLE;
//   };
  
  namespace Internal {
    class DescriptorSetBase;
    class FramebufferBase;
    class SamplerBase;
    class BufferViewBase;
    class BufferBase;
    class ImageViewBase;
    class ImageBase;
  }
  
  class DescriptorSet;
  class Image;
  
//   class DescriptorSetLayout {
//   public:
//     DescriptorSetLayout(const VkDescriptorSetLayout &layout, const VkDescriptorType &setType);
//     ~DescriptorSetLayout();
//     
//     VkDescriptorSetLayout handle() const;
//     VkDescriptorType type() const;
//     
//     operator VkDescriptorSetLayout() const;
//     bool operator==(const DescriptorSetLayout &layout) const;
//     bool operator!=(const DescriptorSetLayout &layout) const;
//   private:
//     VkDescriptorSetLayout layout;
//     VkDescriptorType setType;
//   };
  
  class Swapchain {
  public:
    Swapchain();
    Swapchain(const Swapchain &swapchain);
    Swapchain(const VkExtent2D &extent, Device* device, const VkSwapchainKHR &swap);
    ~Swapchain();
    
    VkResult getImages(uint32_t* imagesCount, Image** images);
    VkResult acquireNextImage(const uint64_t &timeout, const Semaphore &semaphore, const Fence &fence, uint32_t* pImageIndex);
    
    VkSwapchainKHR handle() const;
    
    Swapchain & operator=(const Swapchain &another);
    
    void setImageExtent(const VkExtent2D &imageExtent);
    VkExtent2D getImageExtent() const;
    
    operator VkSwapchainKHR() const;
    bool operator==(const Swapchain &another) const;
    bool operator!=(const Swapchain &another) const;
  private:
    VkExtent2D imageExtent;
    Device* device;
    VkSwapchainKHR swap;
  };
  
  class Framebuffer {
  public:
    Framebuffer();
    Framebuffer(const Framebuffer &framebuffer);
    Framebuffer(VkFramebuffer buffer);
    ~Framebuffer();
    
    VkFramebuffer handle() const;
    
    Framebuffer & operator=(const Framebuffer &another);
    
    operator VkFramebuffer() const;
    bool operator==(const Framebuffer &another) const;
    bool operator!=(const Framebuffer &another) const;
  private:
    VkFramebuffer buffer;
  };
  
  class Pipeline {
  public:
    Pipeline();
    Pipeline(VkPipeline h, VkPipelineLayout l);
    
    VkPipeline handle() const;
    VkPipelineLayout layout() const;

    bool operator==(const Pipeline &other) const;
    bool operator!=(const Pipeline &other) const;
    operator VkPipeline() const;
  private:
    VkPipeline h = VK_NULL_HANDLE;
    VkPipelineLayout layoutH = VK_NULL_HANDLE;
  };
  
  
//   class Sampler {
//   public:
//     Sampler();
//     Sampler(Internal::SamplerBase* ptr);
//     ~Sampler();
//     
//     bool valid() const;
//     
//     Internal::SamplerBase* operator->() const;
//     
//     Sampler & operator=(const Sampler &another);
//     
//     operator VkSampler() const;
//     bool operator==(const Sampler &another) const;
//     bool operator!=(const Sampler &another) const;
//   private:
//     Internal::SamplerBase* ptr;
//   };
  
  class Sampler {
  public:
    Sampler();
    Sampler(VkSampler sampler);
    ~Sampler();
    
    bool valid() const;
    
    VkSampler handle() const;

    DescriptorSet* descriptorSet() const;
    size_t descriptorSetIndex() const;
    
    //const uint32_t &binding, const uint32_t &arrayNum
    void setDescriptor(DescriptorSet* set, const size_t &index);
    
    operator VkSampler() const;
    bool operator==(const Sampler &another) const;
    bool operator!=(const Sampler &another) const;
  private:
    VkSampler sampler;
    DescriptorSet* setHandle;
    size_t resIndex;
  };
  
  class Buffer;
  
  class BufferView : public raii::RAIIType1<VkBufferView, VkBufferViewCreateInfo, raii::vkCreateBufferView, raii::vkDestroyBufferView> {
    friend Device; // без этого скорее всего не обойтись
  public:
    BufferView();
    BufferView(Device* device, const VkBufferViewCreateInfo &info, Buffer* buffer);
    ~BufferView();
    
    void construct(Device* device, const VkBufferViewCreateInfo &info, Buffer* buffer);
    
    VkFormat format() const; 
    size_t offset() const; 
    size_t size() const;
    
    DescriptorSet* descriptorSet() const;
    size_t descriptorSetIndex() const;
    void setDescriptor(DescriptorSet* set, const size_t &index);
    
    VkBufferView handle() const;
    
    Buffer* buffer() const;
    
    BufferView* next() const;
    void setNext(BufferView* ptr);
    
    void recreate();
  private:
    VkFormat formatVar;
    size_t internalIndex;
    size_t offsetVar;
    size_t sizeVar;
    
    Buffer* relatedBuffer;
    Device* device;
    
    DescriptorSet* set;
    size_t setIndex;
    
    BufferView* nextPtr;
  };
  
//   class BufferView {
//   public:
//     BufferView();
//     BufferView(Internal::BufferViewBase* ptr);
//     ~BufferView();
//     
//     bool valid() const;
//     
//     Internal::BufferViewBase* operator->() const;
//     
//     BufferView & operator=(const BufferView &another);
//     
//     operator VkBufferView() const;
//     bool operator==(const BufferView &another) const;
//     bool operator!=(const BufferView &another) const;
//   private:
//     Internal::BufferViewBase* ptr;
//   };
  
  class Buffer : public raii::RAIIType4<VkBuffer, VkBufferCreateInfo, raii::vmaCreateBuffer, raii::vmaDestroyBuffer> {
    friend Device;
  public:
    Buffer();
    Buffer(Device* device, const VkBufferCreateInfo &info, const VmaMemoryUsage &memUsage);
    ~Buffer();
    
    void construct(Device* device, const VkBufferCreateInfo &info, const VmaMemoryUsage &memUsage);
    
    void recreate(const size_t &newSize);
    void resize(const size_t &newSize);
    void flush() const;
    
    // что с вью? их может быть несколько, и их нужно обновлять каждый раз как я переделываю буфер
    // придется хранить их всех в буфере
    size_t viewCount() const;
    BufferView* view(const size_t &index = 0) const;
    BufferView* createView(const VkFormat &format, const size_t &offset = 0, const size_t &size = VK_WHOLE_SIZE);
    
    DescriptorSet* descriptorSet() const;
    size_t descriptorSetIndex() const;
    void setDescriptor(DescriptorSet* set, const size_t &index);
    
    Internal::BufferInfo & info();
    VkBuffer handle() const;
    VmaAllocation allocation() const;
    void* ptr() const;
  private:
    VmaMemoryUsage memUsage;
    size_t internalIndex;

    Device* device;
    void* pointer;
    
    size_t viewSize;
    BufferView* bufferView;
    
    DescriptorSet* set;
    size_t setIndex;

    Internal::BufferInfo parameters;
  };
  
//   class Buffer {
//   public:
//     Buffer();
//     Buffer(Internal::BufferBase* ptr);
//     ~Buffer();
//     
//     bool valid() const;
//     
//     Internal::BufferBase* operator->() const;
//     
//     Buffer & operator=(const Buffer &another);
//     
//     operator VkBuffer() const;
//     bool operator==(const Buffer &another) const;
//     bool operator!=(const Buffer &another) const;
//   private:
//     Internal::BufferBase* bufferPtr;
//   };
  
  class ImageView : public raii::RAIIType1<VkImageView, VkImageViewCreateInfo, raii::vkCreateImageView, raii::vkDestroyImageView> {
    friend Device;
  public:
    ImageView();
    ImageView(Device* device, const VkImageViewCreateInfo &info, Image* relatedImage);
    ~ImageView();
    
    void construct(Device* device, const VkImageViewCreateInfo &info, Image* relatedImage);
    
    VkImageViewType type() const;
    VkFormat format() const;
    VkComponentMapping components() const;
    VkImageSubresourceRange subresourceRange() const;
    VkImageView handle() const;
    
    Image* image() const;
    
    DescriptorSet* descriptorSet() const;
    size_t descriptorSetIndex() const;
    void setDescriptor(DescriptorSet* res, const size_t &resIndex);
    
    ImageView* next() const;
    void setNext(ImageView* ptr);
    
    void recreate();
  private:
    VkImageViewType typeVar;
    VkFormat formatVar;
    VkComponentMapping componentsVar;
    VkImageSubresourceRange subresourceRangeVar;
    
    size_t internalIndex;
    Image* imageVar;
    Device* device;
    
    DescriptorSet* set;
    size_t setIndex;
    
    ImageView* nextPtr;
  };

//   class ImageView {
//   public:
//     ImageView();
//     ImageView(Internal::ImageViewBase* ptr);
//     ~ImageView();
//     
//     bool valid() const;
//     
//     Internal::ImageViewBase* operator->() const;
//     
//     ImageView & operator=(const ImageView &another);
//     
//     operator VkImageView() const;
//     bool operator==(const ImageView &another) const;
//     bool operator!=(const ImageView &another) const;
//   private:
//     Internal::ImageViewBase* ptr;
//   };
  
//   class Image {
// //     friend class TaskInterface;
//   public:
//     Image();
//     Image(Internal::ImageBase* imagePtr);
//     ~Image();
//     
//     bool valid() const;
//     
//     Internal::ImageBase* operator->() const;
//     
//     Image & operator=(const Image &another);
//     
//     operator VkImage() const;
//     bool operator==(const Image &another) const;
//     bool operator!=(const Image &another) const;
//   private:
//     Internal::ImageBase* imagePtr;
//   };
  
  class Image : public raii::RAIIType4<VkImage, VkImageCreateInfo, raii::vmaCreateImage, raii::vmaDestroyImage> {
    friend Device;
  public:
    Image();
    Image(Device* device, const VkImageCreateInfo &info, const VmaMemoryUsage &memUsage);
    Image(Device* device, VkImage image, const VkExtent2D &size);
    ~Image();
    
    void construct(Device* device, const VkImageCreateInfo &info, const VmaMemoryUsage &memUsage);
    
    void recreate(const VkExtent3D &newSize);
    void flush() const;
    
    size_t viewCount() const;
    ImageView* view(const size_t &index = 0);
    ImageView* createView(const VkImageViewType &type, const VkImageSubresourceRange &range,
                          const VkFormat &format = VK_FORMAT_UNDEFINED, const VkComponentMapping &mapping = YAVF_DEFAULT_SWIZZLE);
    
    ImageView* createView(const VkImageViewType &type, const VkImageAspectFlags &aspect,
                          const VkFormat &format = VK_FORMAT_UNDEFINED, const VkComponentMapping &mapping = YAVF_DEFAULT_SWIZZLE);

    Internal::ImageInfo & info();
    VkImage handle() const;
    VmaAllocation allocation() const;
    void* ptr() const;
  private:
    VmaMemoryUsage memUsage;
    size_t internalIndex;

    Device* dev;
    void* pointer;

    size_t viewSize;
    ImageView* imageView;

    Internal::ImageInfo parameters;
  };
  
  struct DescriptorSetData {
    struct BufferData {
      Buffer* buffer;
      size_t offset;
      size_t range;
      
      BufferData() : buffer(nullptr), offset(0), range(0) {}
      BufferData(const BufferData &data) : buffer(data.buffer), offset(data.offset), range(data.range) {}
      BufferData(Buffer* buffer, const size_t &offset, const size_t &range) : buffer(buffer), offset(offset), range(range) {}
      ~BufferData() {}
      
      BufferData & operator=(const BufferData &data) {
        buffer = data.buffer; 
        offset = data.offset; 
        range = data.range;
        return *this;
      }
      
      operator VkDescriptorBufferInfo() const {
        return {
          buffer->handle(),
          offset,
          range
        };
      }
    };
    
    struct ImageData {
      Sampler sampler;
      ImageView* imageView;
      VkImageLayout imageLayout;
      
      ImageData() : sampler(VK_NULL_HANDLE), imageView(nullptr), imageLayout(VK_IMAGE_LAYOUT_MAX_ENUM) {}
      ImageData(const ImageData &data) : sampler(data.sampler), imageView(data.imageView), imageLayout(data.imageLayout) {}
      ImageData(const Sampler &sampler, ImageView* imageView, const VkImageLayout &imageLayout) : sampler(sampler), imageView(imageView), imageLayout(imageLayout) {}
      ~ImageData() {}
      
      ImageData & operator=(const ImageData &data) {
        sampler = data.sampler; 
        imageView = data.imageView; 
        imageLayout = data.imageLayout;
        return *this;
      }
      
      operator VkDescriptorImageInfo() const {
        return {
          sampler,
          imageView->handle(),
          imageLayout
        };
      }
    };
    
    DescriptorSetData() {
//       memset(&imageData, 0, sizeof(ImageData));
      
      arrayElement = 0;
      bindingNum = 0;
      descType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
    
    DescriptorSetData(const DescriptorSetData &data) {
      bufferData = data.bufferData;
      imageData = data.imageData;
      view = data.view;
      arrayElement = data.arrayElement;
      bindingNum = data.bindingNum;
      descType = data.descType;
    }
    
    DescriptorSetData(Buffer* buffer, const size_t &offset, const size_t &range, const uint32_t &arrayElement, const uint32_t &bindingNum, const VkDescriptorType &descType)
    : bufferData(buffer, offset, range), arrayElement(arrayElement), bindingNum(bindingNum), descType(descType) {}
    DescriptorSetData(const Sampler &sampler, ImageView* imageView, const VkImageLayout &imageLayout, const uint32_t &arrayElement, const uint32_t &bindingNum, const VkDescriptorType &descType)
    : imageData(sampler, imageView, imageLayout), arrayElement(arrayElement), bindingNum(bindingNum), descType(descType) {}
    DescriptorSetData(BufferView* view, const uint32_t &arrayElement, const uint32_t &bindingNum, const VkDescriptorType &descType)
    : view(view), arrayElement(arrayElement), bindingNum(bindingNum), descType(descType) {}
    
    DescriptorSetData & operator=(const DescriptorSetData &data) {
//       memcpy(&imageData, &data.imageData, sizeof(ImageData));
      bufferData = data.bufferData;
      imageData = data.imageData;
      view = data.view;
      arrayElement = data.arrayElement;
      bindingNum = data.bindingNum;
      descType = data.descType;
      return *this;
    }
    
    ~DescriptorSetData() {}
    
//     union {
      BufferData bufferData;
      
      ImageData imageData;
      
      BufferView* view;
//     };
    
    uint32_t arrayElement;
    uint32_t bindingNum;
    VkDescriptorType descType;
  };
  
  class DescriptorSet {
  public:
    DescriptorSet(Device* device, VkDescriptorSet h);
    ~DescriptorSet();
    
    size_t add(const DescriptorSetData &data);
    void erase(const size_t &index);
    void update(const size_t &index = SIZE_MAX);
    
    size_t size() const;
    VkDescriptorSet handle() const;
//     VkDescriptorType type() const;
    
    DescriptorSetData & operator[](const size_t &index);
    const DescriptorSetData & operator[](const size_t &index) const;
    
    DescriptorSetData & at(const size_t &index);
    const DescriptorSetData & at(const size_t &index) const;
  private:
    size_t freeData;
    std::vector<DescriptorSetData> datas;
    Device* device;
    VkDescriptorSet h;
  };
  
//   class DescriptorSet {
//   public:
//     DescriptorSet();
//     DescriptorSet(Internal::DescriptorSetBase* ptr);
//     ~DescriptorSet();
//     
//     bool valid() const;
//     
//     Internal::DescriptorSetBase* operator->() const;
//     
//     DescriptorSetData & operator[](const size_t &index);
//     const DescriptorSetData & operator[](const size_t &index) const;
//     
//     DescriptorSet & operator=(const DescriptorSet &another);
//     
//     operator VkDescriptorSet() const;
//     bool operator==(const DescriptorSet &another) const;
//     bool operator!=(const DescriptorSet &another) const;
//   private:
//     Internal::DescriptorSetBase* ptr;
//   };
//   
//   namespace Internal {
// //     class SamplerBase {
// //     public:
// //       SamplerBase(VkSampler sampler);
// //       ~SamplerBase();
// //       
// //       VkSampler handle() const;
// // 
// //       DescriptorSet descriptorSet() const;
// //       size_t descriptorSetIndex() const;
// //       
// //       //const uint32_t &binding, const uint32_t &arrayNum
// //       void setDescriptor(const DescriptorSet &set, const size_t &index);
// //       
// //       operator VkSampler() const;
// //       bool operator==(const SamplerBase &another) const;
// //       bool operator!=(const SamplerBase &another) const;
// //     private:
// //       VkSampler sampler;
// //       DescriptorSet setHandle;
// //       size_t resIndex;
// //     };
//     
//     class BufferViewBase {
//       friend Device;
//     public:
//       struct CreateInfo {
//         VkFormat formatVar;
//         size_t internalIndex;
//         size_t offsetVar;
//         size_t sizeVar;
//         VkBufferView h;
//         
//         Buffer relatedBuffer;
//         Device* device;
//       };
//       BufferViewBase(const CreateInfo &info);
//       ~BufferViewBase();
//       
//       VkFormat format() const; 
//       size_t offset() const; 
//       size_t size() const;
//       
//       DescriptorSet descriptorSet() const;
//       size_t descriptorSetIndex() const;
//       void setDescriptor(const DescriptorSet &set, const size_t &index);
//       
//       VkBufferView handle() const;
//       Buffer buffer() const;
//       
//       BufferViewBase* next() const;
//       void setNext(BufferViewBase* ptr);
//       
//       operator VkBufferView() const;
//       bool operator==(const BufferViewBase &another) const;
//       bool operator!=(const BufferViewBase &another) const;
//       
//       void recreate();
//     private:
//       VkFormat formatVar;
//       size_t internalIndex;
//       size_t offsetVar;
//       size_t sizeVar;
//       VkBufferView h;
//       
//       Buffer relatedBuffer;
//       Device* device;
//       
//       DescriptorSet set;
//       size_t setIndex;
//       
//       BufferViewBase* nextPtr;
//     };
//     
//     class BufferBase {
//       friend Device;
//     public:
//       struct CreateInfo {
//         Device* device;
//         size_t internalIndex;
//         VmaMemoryUsage memUsage;
//         
//         void* pointer;
//         VkBuffer h;
//         VmaAllocation a;
//         Internal::BufferInfo parameters;
//       };
//       BufferBase(const CreateInfo &info);
//       ~BufferBase();
//       
//       void recreate(const size_t &newSize);
//       void flush() const;
//       
//       // что с вью? их может быть несколько, и их нужно обновлять каждый раз как я переделываю буфер
//       // придется хранить их всех в буфере
//       size_t viewCount() const;
//       BufferView view(const size_t &index = 0) const;
//       BufferView createView(const VkFormat &format, const size_t &offset = 0, const size_t &size = VK_WHOLE_SIZE);
//       
//       DescriptorSet descriptorSet() const;
//       size_t descriptorSetIndex() const;
//       void setDescriptor(const DescriptorSet &set, const size_t &index);
//       
//       Internal::BufferInfo & info();
//       VkBuffer handle() const;
//       VmaAllocation allocation() const;
//       void* ptr() const;
//       
//       operator VkBuffer() const;
//       bool operator==(const BufferBase &another) const;
//       bool operator!=(const BufferBase &another) const;
//     private:
//       VmaMemoryUsage memUsage;
//       size_t internalIndex;
// 
//       Device* device;
//       void* pointer;
// 
//       VkBuffer h;
//       VmaAllocation a;
//       
//       size_t viewSize;
//       BufferView bufferView;
//       
//       DescriptorSet set;
//       size_t setIndex;
// 
//       Internal::BufferInfo parameters;
//     };
//     
//     class ImageViewBase {
//       friend Device;
//     public:
//       struct CreateInfo {
//         VkImageViewType type;
//         VkFormat format;
//         VkComponentMapping components;
//         VkImageSubresourceRange subresourceRange;
//         VkImageView h;
//         
//         size_t internalIndex;
//         Image image;
//         Device* device;
//       };
//       ImageViewBase(const CreateInfo &info);
//       ~ImageViewBase();
//       
//       VkImageViewType type() const;
//       VkFormat format() const;
//       VkComponentMapping components() const;
//       VkImageSubresourceRange subresourceRange() const;
//       VkImageView handle() const;
//       
//       Image image() const;
//       
//       DescriptorSet descriptorSet() const;
//       size_t descriptorSetIndex() const;
//       void setDescriptor(const DescriptorSet &res, const size_t &resIndex);
//       
//       ImageViewBase* next() const;
//       void setNext(ImageViewBase* ptr);
//       
//       operator VkImageView() const;
//       bool operator==(const ImageViewBase &another) const;
//       bool operator!=(const ImageViewBase &another) const;
//       
//       void recreate();
//     private:
//       VkImageViewType typeVar;
//       VkFormat formatVar;
//       VkComponentMapping componentsVar;
//       VkImageAspectFlags aspectVar;
//       VkImageSubresourceRange subresourceRangeVar;
//       VkImageView h;
//       
//       size_t internalIndex;
//       Image imageVar;
//       Device* device;
//       
//       // нужен ли мне здесь сэмплер?
//       //Sampler sampler;
//       
//       DescriptorSet res;
//       size_t resIndex;
//       
//       ImageViewBase* nextPtr;
//     };
//     
//     class ImageBase {
//       friend Device;
//     public:
//       struct CreateInfo {
//         VmaMemoryUsage memUsage;
//         size_t internalIndex;
// 
//         Device* dev;
//         void* pointer;
// 
//         VkImage h;
//         VmaAllocation a;
// 
//         Internal::ImageInfo parameters;
//       };
//       ImageBase(const CreateInfo &info);
//       ~ImageBase();
//       
//       void recreate(const VkExtent3D &newSize);
//       void flush() const;
//       
//       size_t viewCount() const;
//       ImageView view(const size_t &index = 0);
//       ImageView createView(const VkImageViewType &type, const VkImageSubresourceRange &range,
//                            const VkFormat &format = VK_FORMAT_UNDEFINED, const VkComponentMapping &mapping = YAVF_DEFAULT_SWIZZLE);
// 
//       Internal::ImageInfo & info();
//       VkImage handle() const;
//       VmaAllocation allocation() const;
//       void* ptr() const;
//       
//       operator VkImage() const;
//       bool operator==(const ImageBase &another) const;
//       bool operator!=(const ImageBase &another) const;
//     private:
//       VmaMemoryUsage memUsage;
//       size_t internalIndex;
// 
//       Device* dev;
//       void* pointer;
// 
//       VkImage h;
//       VmaAllocation a;
// 
//       size_t viewSize;
//       ImageView imageView;
// 
//       Internal::ImageInfo parameters;
//     };
//     
//     class DescriptorSetBase {
//     public:
//       DescriptorSetBase(Device* device, VkDescriptorSet h, VkDescriptorType descType);
//       ~DescriptorSetBase();
//       
//       size_t add(const DescriptorSetData &data);
//       void erase(const size_t &index);
//       void update(const size_t &index = SIZE_MAX);
//       
//       size_t size() const;
//       VkDescriptorSet handle() const;
//       VkDescriptorType type() const;
//       
//       DescriptorSetData & operator[](const size_t &index);
//       const DescriptorSetData & operator[](const size_t &index) const;
//       
//       DescriptorSetData & at(const size_t &index);
//       const DescriptorSetData & at(const size_t &index) const;
//       
//       void operator=(const DescriptorSetBase &another) = delete;
//       
//       operator VkDescriptorSet() const;
//       bool operator==(const DescriptorSetBase &another) const;
//       bool operator!=(const DescriptorSetBase &another) const;
//     private:
//       size_t freeData;
//       std::vector<DescriptorSetData> datas;
//       Device* device;
//       VkDescriptorSet h;
//       VkDescriptorType descType;
//     };
//   }

//  template <typename T>
//  void resize(const size_t &newSize, const size_t &copySize) {
//    if (parameters.size == newSize) return;
//
//    const size_t finalSize = memUsage == VMA_MEMORY_USAGE_CPU_ONLY ?
//                             alignMemorySize(std::max(device->getMinMemoryMapAlignment(), device->getNonCoherentAtomSize()), newSize) :
//                             //alignMemorySize(device->getNonCoherentAtomSize(), newSize);
//                             newSize;
//
//    const size_t finalCopySize = std::min(std::min(parameters.size, finalSize), copySize);
//    parameters.size = finalSize;
//
//    const VmaAllocationCreateInfo alloc{
//            static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
//            memUsage,
//            0, 0, 0, VK_NULL_HANDLE, nullptr
//    };
//
//    VmaAllocationInfo data;
//
//    VkBuffer buffer;
//    VmaAllocation allocation;
//    vmaCreateBuffer(allocator, &parameters._info, &alloc, &buffer, &allocation, &data);
//
//    memcpy(data.pMappedData, pointer, copySize);
//
//    vmaDestroyBuffer(allocator, obj, RAIIType4::allocation);
//    obj = buffer;
//    RAIIType4::allocation = allocation;
//    pointer = data.pMappedData;
//
//    BufferView* view = bufferView;
//    while (view != nullptr) {
//      view->recreate();
//      view = view->next();
//    }
//
//    if (set != nullptr) {
//      set->at(setIndex).bufferData.range = finalSize;
//      set->update(setIndex);
//    }
//  }

}
#endif
