#include "VulkanRender.h"

#include "Render.h"

#define NK_IMPLEMENTATION
#include "nuklear_header.h"

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

VulkanRender::VulkanRender(const CreateInfo &info) : Render(info.stageContainerSize) {
  this->instance = info.instance;
  this->device = info.device;
//   this->task = info.task;

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
  (void)storage_layout;

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
  
  device->destroyDescriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);

  // когда удалится инстанс, он же и удалит все устройства
  // по идее
}

void VulkanRender::setContext(RenderContext* context) {
//   this->currentIndex = index;
  this->context = context;
}

void VulkanRender::updateCamera() {
  if (perspective) {
    matrices->matrixes.proj = matrices->persp;
  } else {
    matrices->matrixes.proj = matrices->ortho;
  }

  matrices->camera.viewproj = matrices->matrixes.proj * matrices->view;
  matrices->camera.view = matrices->view;
  matrices->matrixes.view = matrices->view;
  matrices->matrixes.invView = simd::inverse(matrices->view);
  matrices->matrixes.invProj = simd::inverse(matrices->matrixes.proj);
  matrices->matrixes.invViewProj = simd::inverse(matrices->camera.viewproj);

  memcpy(uniformCameraData->ptr(), &matrices->camera, sizeof(CameraData));
  memcpy(uniformMatrixes->ptr(), &matrices->matrixes, sizeof(MatBuffer));
}

void VulkanRender::update(const uint64_t &time) {
  (void)time;

  for(uint32_t i = 0; i < stages.size(); ++i) {
    stages[i]->begin();
  }

  for(uint32_t i = 0; i < stages.size(); ++i) {
    stages[i]->doWork(context);
  }
}

void VulkanRender::start() {
  yavf::TaskInterface* task = context->interface();
  const VkSubmitInfo &info = task->getSubmitInfo();
  const uint32_t &family = task->getFamily();

  waitFence = device->submit(family, 1, &info);
}

void VulkanRender::wait() {

  const VkResult res = vkWaitForFences(device->handle(), 1, &waitFence.fence, VK_TRUE, 1000000000);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("hlqhvlgvuvfgowqvuqopquwvuonvev");
  }
}

yavf::Instance* VulkanRender::getInstance() {
  return instance;
}

yavf::Buffer* VulkanRender::getCameraDataBuffer() const {
  return uniformCameraData;
}

yavf::Buffer* VulkanRender::getMatrixesBuffer() const {
  return uniformMatrixes;
}

void VulkanRender::printStats() {

}
