#include "VulkanRender.h"

// #include "stbi_image_header.h"

// #define MAX_TEXTURE_LAYERS 2048

// #define FREE_DESCRIPTOR_POOL_NAME "free_descriptor_pool"

#include "Render.h"

#define NK_IMPLEMENTATION
#include "nuklear_header.h"
// #define NK_INCLUDE_FIXED_TYPES
// #define NK_INCLUDE_FONT_BAKING
// #define NK_INCLUDE_DEFAULT_FONT
// #define NK_INCLUDE_DEFAULT_ALLOCATOR
// #define NK_INCLUDE_COMMAND_USERDATA
// #define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
// #define NK_IMPLEMENTATION
// #include <nuklear/nuklear.h>

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

bool Render::isPrespective() const {
  return perspective;
}

void Render::toggleProjection() {
  perspective = !perspective;
}

Matrices* Render::getMatrices() {
  return matrices;
}

void Render::setView(const simd::mat4 &view) {
  matrices->view = view;
}

void Render::setPersp(const simd::mat4 &persp) {
  matrices->persp = persp;
}

void Render::setOrtho(const simd::mat4 &ortho) {
  matrices->ortho = ortho;
}

void Render::setCameraDir(const simd::vec4 &dir) {
  matrices->camera.dir = dir;
}

void Render::setCameraPos(const simd::vec4 &pos) {
  matrices->camera.pos = pos;
}

void Render::setCameraDim(const uint32_t &width, const uint32_t &height) {
  matrices->camera.width = width;
  matrices->camera.height = height;
}

simd::mat4 Render::getViewProj() const {
  return matrices->camera.viewproj;
}

simd::mat4 Render::getView() const {
  return matrices->view;
}

simd::mat4 Render::getPersp() const {
  return matrices->persp;
}

simd::mat4 Render::getOrtho() const {
  return matrices->ortho;
}

// VulkanRender::VulkanRender(const size_t &stageContainerSize) : Render(stageContainerSize) {
// //   imagesCount = 0;
// //   samplersCount = 0;
// //   images = VK_NULL_HANDLE;
// //   samplers = VK_NULL_HANDLE;
// //   imageLayout = VK_NULL_HANDLE;
// //   samplerLayout = VK_NULL_HANDLE;
// }

VulkanRender::VulkanRender(const CreateInfo &info) : Render(info.stageContainerSize) {
  this->instance = info.instance;
  this->device = info.device;
  this->task = info.task;
  
  yavf::DescriptorPool defaultPool = VK_NULL_HANDLE;
  {
    yavf::DescriptorPoolMaker dpm(device);
       
    defaultPool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 20)
                     .poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10)
                     .poolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10)
                     .poolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10)
                     .create(DEFAULT_DESCRIPTOR_POOL_NAME);
  }
  
  yavf::DescriptorSetLayout uniform_layout = VK_NULL_HANDLE;
  yavf::DescriptorSetLayout storage_layout = VK_NULL_HANDLE;
  {
    yavf::DescriptorLayoutMaker dlm(device);
    
    uniform_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL).create(UNIFORM_BUFFER_LAYOUT_NAME);
    storage_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL).create(STORAGE_BUFFER_LAYOUT_NAME);
  }
  
  {
    uniformCameraData = device->create(yavf::BufferCreateInfo::buffer(sizeof(CameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
    
    yavf::DescriptorMaker dm(device);
    
    auto desc = dm.layout(uniform_layout).create(defaultPool)[0];
    const size_t index = desc->add({uniformCameraData, 0, uniformCameraData->info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
    uniformCameraData->setDescriptor(desc, index);
  }
  
  {
    uniformMatrixes = device->create(yavf::BufferCreateInfo::buffer(sizeof(MatBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
    
    yavf::DescriptorMaker dm(device);
    
    auto desc = dm.layout(uniform_layout).create(defaultPool)[0];
    const size_t index = desc->add({uniformMatrixes, 0, uniformMatrixes->info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
    uniformMatrixes->setDescriptor(desc, index);
  }
  
//   for (uint32_t i = 0; i < 3; ++i) {
//     std::cout << "task " << i << " ptr " << info.task[i] << " family " << info.task[i]->getFamily() << "\n";
//   }
//   std::cout << "info.task " << info.task << "\n";
}

VulkanRender::~VulkanRender() {
  device->wait();
  
  device->destroy(uniformCameraData);
  device->destroy(uniformMatrixes);
  
//   for (uint32_t i = 0; i < arrays.size(); ++i) {
//     for (uint32_t j = 0; j < arrays[i].size(); ++j) {
//       device->destroy(arrays[i][j].texture);
//     }
//   }
  
//   device->destroy(sampler);
//   device->destroy(imageLayout);
//   device->destroy(samplerLayout);
  
//   device->destroyDescriptorPool(FREE_DESCRIPTOR_POOL_NAME);
  device->destroyDescriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
  
  // когда удалится инстанс, он же и удалит все устройства
  // по идее
}

void VulkanRender::setContextIndex(const uint32_t &index) {
  this->currentIndex = index;
}

//#define PRINT_VEC(name, vec) std::cout << name << " (" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")" << "\n";

void VulkanRender::updateCamera() {  
  if (perspective) {
    //matrices.camera.viewproj = matrices.persp * matrices.view;
    matrices->matrixes.proj = matrices->persp;
    //matrices.matrixes.invProj = glm::inverse(matrices.persp);
  } else {
    matrices->matrixes.proj = matrices->ortho;
    //matrices.matrixes.invProj = glm::inverse(matrices.ortho);
  }
  
  matrices->camera.viewproj = matrices->matrixes.proj * matrices->view;
  matrices->camera.view = matrices->view;
  matrices->matrixes.view = matrices->view;
  matrices->matrixes.invView = simd::inverse(matrices->view);
  matrices->matrixes.invProj = simd::inverse(matrices->matrixes.proj);
  matrices->matrixes.invViewProj = simd::inverse(matrices->camera.viewproj);
  
  memcpy(uniformCameraData->ptr(), &matrices->camera, sizeof(CameraData));
  memcpy(uniformMatrixes->ptr(), &matrices->matrixes, sizeof(MatBuffer));
  
  //CameraData* data = reinterpret_cast<CameraData*>(uniformCameraData->ptr());
  
//   PRINT_VEC("viewproj ", data->viewproj[0])
//   PRINT_VEC("         ", data->viewproj[1])
//   PRINT_VEC("         ", data->viewproj[2])
//   PRINT_VEC("         ", data->viewproj[3])
  
  //throw std::runtime_error("no more");
}

void VulkanRender::update(const uint64_t &time) {
  (void)time;
  
  {
    RegionLog rl("stages->update()");
    
    for(uint32_t i = 0; i < stages.size(); ++i) {
      stages[i]->begin();
    }
  }
  
//   for (uint32_t i = 0; i < 3; ++i) {
//     std::cout << "task " << i << " family " << task[currentIndex]->getFamily() << "\n";
//   }
  
  {
    RegionLog rl("stages->doWork()");
    
    for(uint32_t i = 0; i < stages.size(); ++i) {
      stages[i]->doWork(currentIndex);
//     std::cout << "stage " << i << " task " << currentIndex << " family " << task[currentIndex]->getFamily() << "\n";
    }
  }
  
//   ASSERT(task[currentIndex]->getFamily() < 4);
}

void VulkanRender::start() {
//   return;
  
  // как то так, только task'ов добавить
  //waitFence = device->submit({task});
  const VkSubmitInfo &info = task[currentIndex]->getSubmitInfo();
  const uint32_t &family = task[currentIndex]->getFamily();
  
//   std::cout << "wait size()   " << info.waitSemaphoreCount << "\n";
//   std::cout << "signal size() " << info.signalSemaphoreCount << "\n";
//   std::cout << "wait data()   " << info.pWaitSemaphores[0] << "\n";
//   std::cout << "signal data() " << info.pSignalSemaphores[0] << "\n";
//   std::cout << "command buffer " << info.pCommandBuffers[0] << "\n";
  
//   for (uint32_t i = 0; i < 3; ++i) {
//     std::cout << "task " << i << " family " << task[i]->getFamily() << "\n";
//   }
  
//   std::cout << "Family: " << family << "\n";
  waitFence = device->submit(family, 1, &info);
}

void VulkanRender::wait() {
//   return;
  
  const VkResult res = vkWaitForFences(device->handle(), 1, &waitFence.fence, VK_TRUE, 1000000000);
  //const VkResult res = vkWaitForFences(device->handle(), 1, &waitFence.fence, VK_TRUE, 100000);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("hlqhvlgvuvfgowqvuqopquwvuonvev");
  }
}

// void VulkanRender::createInstance(const std::vector<const char*> &extensions) {
// //   yavf::Instance::setExtensions(extensions);
// //   yavf::Instance::setLayers({
// //     "VK_LAYER_LUNARG_standard_validation",
// //     "VK_LAYER_LUNARG_api_dump",
// //     "VK_LAYER_LUNARG_assistant_layer"
// //   });
// //   
// //   instance.create(VK_API_VERSION_1_0, "TheMadnessReturns", 0, "DevilsEngine1", 0);
// }

yavf::Instance* VulkanRender::getInstance() {
  return instance;
}

yavf::Buffer* VulkanRender::getCameraDataBuffer() const {
  return uniformCameraData;
}

yavf::Buffer* VulkanRender::getMatrixesBuffer() const {
  return uniformMatrixes;
}

// void VulkanRender::setData(const Data &data) {
//   this->device = data.device;
//   this->task = data.task;
//   
//   yavf::DescriptorPool defaultPool = VK_NULL_HANDLE;
//   {
//     yavf::DescriptorPoolMaker dpm(device);
//     
// //     dpm.flags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
// //        .poolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10)
// //        .poolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 10)
// //        .create(FREE_DESCRIPTOR_POOL_NAME);
//        
//     defaultPool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 20)
//                      .poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10)
//                      .poolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10)
//                      .create(DEFAULT_DESCRIPTOR_POOL_NAME);
//   }
//   
//   yavf::DescriptorSetLayout uniform_layout = VK_NULL_HANDLE;
//   
//   {
//     yavf::DescriptorLayoutMaker dlm(device);
//     
//     uniform_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL).create("uniform_layout");
//   }
//   
//   {
//     const yavf::BufferCreateInfo info{
//       0,
//       sizeof(CameraData),
//       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//       1,
//       VMA_MEMORY_USAGE_CPU_ONLY
//     };
//     
//     uniformCameraData = device->createBuffer(info);
//     
//     yavf::DescriptorMaker dm(device);
//     
//     auto desc = dm.layout(uniform_layout).create(defaultPool);
//     
//     const yavf::DescriptorUpdate du{
//       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//       0,
//       0,
//       desc[0]
//     };
//     uniformCameraData->setDescriptorData(du);
//   }
//   
//   {
//     const yavf::BufferCreateInfo info{
//       0,
//       sizeof(MatBuffer),
//       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//       1,
//       VMA_MEMORY_USAGE_CPU_ONLY
//     };
//     
//     uniformMatrixes = device->createBuffer(info);
//     
//     yavf::DescriptorMaker dm(device);
//     
//     auto desc = dm.layout(uniform_layout).create(defaultPool);
//     
//     const yavf::DescriptorUpdate du{
//       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//       0,
//       0,
//       desc[0]
//     };
//     uniformMatrixes->setDescriptorData(du);
//   }
// }

// yavf::Buffer* VulkanRender::getCameraDataBuffer() const {
//   return uniformCameraData;
// }
// 
// yavf::Buffer* VulkanRender::getMatrixesBuffer() const {
//   return uniformMatrixes;
// }
// 
// uint32_t VulkanRender::imageCount() const {
//   return imagesCount;
// }
// 
// yavf::Descriptor VulkanRender::imageDescriptor() const {
//   return images;
// }
// 
// yavf::DescriptorSetLayout VulkanRender::imageSetLayout() const {
//   return imageLayout;
// }
// 
// uint32_t VulkanRender::samplerCount() const {
//   return samplersCount;
// }
// 
// yavf::Descriptor VulkanRender::samplerDescriptor() const {
//   return samplers;
// }
// 
// yavf::DescriptorSetLayout VulkanRender::samplerSetLayout() const {
//   return samplerLayout;
// }
// 
// void VulkanRender::precacheLayers(const VkExtent2D &size, const uint32_t &count) {
//   uint32_t newCount = count;
//   auto itr = arrayIndices.find(size);
//   if (itr == arrayIndices.end()) {
//     const uint32_t index = arrays.size();
//     arrays.emplace_back();
//     auto tmp = arrayIndices.insert(std::make_pair(size, index));
//     itr = tmp.first;
//   }
//   
//   uint32_t mipmapCount = (uint32_t)glm::floor(glm::log2(float(glm::max(size.width, size.height)))) + 1;
//   std::cout << "texture size: width " << size.width << " height " << size.height << " mipmap count " << mipmapCount << "\n";
//   
//   while (newCount > 0) {
//     const uint32_t layersCount = std::min(uint32_t(MAX_TEXTURE_LAYERS), newCount);
//     
//     const yavf::ImageCreateInfo imageInfo{
//       0,
//       VK_IMAGE_TYPE_2D,
//       VK_FORMAT_R8G8B8A8_UNORM,
//       {size.width, size.height, 1},
//       mipmapCount,
//       layersCount,
//       VK_SAMPLE_COUNT_1_BIT,
//       VK_IMAGE_TILING_OPTIMAL,
//       VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//       VK_IMAGE_ASPECT_COLOR_BIT,
//       VMA_MEMORY_USAGE_GPU_ONLY
//     };
//     
//     yavf::Image* image = device->createImage(imageInfo);
//     image->createView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT);
//     
// //     std::cout << "Precached image view: " << image->view().handle << "\n";
//     
//     yavf::DescriptorUpdater du(device);
//     
//     //du.currentSet(imageDescriptor.handle).begin(0, currentArrayElement, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE).image(image).update();
//     du.currentSet(images).begin(0, currentArrayElement, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE).image(image).update();
//     const uint32_t index = itr->second;
//     arrays[index].push_back({image, 0, currentArrayElement});
//     ++currentArrayElement;
//     
//     newCount -= layersCount;
//   }
// }
// 
// void mipmapping(const uint32_t &width, const uint32_t &height, const uint32_t &miplevels, std::vector<VkImageBlit> &mipmaps) {
//   for (uint32_t i = 1; i < miplevels; ++i) {
//     VkImageBlit blit{
//       {
//         VK_IMAGE_ASPECT_COLOR_BIT,
//         i-1, 0, 1 // прежде чем приступать к генерации мипмап, необходимо решить, как быть с layer
//       },
//       {{0, 0, 0}, {int32_t(width >> (i-1)), int32_t(height >> (i-1)), 1}},
//       {
//         VK_IMAGE_ASPECT_COLOR_BIT,
//         i, 0, 1
//       },
//       {{0, 0, 0}, {int32_t(width >> i), int32_t(height >> i), 1}}
//     };
// 
//     mipmaps.push_back(blit);
//   }
// }
// 
// void copy(const uint32_t &startingLayer, const uint32_t &count, const uint32_t &rows, const uint32_t &columns, const VkExtent3D &size, std::vector<VkImageCopy> &copies) {
//   uint32_t counter = 0;
//   uint32_t width = 0;
//   uint32_t height = 0;
//   
//   //std::cout << "count " << count << "\n";
//   for (uint32_t i = 0; i < rows; ++i) {
//     if (counter == count) break;
//     height = size.height * i;
//     // std::cout << "Height: " << height << "\n";
//     width = 0;
// 
//     for (uint32_t j = 0; j < columns; ++j) {
//       if (counter == count) break;
//       width = size.width * j;
//       // std::cout << "Width: " << width << "\n";
// 
//       VkImageCopy copy{
//         {
//           VK_IMAGE_ASPECT_COLOR_BIT,
//           0, 0, 1
//         },
//         {int32_t(width), int32_t(height), 0},
//         {
//           VK_IMAGE_ASPECT_COLOR_BIT,
//           0, counter + startingLayer, 1
//         },
//         {0, 0, 0},
//         size
//       };
// 
//       copies.push_back(copy);
//       ++counter;
//     }
//   }
// }
// 
// void VulkanRender::loadTexture(const std::string &prefix, const ImageCreateInfo &info) {
//   std::string name = info.name.empty() ? info.path : info.name;
//   
//   auto itr = textureNames.find(name);
//   if (itr != textureNames.end()) throw std::runtime_error("Textures with name " + name + " already exist");
//   
//   // загружаем
//   int texWidth, texHeight, texChannels;
//   unsigned char* pixels = stbi_load((prefix + info.path).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//   VkDeviceSize imageSize = texWidth * texHeight * 4;
// 
//   // std::cout << "Image width: " << texWidth << " height: " << texHeight << "\n";
//   
//   CHECK_ERROR(!pixels, "failed to load image " + name + "!")
//   
//   yavf::ImageCreateInfo imageInfo{
//     0,
//     VK_IMAGE_TYPE_2D,
//     VK_FORMAT_R8G8B8A8_UNORM,
//     {uint32_t(texWidth), uint32_t(texHeight), 1},
//     1, // не помешало бы вынести в ImageCreateInfo
//     1,
//     VK_SAMPLE_COUNT_1_BIT, // по идее должно быть определенно в настройках
//     VK_IMAGE_TILING_LINEAR,
//     VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
//     VK_IMAGE_ASPECT_COLOR_BIT,
//     VMA_MEMORY_USAGE_CPU_ONLY
//   };
// 
//   yavf::Image* staging = device->createImage(imageInfo);
//   memcpy(staging->ptr(), pixels, imageSize);
//   
//   stbi_image_free(pixels);
//   
//   auto pair = info.count == 1 ? VkExtent2D{uint32_t(texWidth), uint32_t(texHeight)} : VkExtent2D{info.size.width, info.size.height};
//   auto precached = arrayIndices.find(pair);
//   
//   CHECK_ERROR(precached == arrayIndices.end(), "could not find precached image array")
//   
//   std::vector<Texture> textureVector;
//   yavf::TransferTask* task = device->allocateTransferTask();
//   uint32_t imagesCount = info.count;
//   const uint32_t index = precached->second;
//   VkExtent3D size = info.count == 1 ? VkExtent3D{uint32_t(texWidth), uint32_t(texHeight), 1} : VkExtent3D{info.size.width, info.size.height, 1};
//   for (uint32_t i = 0; i < arrays[index].size(); ++i) {
//     if (imagesCount == 0) break;
//     
//     uint32_t maxLayers = arrays[index][i].texture->param().layers;
//     if (arrays[index][i].currentLayer >= maxLayers) continue;
//     
//     uint32_t currentLayer = arrays[index][i].currentLayer;
//     
//     uint32_t newCount = maxLayers - currentLayer > imagesCount ? imagesCount : maxLayers - currentLayer;
//     
//     std::vector<VkImageCopy> copies;
//     copy(currentLayer, newCount, info.rows, info.columns, size, copies);
//     
// //     VkImageSubresourceRange range{
// //       VK_IMAGE_ASPECT_COLOR_BIT,
// //       0, 1, currentLayer, newCount
// //     };
//     
//     task->begin();
//     task->setBarrier(staging, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
//     task->setBarrier(arrays[index][i].texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//     task->copy(staging, arrays[index][i].texture, copies);
//     task->setBarrier(arrays[index][i].texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//     task->end();
//     
//     task->start();
//     
//     //std::cout << "Texture " << name << "\n";
//     for (uint32_t j = 0; j < newCount; ++j) {
//       textureVector.push_back({arrays[index][i].descriptorIndex, currentLayer + j, 0});
//       //std::cout << "Image array index: " << textureVector.back().imageArrayIndex << "\n";
//       //std::cout << "Image array layer: " << textureVector.back().imageArrayLayer << "\n";
//       //std::cout << "Texture sampler  : " << textureVector.back().samplerIndex << "\n";
//     }
//     
//     task->wait();
//     
//     arrays[index][i].currentLayer += newCount;
//     imagesCount -= newCount;
//   }
//   
//   CHECK_ERROR(imagesCount != 0, "bad precaching!")
//   
//   textureNames[name] = textureVector;
// }
// 
// void VulkanRender::createMipmaps() {
//   yavf::GraphicTask* task = device->allocateGraphicTask(1);
//   
//   task->begin();
//   for (const auto &image : arrayIndices) {
//     uint32_t mipmapCount = glm::floor(glm::log2(float(glm::max(image.first.width, image.first.height)))) + 1;
//     
//     const std::vector<TextureArrayData> &elem = arrays[image.second];
//     for (uint32_t i = 0; i < elem.size(); ++i) {
//       yavf::Image* img = elem[i].texture;
//       uint32_t layers = elem[i].currentLayer;
//       
//       VkImageSubresourceRange range{
//         VK_IMAGE_ASPECT_COLOR_BIT,
//         0, 1, 0, layers
//       };
//       task->setBarrier(img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range);
//       
//       for (uint32_t j = 1; j < mipmapCount; ++j) {
//         VkImageBlit blit{
//           {
//             VK_IMAGE_ASPECT_COLOR_BIT,
//             j-1, 0, layers
//           },
//           {
//             {0, 0, 0}, {int32_t(image.first.width >> (j - 1)), int32_t(image.first.height >> (j - 1)), 1}
//           },
//           {
//             VK_IMAGE_ASPECT_COLOR_BIT,
//             j, 0, layers
//           },
//           {
//             {0, 0, 0}, {int32_t(image.first.width >> j), int32_t(image.first.height >> j), 1}
//           }
//         };
//         
//         VkImageSubresourceRange mipSubRange{
//           VK_IMAGE_ASPECT_COLOR_BIT,
//           j, 1, 0, layers
//         };
//         
//         // попробовать с VK_FILTER_NEAREST?
//         
//         task->setBarrier(img->get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipSubRange);
//         task->copyBlit(img, img, blit, VK_FILTER_NEAREST);
//         task->setBarrier(img->get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mipSubRange);
//       }
//       
//       range.levelCount = mipmapCount;
//       task->setBarrier(img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range);
//     }
//   }
//   
//   task->end();
//   
//   task->start();
//   task->wait();
//   
//   device->deallocate(task);
// }
// 
// void VulkanRender::updateImageDescriptors() {
//   imagesCount = 0;
//   for (auto &array : arrays) {
//     for (size_t i = 0; i < array.size(); ++i) {
//       ++imagesCount;
//     }
//   }
//   
//   samplersCount = 1;
//   
//   {
//     yavf::DescriptorLayoutMaker dlm(device);
//     
//     if (imageLayout != VK_NULL_HANDLE) device->destroy(imageLayout);
//     if (samplerLayout != VK_NULL_HANDLE) device->destroy(samplerLayout);
//                               
//     imageLayout   = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, imagesCount).create("new_layout");
//     
//     samplerLayout = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, samplersCount).create("new_layout_sampler");
//   }
//   
//   // тут нужно бы удалить старые дескрипторы создать новые
//   auto pool = device->descriptorPool(FREE_DESCRIPTOR_POOL_NAME);
//   
//   if (images != VK_NULL_HANDLE) {
//     vkFreeDescriptorSets(device->handle(), pool, 1, &images);
//     images = VK_NULL_HANDLE;
//   }
//   
//   if (samplers != VK_NULL_HANDLE) {
//     vkFreeDescriptorSets(device->handle(), pool, 1, &samplers);
//     samplers = VK_NULL_HANDLE;
//   }
//   
//   {
//     yavf::DescriptorMaker dm(device);
//     
//     auto descs = dm.layout(imageLayout).layout(samplerLayout).create(pool);
//     
//     images = descs[0];
//     samplers = descs[0];
//   }
//   
//   auto sampler = device->sampler("default_sampler");
// 
//   yavf::DescriptorUpdater du(device);
// 
//   du.currentSet(images)
//       .begin(0, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
// 
//   for (auto &array : arrays) {
//     for (size_t i = 0; i < array.size(); ++i) {
//       du.image(array[i].texture);
//     }
//   }
//   
//   du.currentSet(samplers).begin(1, 0, VK_DESCRIPTOR_TYPE_SAMPLER).sampler(sampler.handle());
// 
//   du.update();
// }
// 
// void VulkanRender::clearImages() {
//   for (auto &array : arrays) {
//     for (size_t i = 0; i < array.size(); ++i) {
//       device->destroy(array[i].texture);
//     }
//   }
//   
//   arrays.clear();
// }
// 
// std::vector<Texture> VulkanRender::getTextures(const std::string &name) const {
//   auto itr = textureNames.find(name);
//   if (itr == textureNames.end()) return std::vector<Texture>();
// 
//   return itr->second;
// }
// 
// Texture VulkanRender::getTexture(const std::string &name, const uint32_t &index) const {
//   auto itr = textureNames.find(name);
//   if (itr == textureNames.end()) return Texture();
// 
//   return itr->second[index];
// }
// 
// uint32_t VulkanRender::getSampler(const std::string &name) const {
//   auto itr = samplerNames.find(name);
//   if (itr == samplerNames.end()) return 0;
//   
//   if (itr->second != 0) throw std::runtime_error("samplers is not ready yet");
// 
//   return itr->second;
// }

void VulkanRender::printStats() {
  
}
