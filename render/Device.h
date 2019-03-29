#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include <string>
#include <chrono>
#include <set>

#include "vulkan/vulkan.h"
#include "QueueFamily.h"
#include "FileTools.h"
//#include "ShaderTools.h"
#include "Structures.h"

//class Device;
//#include "Memory.h"
#include "Memory1.h"

// возможное количество текстурок
#define MAX_TEXTURES 2048
#define MIN_MEMORY_SIZE 1440000
#define MIN_IMAGE_MEMORY_SIZE 5242880
// 393216
// как выяснилось 1665 это либо единственный тип, либо тип который покрывает все что нужно (на AMD картах он другой)
#define DEFAULT_MEMORY_TYPE_BITS 1665

#define VK_CHECK_RESULT(res, msg) if (res != VK_SUCCESS) { \
    if (res == VK_ERROR_OUT_OF_HOST_MEMORY) std::cout << "Error: VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl; \
    if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY) std::cout << "Error: VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl; \
    if (res == VK_ERROR_INITIALIZATION_FAILED) std::cout << "Error: VK_ERROR_INITIALIZATION_FAILED" << std::endl; \
    if (res == VK_ERROR_DEVICE_LOST) std::cout << "Error: VK_ERROR_DEVICE_LOST" << std::endl; \
    if (res == VK_ERROR_MEMORY_MAP_FAILED) std::cout << "Error: VK_ERROR_MEMORY_MAP_FAILED" << std::endl; \
    if (res == VK_ERROR_LAYER_NOT_PRESENT) std::cout << "Error: VK_ERROR_LAYER_NOT_PRESENT" << std::endl; \
    if (res == VK_ERROR_EXTENSION_NOT_PRESENT) std::cout << "Error: VK_ERROR_EXTENSION_NOT_PRESENT" << std::endl; \
    if (res == VK_ERROR_FEATURE_NOT_PRESENT) std::cout << "Error: VK_ERROR_FEATURE_NOT_PRESENT" << std::endl; \
    if (res == VK_ERROR_INCOMPATIBLE_DRIVER) std::cout << "Error: VK_ERROR_INCOMPATIBLE_DRIVER" << std::endl; \
    if (res == VK_ERROR_TOO_MANY_OBJECTS) std::cout << "Error: VK_ERROR_TOO_MANY_OBJECTS" << std::endl; \
    if (res == VK_ERROR_FORMAT_NOT_SUPPORTED) std::cout << "Error: VK_ERROR_FORMAT_NOT_SUPPORTED" << std::endl; \
    if (res == VK_ERROR_FRAGMENTED_POOL) std::cout << "Error: VK_ERROR_FRAGMENTED_POOL" << std::endl; \
    Global::console()->print(msg); \
    throw std::runtime_error(msg); \
  } 

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct PipelineCreationInfo {
  std::vector<VkPipelineShaderStageCreateInfo> shaders;
  std::vector<VkVertexInputBindingDescription> bindingDescriptions;
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  std::vector<VkPipelineColorBlendAttachmentState> colorBlendings;
  VkPolygonMode mode = VK_POLYGON_MODE_FILL;
  //VkPipelineLayout layout = VK_NULL_HANDLE;
  uint32_t layoutIndex = UINT32_MAX;
  VkBool32 depthTest = VK_TRUE;
  VkBool32 depthWrite = VK_TRUE;
  uint32_t renderPassIndex = 0;
};

struct PipelineLayout {
  VkPipelineLayout handle = VK_NULL_HANDLE;
  VkDescriptorSet  set    = VK_NULL_HANDLE;
};

struct Pipeline {
  VkPipeline handle           = VK_NULL_HANDLE;
  uint32_t layoutIndex        = UINT32_MAX;
//   VkPipelineLayout* layout    = nullptr;
//   VkDescriptorSet pipelineSet = VK_NULL_HANDLE;
};

struct JobInfo {
  VkQueueFlags        queueFlags;
  bool                present;
  uint32_t            submitCount;
  const VkSubmitInfo* submitInfo;
  VkFence*            fence = nullptr;
};

struct ImageParameters {
  VkImage        handle  = VK_NULL_HANDLE;
  VkImageView    view    = VK_NULL_HANDLE;
  VkDeviceMemory memory  = VK_NULL_HANDLE;
};

struct BufferParameters {
  VkBuffer       handle = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  uint32_t       size   = 0;
};

// struct BufferObject {
//   Object                   *parameters   = nullptr;
//   //BufferParameters          parameters;
//   Memory                   *memPtr       = nullptr;
//   void                     *mapMemoryPtr = nullptr;
//   VkDescriptorSet           descriptor   = VK_NULL_HANDLE;
//   // сумма оффсетов всегда начинается с нуля и должна быть равна величине буффера!!!
//   std::vector<VkDeviceSize> offsets;
//   uint32_t                  indicesSize = 0;
//   bool                      allocated   = false;
// };

// struct ImageObject {
//   ImageParameters parameters;
//   VkDescriptorSet descriptor = VK_NULL_HANDLE;
//   VkDeviceSize    size       = VK_NULL_HANDLE;
//   VkExtent3D      sizeWHD;
//   bool            allocated  = false;
// };

struct Sampler {
  VkSampler       handle     = VK_NULL_HANDLE;
  VkDescriptorSet descriptor = VK_NULL_HANDLE;
};

// скорее всего нужно будет кардинально переделать, возможно даже создать отдельный класс
// неплохо работает пока что и так
struct VirtualFrame {
  // command buffers, semaphores, fences and framebuffers
  VkFramebuffer   framebuffer                = VK_NULL_HANDLE;
  VkCommandBuffer commandBuffer              = VK_NULL_HANDLE;
  VkSemaphore     imageAvailableSemaphore    = VK_NULL_HANDLE;
  VkSemaphore     finishedRenderingSemaphore = VK_NULL_HANDLE;
  VkFence         fence                      = VK_NULL_HANDLE;
};

// дополнительные командные буферы и заборы для операций вычислений и копирования
// так же сюда буду складывать временные объекты
// не тестировал пока что вычисления
struct AdditionalResources {
  VkCommandBuffer  commandBuffer = VK_NULL_HANDLE;
  VkFence          fence         = VK_NULL_HANDLE;
  // на всякий случай метка доступности ресурса (в многопоточном приложении блокировку скорее всего нужно будет переделать (синхронизации))
  bool             free = false;
  BufferParameters temporaryBufferObject;
  ImageParameters  temporaryImageObject;
};

struct SwapChain {
  VkSwapchainKHR handle = VK_NULL_HANDLE;
  std::vector<VkImage> images;
  std::vector<VkImageView> imageViews;
  VkFormat imageFormat;
  VkExtent2D extent;
};

struct QueueFamilyInfo {
  uint32_t queueCount = 0;
  VkQueueFlags flags = 0;
  VkBool32 present = VK_FALSE;
};

class Device {
  friend class VulkanRender;
public:
  Device();
  ~Device();
  
  void clean();
  
  void init(const VkPhysicalDevice &physicalDevice, 
            const std::vector<const char*> &deviceExtensions, 
            const bool &enableValidationLayers, 
            const std::vector<const char*> &validationLayers, 
            const VkSurfaceKHR &surface);
  void createSwapChain(const SwapChainSupportDetails &swapChainSupport, 
                       const VkSurfaceFormatKHR &surfaceFormat, 
                       const VkPresentModeKHR &presentMode, 
                       const VkExtent2D &extent,
                       const VkSurfaceKHR &surface);
  void createImageViews();
  // нужны какие-то настройки для рендер пасса
  void createRenderPass();
  
//   void createDepthSwapChain(const SwapChainSupportDetails &swapChainSupport, 
//                        const VkSurfaceFormatKHR &surfaceFormat, 
//                        const VkPresentModeKHR &presentMode, 
//                        const VkExtent2D &extent,
//                        const VkSurfaceKHR &surface);
  //void createDepthImageViews();
  void createOcclusionDepthImage();
  void createDepthRenderPass();
  
  void createPipelineLayout(const std::vector<VkPushConstantRange> &ranges, VkPipelineLayout* layout);
  uint32_t createPipelineLayout(const std::vector<VkPushConstantRange> &ranges, const std::vector<VkDescriptorSetLayout> &descLayouts);
  
  // переделал создание пайплайна, теперь создаются пачками и необходимо заполнить специальную структуру
  void createGraphicPipelines(const std::vector<PipelineCreationInfo> &pipelineCrationInfos);
  
//   uint32_t createNewGraphicPipeline(std::string vert, 
//                                     std::string frag, 
//                                     std::vector<VkVertexInputBindingDescription> bindingDescriptions, 
//                                     std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
//                                     VkPipelineInputAssemblyStateCreateInfo inputAssembly);
  //int createNewComputePipeline();
  
  // создаются для каждого устройства по отдельности
  int createCommandPool(const VkQueueFlags &flags, const bool &present, const VkCommandPoolCreateFlags &commandPoolFlags);
  void createDepthResources();
  void createVirtualFrames(int graphicsCommandPoolIndex);
  void createDesriptorSet();
  // выделяем память для приложения
  void createMemory(const VkDeviceSize &size);
  
  uint16_t getResIndex();
  uint16_t getResIndex(ImageParameters param);
  uint16_t getResIndex(BufferParameters param);
  VkResult waitForRes(uint16_t index);
  void blockResource(uint16_t index);
  void unblockResource(uint16_t index);
  void waitResource(uint16_t index, uint64_t nanoSec);
  
  // операции с буферами нужно продумать глубже:
  // сколько буферов у меня может быть всего? если переиспользовать меши, то в принципе буферов будет не так много
  // как будет вести себя память при очистке определенного буфера? 
  // когда мне нужно будет чистить буферы? опять же при переиспользовании буферов, их скорее всего чистить надо будет только в конце игры
  // не займу ли я всю память буферами, если буферы будут выделяться для каждого меша? переиспользование позволит выделить буферы для каждого меша
  // Возвращает индекс буфера, в который только что записали данные
//   uint32_t createBuffer(VkDeviceSize size,
//                         VkBufferUsageFlags usage,
//                         VkMemoryPropertyFlags properties,
//                         std::vector<VkDeviceSize> offsets,
//                         uint32_t indicesSize);
//   uint32_t createBuffer(VkDeviceSize size,
//                         VkBufferUsageFlags usage,
//                         VkMemoryPropertyFlags properties,
//                         std::vector<VkDeviceSize> offsets);
  Buffer createBuffer(const VkDeviceSize &size,
                      const VkBufferUsageFlags &usage,
                      const VkMemoryPropertyFlags &properties,
                      const uint32_t &dataCount);
  
  void createStagingBuffer(VkDeviceSize size, 
                           VkBufferUsageFlags usage, 
                           VkMemoryPropertyFlags properties, 
                           uint16_t resIndex);
  void createStagingBuffer(VkDeviceSize size, 
                           VkBufferUsageFlags usage, 
                           VkMemoryPropertyFlags properties, 
                           VkBuffer& buffer, 
                           VkDeviceMemory& bufferMemory);
  void copyBuffer(const VkBuffer &srcBuffer, const VkBuffer &dstBuffer, const VkDeviceSize &size, const uint16_t &resIndex);
  //void copyBuffer(const uint16_t &resIndex, const uint32_t &dstBuffer, const VkDeviceSize &size);
  void copyBuffer(const uint16_t &resIndex, const Buffer &dstBuffer, const VkDeviceSize &size);
  //void bufferBarrier(VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
  void copyQueryPoolResults(const uint16_t &resIndex,
                            const uint32_t &firstQuery, 
                            const uint32_t &queryCount, 
                            const Buffer &dstBuffer, 
                            const VkDeviceSize &dstOffset, 
                            const VkDeviceSize &stride, 
                            const VkQueryResultFlags &flags);
  
  // операции с картинками 
  void createImage(uint32_t width,
                   uint32_t height,
                   VkFormat format,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   uint16_t resIndex);
  void createImage(uint32_t width,
                   uint32_t height,
                   VkFormat format,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage& stagingImage, 
                   VkDeviceMemory& stagingImageMemory);
//   uint32_t createImage(uint32_t width,
//                        uint32_t height,
//                        VkFormat format,
//                        VkImageTiling tiling,
//                        VkImageUsageFlags usage,
//                        VkMemoryPropertyFlags properties,
//                        VkDeviceSize imageSize);
  Image createImage(const uint32_t &width,
                    const uint32_t &height,
                    const VkFormat &format,
                    const VkImageTiling &tiling,
                    const VkImageUsageFlags &usage,
                    const VkMemoryPropertyFlags &properties,
                    const VkDeviceSize &imageSize);
  void transitionImageLayout(const VkImage &image, const VkImageLayout &oldLayout, const VkImageLayout &newLayout, const uint16_t &resIndex);
  void transitionImageLayout(const uint16_t &resIndex, const VkImageLayout &oldLayout, const VkImageLayout &newLayout);
  //void transitionImageLayout(uint32_t imageIndex, VkImageLayout oldLayout, VkImageLayout newLayout, uint16_t resIndex);
  void transitionImageLayout(const Image &image, const VkImageLayout &oldLayout, const VkImageLayout &newLayout, const uint16_t &resIndex);
  void copyImage(const VkImage &srcImage, const VkImage &dstImage, const uint32_t &width, const uint32_t &height, const uint16_t &resIndex);
  void copyImage(const uint16_t &resIndex, const Image &dstImage, const uint32_t &width, const uint32_t &height);
  void copyImageToBuffer(const VkImage &srcImage, const VkBuffer &dstBuffer, const uint16_t &resIndex);
  void blitImage(const VkImage &srcImage, 
                 const VkImageLayout &srcImageLayout, 
                 const VkImage &dstImage, 
                 const VkImageLayout &dstImageLayout, 
                 const uint32_t &width, 
                 const uint32_t &height, 
                 const VkFilter& filter,
                 const uint16_t &resIndex);
  //void createImageView(const uint32_t &imageIndex, const VkFormat &format, const VkImageAspectFlags &aspectFlags);
  void createImageView(const Image &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags);
  // возможно стоит добавить пару настроек
  void createTextureSampler(const VkFilter &magFilter, const VkFilter &minFilter);
  //void updateDescriptorSets(const uint32_t &index);
  void updateDescriptorSets(const Image &image);
  void updateDescriptorSets1(const Image &image);
  
  int addJob(const JobInfo &info);
  VkResult presentImage(const VkPresentInfoKHR &presentInfo, const int &queueIndex);
  
  int pickQueueFamilies(const VkQueueFlags &flags, const bool &presentSupport);
  
  // команды
  //void setCurrentCommandBuffer(uint32_t commandBufferIndex);
  //VkCommandBuffer* getCurrentCommandBuffer();
  //uint32_t createCommandBuffer(int graphicsCommandPoolIndex, VkCommandBufferLevel level);
  //void setInheritanceInfo(uint32_t subpass);
  void beginCommandBuffer(const VkCommandBufferUsageFlags &flags);
  void setViewport();
  void setScissor();
  void resetQueryPool(const uint32_t &queries);
  void bindPipeline(const bool &drawing, const uint32_t &pipelineIndex);
  void bindGraphicPipeline(const uint32_t &pipelineIndex);
  void bindComputePipeline(const uint32_t &pipelineIndex);
  //void pushConst();
  // непонятна ситуация с буферами:
  // выгоднее держать много вершинных данных в одном вершинном буфере (тоже и с индексными)
  // на практике часто ли это получиться делать?
//   void bindVertexBuffer(uint32_t bufferIndex);
//   void bindVertexBuffer(uint32_t bufferIndex, VkDeviceSize offset);
//   void bindVertexBuffer(uint32_t firstBinding, uint32_t bufferIndex, VkDeviceSize offset);
//   void bindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, uint32_t* bufferIndex, VkDeviceSize* offsets);
//   void bindIndexBuffer(uint32_t bufferIndex);
  void bindVertexBuffer(const Buffer &buffer);
  void bindVertexBuffer(const Buffer &buffer, const VkDeviceSize &offset);
  void bindVertexBuffer(const uint32_t &firstBinding, const Buffer &buffer, const VkDeviceSize &offset);
  void bindVertexBuffer(const uint32_t &firstBinding, const uint32_t &bindingCount, const Buffer *buffer, const VkDeviceSize* offsets);
  void bindIndex32bBuffer(const Buffer &buffer);
  void bindIndex16bBuffer(const Buffer &buffer);
  // возможно добавится какой-нибудь индекс
  void pushConstants(const uint32_t &pipelineIndex, const VkShaderStageFlags &stage, const VkDeviceSize &offset, const VkDeviceSize &size, void* data);
  void bindDescriptorSets(const Texture &texture);
  void bindDescriptorSets1(const Texture &texture);
  void bindDescriptorSets(const uint32_t &firstSet, const Buffer &b);
  
  //void drawIndexed(uint32_t bufferIndex);
  void drawIndexed(const Buffer &bufferIndex);
  void drawIndexed(const uint32_t &elemCount, 
                   const uint32_t &instanceCount, 
                   const uint32_t &firstIndex, 
                   const int32_t &vertexOffset, 
                   const uint32_t &firstInstance);
  void draw(const uint32_t &vertexCount, const uint32_t &instanceCount, const uint32_t &firstVertex, const uint32_t &firstInstance);
  void beginQuery(const uint32_t &query, const uint32_t &flags);
  void endQuery(const uint32_t &query);
  void nextSubpass(const VkSubpassContents &contents);
  void endCommandBuffer();
  //void executeSecondaryCommandBuffers();
  //void clearCommandBuffers();
  
  bool allocateCommandBuffers(const uint32_t &poolIndex, const VkCommandBufferLevel &level, const uint32_t &count, VkCommandBuffer *command_buffers);
  void createQueryPool(const uint32_t &queryCount);
  void getQueryPoolResults(const uint32_t &firstQuery, const uint32_t &queryCount, const size_t &dataSize, void* data, const VkDeviceSize &stride, const VkQueryResultFlags &flags);
  void updateDescriptorSetUniform(const Buffer &uniform);
//   void updateDescriptorSetStorage(const Buffer &storage);
  void updateDescriptorSets1(const Buffer &uniform);
//   void freeDescriptorSet(const Buffer &buffer);
//   void resetStoragePool();
  
  // get'еры и set'еры
  VkPhysicalDevice getPhysicalDevice();
  VkDevice getDevice();
  VkSwapchainKHR getSwapChain();
  VkRenderPass getRenderPass(const uint32_t &renderPassIndex = 0);
  VkExtent2D getSwapchainExtent();
  void setSwapChainExtend(VkExtent2D extend);
  VkCommandPool getCommandPool(int i);
  VirtualFrame* getNextFrame();
  VkImageView getImageView(uint32_t i);
  VkImageView getDepthImageView();
  VkImageView getOcclusionImageView();
  VkImage     getImage(uint32_t i);
  VkImage     getOcclusionImage();
  VkExtent3D  getImageWHD(uint32_t i);
  void setTransferPoolIndex(int i);
  VkBool32 isPresentSupport();
  VkPipeline        getGraphicPipeline(const uint32_t &pipelineIndex);
  VkPipelineLayout  getPipelineLayout(const uint32_t &pipelineIndex);
  VkDescriptorSet   getPipelineLayoutSet(const uint32_t &pipelineIndex);
  Memory* getMemory();
  VkDescriptorSetLayout getLayoutSampler();
  VkDescriptorSetLayout getLayoutSampledImage();
  VkDescriptorSetLayout getLayoutCombineImageSampler();
  VkDescriptorSetLayout getUboSetLayout();
  VkDescriptorSetLayout getStorageSetLayout();
  uint32_t   getBasicPIndex();
  uint32_t   getBasicUVPIndex();
  uint32_t   getBasicTextPIndex();
  uint32_t   getBasicAABBPIndex();
  uint32_t   getBasicImagePIndex();
  uint32_t   getBasicSpherePIndex();
  uint32_t   getBasicGuiPIndex();
  uint32_t   getOccluderPIndex();
  uint32_t   getOccludeePIndex();
  void       setBasicPIndex(uint32_t index);
  void       setBasicUVPIndex(uint32_t index);
  void       setBasicTextPIndex(uint32_t index);
  void       setBasicAABBPIndex(uint32_t index);
  void       setBasicImagePIndex(uint32_t index);
  void       setBasicSpherePIndex(uint32_t index);
  void       setBasicGuiPIndex(uint32_t index);
  void       setOccluderPIndex(uint32_t index);
  void       setOccludeePIndex(uint32_t index);
  size_t     getGraphicPipelineSize();
  //BufferObject* getBufferObject(uint32_t bufferIndex);
//   void          setMapMemoryPtr(uint32_t bufferIndex, void* ptr);
//   void*         getMapMemoryPtr(uint32_t bufferIndex);
//   VkDeviceMemory getDeviceMemory(VkMemoryPropertyFlags properties);
  VkPhysicalDeviceMemoryProperties getDeviceMemoryProperties();
  int32_t f(const uint32_t &typefilter, const uint32_t &propeties);
  
  // ждем и чистим временные ресурсы
  void deviceWaitIdle();
  
  // выводит на экран memoryTypeBits разных буфферов
  // возвращает memory type буфферов
  uint32_t showBufferMemoryTypes();
  uint32_t showImageMemoryTypes();
private:
  // device собственно
  // VkPhysicalDevice будет неявно уничтожен после уничтожения VkInstance
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceMemoryProperties memoryProperties;
  VkDevice device = VK_NULL_HANDLE;
  VkBool32 presentSupport = VK_FALSE;
  // QueueFamilies, которые используются девайсом
  std::vector<QueueFamily> queueFamilies;
  //std::vector<Memory> deviceMemory;
  
  // Специфичные для устройства штуки вроде:
  // swapchain
  std::array<SwapChain, 1> swapchains;
//   VkSwapchainKHR swapChain = VK_NULL_HANDLE;
//   std::vector<VkImage> swapChainImages;
//   std::vector<VkImageView> swapChainImageViews;
//   VkFormat swapChainImageFormat;
//   VkExtent2D swapChainExtent;
  // render pass
  std::array<VkRenderPass, 2> renderPasses;
  //VkRenderPass renderPass = VK_NULL_HANDLE;
  // pipelines (graphical and compute)
//   VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  std::vector<PipelineLayout> layouts;
  std::vector<Pipeline> graphicsPipelines;
  
  uint32_t basicPipelineIndex       = 0;
  uint32_t basicUVPipelineIndex     = 0;
  uint32_t basicTextPipelineIndex   = 0;
  uint32_t basicAABBPipelineIndex   = 0;
  uint32_t basicImagePipelineIndex  = 0;
  uint32_t basicSpherePipelineIndex = 0;
  uint32_t basicGuiPipelineIndex    = 0;
  uint32_t occluderPipelineIndex    = 0;
  uint32_t occludeePipelineIndex    = 0;
  
  std::vector<Pipeline> computePipelines;
  
  std::vector<VkCommandPool> commandPools;
  int transferPoolIndex;
  int commandPoolIndex;
  // buffers (command and other)
//   Memory* hostVisible = nullptr;
//   Memory* deviceLocal = nullptr;
  Memory deviceMemory;
  //std::vector<BufferObject> bufferObjects;
  //std::vector<ImageObject> imageObjects;
  
  VkDescriptorPool      descriptorPool   = VK_NULL_HANDLE;
//   VkDescriptorPool      storagePool      = VK_NULL_HANDLE;
  // лайаутов будет несколько
  VkDescriptorSetLayout layoutSampler = VK_NULL_HANDLE;
  VkDescriptorSetLayout layoutSampledImage = VK_NULL_HANDLE;
  VkDescriptorSetLayout layoutCombinedImageSampler = VK_NULL_HANDLE;
  VkDescriptorSetLayout uboSetLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout storageSetLayout = VK_NULL_HANDLE;
  ImageParameters depthImage;
  ImageParameters occlusionImage;
  // два следующих параметра возможно будут в массиве
  // также массив тогда необходим дескриптору
  // с массивом наверное все очень хреново
  //ImageParameters textureImage; // массив текстурок разного размера
  Sampler texSampler; // массив разных сэмплеров
  Sampler fontSampler;
  
  VkQueryPool queryPool = VK_NULL_HANDLE;
  
  std::vector<VirtualFrame> virtualFrames;
  const int framesCount = 3;
  int currentVirtualFrameIndex = 0;
  // текушее изображение, в которое рендерятся ресурсы
  uint32_t imageIndex;
  
  std::vector<AdditionalResources> res;
  const int resCount = 3;
  
  // данные для отрисовки
  int currentPipeline = -1;
  
  static std::list<Buffer_t> nullBufferList;
  static std::list<Image_t> nullImageList;
  
  // вспомагательные функции
  void findQueueFamilies(const VkSurfaceKHR &surface, const uint32_t &numberOfQueues, std::vector<QueueFamilyInfo> &infos);
  void createAdditionalResources();
  VkFormat findDepthFormat();
  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling &tiling, const VkFormatFeatureFlags &features);
  void clearImageViews(const uint32_t &swapChainIndex);
  void createImageView(const VkImage &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags, VkImageView &imageView);
  bool allocateCommandBuffers(const VkCommandPool &pool, const VkCommandBufferLevel &level, const uint32_t &count, VkCommandBuffer *command_buffers);
  void createSemaphores(VkSemaphore &semaphore);
  void createFence(VkFence &fence);
  void beginSingleTimeCommands(const uint16_t &resourceIndex);
  void endSingleTimeCommands(const uint16_t &resIndex);
  int32_t findMemoryType(const uint32_t &typeFilter, const VkMemoryPropertyFlags &properties);
  void clearResource(const uint16_t &resourceIndex);
};

#endif // DEVICE_H
