#ifndef VULKAN_RENDER_H
#define VULKAN_RENDER_H

#include "Render.h"
#include "yavf.h"
#include "Optimizer.h"

#include "ImageResourceContainer.h"

// #include <imgui.h>
#include "nuklear_header.h"

#define CHECK_ERROR(res, str) if (res) {                         \
                                Global::console()->printE(str);  \
                                throw std::runtime_error(str);   \
                              }

class VulkanRender : public Render {
public:
  struct CreateInfo {
    yavf::Instance* instance;
    yavf::Device* device;
//     yavf::CombinedTask** task;
    
    size_t stageContainerSize;
  };
  VulkanRender(const CreateInfo &info);
  ~VulkanRender();
  
  void setContext(RenderContext* context);

  void addOptimizerToClear(Optimizer* opt);
  
  void updateCamera() override;
  
  void update(const uint64_t &time) override;
  void start() override;
  void wait() override;
  
  yavf::Instance* getInstance();
  
  yavf::Buffer* getCameraDataBuffer() const;
  yavf::Buffer* getMatrixesBuffer() const;
  
  void printStats() override;
private:
  yavf::Instance* instance;
  
  yavf::Device* device = nullptr;
  
  RenderContext* context;
  
  yavf::Buffer* uniformCameraData;
  yavf::Buffer* uniformMatrixes;
  
  yavf::Internal::Queue waitFence;
  
  uint32_t currentArrayElement = 0;

  std::vector<Optimizer*> optimizers;
};

#endif
