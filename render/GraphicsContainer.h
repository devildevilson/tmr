#ifndef GRAPHICS_CONTAINER_H
#define GRAPHICS_CONTAINER_H

#include "Engine.h"
#include "Core.h"
#include "RenderStage.h"

class VulkanRender;
class WindowInterface;
class Window;
class GameSystemContainer;

// нужно ли нам по разному обрабатывать окна с разной частотой обновления?
// или лучше все подвести под одну?
// ну, второе по идее проще, с другой стороны у нас половина картинки будет простаивать
// в общем нужно будет придумать систему которая будет рисовать кадр для окон только в случае
// их доступности, то есть у нас должен рендерится кадр только на половину фреймбуфера
// в общем пока что оставлю это

// я как то сделал костыль с тасками, теперь бы не помешало сделать нормально
// а нормально означает что мне нужно сделать некий контекст
// контекст должен возвращать определенный таск

struct GraphicsOutputStruct {
  VulkanRender* render;
  WindowInterface* windows;
  
  //yavf::Device* device;
  // один render
  // несколько окон window
};

class GraphicsContainer : public Engine, public RenderContext {
public:
  GraphicsContainer();
  ~GraphicsContainer();
  
  struct CreateInfo {
    size_t containerSize;
    GameSystemContainer* systemContainer;
  };
  void construct(CreateInfo &info);
  
  void update(const uint64_t &time) override;
  
  yavf::Instance* instance();
  yavf::Device* device() const;
  
  yavf::TaskInterface* interface() const override;
  yavf::CombinedTask* combined() const override;
  yavf::ComputeTask* compute() const override;
  yavf::GraphicTask* graphics() const override;
  yavf::TransferTask* transfer() const override;
  
  yavf::CombinedTask** tasks() const;
  yavf::ComputeTask** tasks1() const;
  yavf::GraphicTask** tasks2() const;
  yavf::TaskInterface** tasks3() const;
private:
  yavf::Instance inst;
  yavf::Device* dev;
  yavf::CombinedTask** task;
  yavf::ComputeTask** task1;
  yavf::GraphicTask** task2;
  yavf::TaskInterface** task3;
  //WindowInterface* windows;
  Window* windows;
  VulkanRender* render;
};

#endif
