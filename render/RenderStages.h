#ifndef RENDER_STAGES_H
#define RENDER_STAGES_H

#include "yavf.h"

#include "RenderStage.h"
#include "GPUArray.h"
#include "Optimizers.h"
#include "GPUOptimizers.h"
#include "DecalOptimizer.h"
#include "TypelessContainer.h"

#include "Deferred.h"

#include "ImageResourceContainer.h"

#include "RenderStructures.h"

// поддерживать такого монстра как Game становится все сложнее
// поэтому надо бы от него наконец избавиться и весь (почти) этот код пихнуть в main.cpp
// основная идея теперь это собрать все системы и просто последовательно их выполнять
// если какие-то системы могут быть выполнены параллельно их нужно пихнуть в отдельный класс
// ну в общем ситуация примерно такая же как и у рендера
// только в этом случае стейджи немного другие и называеются движками

class DynamicPipelineStage {
public:
  virtual ~DynamicPipelineStage() {}
  
  virtual void recreatePipelines(ImageResourceContainer* data) = 0;
};

class BeginTaskStage : public RenderStage {
public:
  BeginTaskStage();
  ~BeginTaskStage();
  
  void begin() override;
  void doWork(RenderContext* context) override;
  void recreate(const uint32_t &width, const uint32_t &height) override;
private:
//   yavf::TaskInterface** task;
  //yavf::CombinedTask** task;
};

class EndTaskStage : public RenderStage {
public:
  EndTaskStage();
  ~EndTaskStage();
  
  void begin() override;
  void doWork(RenderContext* context) override;
  void recreate(const uint32_t &width, const uint32_t &height) override;
private:
//   yavf::TaskInterface** task = nullptr;
};

class GBufferPart {
public:
  struct CreateInfo {
    yavf::Device* device;
    yavf::Buffer* uniformBuffer;
    
    yavf::RenderTarget* target;
//     yavf::GraphicTask** task;
    
//     yavf::Descriptor images;
//     yavf::Descriptor samplers;
  };
  
  virtual ~GBufferPart() {}
  
  virtual void create(const CreateInfo &info) = 0;
  
  virtual void recreatePipelines(ImageResourceContainer* data) = 0;
  
  virtual void begin() = 0;
  virtual bool doWork(RenderContext* context) = 0;
  
//   yavf::GraphicTask* getSecondaryTask() const { return localTask[0]; }
protected:
//   yavf::GraphicTask** localTask;
};

class GBufferStage : public RenderStage, public DynamicPipelineStage {
public:
  struct CreateInfo {
    yavf::Device* device;
    yavf::Buffer* uniformBuffer;
    //yavf::RenderTarget* target;
//     yavf::GraphicTask** task;
    
    uint32_t width;
    uint32_t height;
    
//     yavf::Descriptor images;
//     yavf::Descriptor samplers;
  };
  
//   struct MonsterData {
//     GPUArray<MonsterOptimizer::InstanceData> instanceData;
//     MonsterOptimizer* monsterOptimiser;
//   };
  
  GBufferStage(const size_t &containerSize, const CreateInfo &info);
  ~GBufferStage();
  
  // инициализация
  //void create(const CreateInfo &info);
  
  // здесь же скорее всего юниформ буфер и нужно заполнить
  // с другой стороны юниформ буфер это входные данные
  
  // где создать дескрипторы? как я уже говорил, скорее всего лучше 
  // создать отдельный класс, где и будет управление ресурсами
  // то есть там и создание дескрипторов будет, и переделывание их для нового уровня
  
  template<typename T, typename... Args>
  T* addPart(Args&&... args) {
    T* ptr = container.create<T>(std::forward<Args>(args)...);
    parallelParts.push_back(ptr);
    
    const GBufferPart::CreateInfo info{
      device,
      uniformBuffer,
      &target,
//       task
//       images,
//       samplers
    };
    parallelParts.back()->create(info);
    
    //std::cout << "parallelParts " << parallelParts.back() << "\n";
    
    return ptr;
  }
  
  void recreatePipelines(ImageResourceContainer* data) override;
  
  void begin() override;
  void doWork(RenderContext* context) override;
  void recreate(const uint32_t &width, const uint32_t &height) override;
  
  // отсюда нужно будет еще получить созданные пайплайны
  yavf::DescriptorSet* getDeferredRenderTargetDescriptor() const;
  yavf::DescriptorSetLayout getDeferredRenderTargetLayoutDescriptor() const;
  yavf::Image* getDepthBuffer() const;
private:  
  yavf::Device* device = nullptr;
  // создаем их здесь, в других местах они нам особо не нужны
  //yavf::Pipeline pipe;
  yavf::Buffer* uniformBuffer = nullptr;
//   yavf::Descriptor images;
//   yavf::Descriptor samplers;
  
  // также здесь будут располагаться указатели на оптимизеры
  // нужно ли их как нибудь приводить к общему виду? думаю что это невозможно
  // то есть для каждого оптимизера нам нужны разные входные выходные данные
  std::vector<GBufferPart*> parallelParts;
  TypelessContainer container;
  
  //GeometryOptimizer* geometryOptimiser = nullptr;
  
  // тут должен быть рендертаргет
  //yavf::RenderTarget* target = nullptr;
//   yavf::GraphicTask** task;
  
  Deferred target;
};

#define MONSTER_PIPELINE_LAYOUT_NAME "deferred_layout"
#define MONSTER_PIPELINE_NAME "deferred_monster_pipeline"

class MonsterGBufferStage : public GBufferPart {
public:
  // тут нужно создать пайплайн
  // + передать сюда буфер monsterDefault
  // + дескрипторы
  
  MonsterGBufferStage(MonsterGPUOptimizer* monsterOptimiser);
  ~MonsterGBufferStage();
  
  void create(const CreateInfo &info) override;
  
  void recreatePipelines(ImageResourceContainer* data) override;
  
  void begin() override;
  bool doWork(RenderContext* context) override;
  
  GPUArray<MonsterGPUOptimizer::InstanceData>* getInstanceData();
private:
  // тут у нас все данные для монстров
  yavf::Device* device = nullptr;
  
  GPUArray<MonsterGPUOptimizer::InstanceData> instanceData;
  MonsterGPUOptimizer* monsterOptimiser = nullptr;
  
  yavf::Pipeline pipe;
  yavf::Buffer* monsterDefault = nullptr;
  yavf::Buffer* uniformBuffer = nullptr;
  yavf::RenderTarget* target = nullptr;
  
//  yavf::DescriptorSet* images;
//  yavf::DescriptorSet* samplers;
  yavf::DescriptorSet* imagesSet;
};

#define GEOMETRY_PIPELINE_LAYOUT_NAME "deferred_layout2"
#define GEOMETRY_PIPELINE_NAME "deferred_geometry_pipeline"

class GeometryGBufferStage : public GBufferPart {
public:
  GeometryGBufferStage(yavf::Buffer* worldMapVertex, GeometryGPUOptimizer* opt);
  ~GeometryGBufferStage();
  
  void create(const CreateInfo &info) override;
  
  void recreatePipelines(ImageResourceContainer* data) override;
  
  void begin() override;
  bool doWork(RenderContext* context) override;
  
  GPUArray<uint32_t>* getIndicesArray();
  GPUArray<GeometryGPUOptimizer::InstanceData>* getInstanceData();
private:
  yavf::Device* device = nullptr;
  yavf::Pipeline pipe;
  
  GPUArray<uint32_t> indices;
  GPUArray<GeometryGPUOptimizer::InstanceData> instances;
  GeometryGPUOptimizer* opt = nullptr;
  
  yavf::Buffer* uniformBuffer = nullptr;
  yavf::RenderTarget* target = nullptr;
  
  yavf::Buffer* worldMapVertex = nullptr;
  
//  yavf::DescriptorSet* images;
//  yavf::DescriptorSet* samplers;
  yavf::DescriptorSet* imagesSet;
};

// нам по идее нужен еще рисовальщик частиц и декалей
// такой рисовальщик не очень отличается от других, но обладает некоторыми особенностями
// например, частицы скорее всего нужно будет рисовать в один присест с помощью геометрического шейдера 
// (хотя наверное можно скормить и 4 точки вершинному шейдеру)
// + ко всему у меня должны быть частицы которые обладают физикой (это скорее всего будут просто объекты в форме частицы) и которые нет
// 

// те частицы что мы будем рисовать для гбуфера должны быть непрозрачными
// а нужно ли их рисовать обязательно в гбуфере? так мы сможем легко кидать на них свет
// с другой стороны большое количество частиц вообще то прозрачны, 
// а прозрачные объекты мы в гбуфер добавить не можем, так как у нас будет неправильная глубина
// в кваке частицы непрозрачные, ну и там нет дефферед шейдинга
// мне бы еще хотелось сделать качественное затуманивание, для этого скорее всего придется добавить прозрачные частицы
// для частиц у меня будет два стейджа, 1 - считаем данные частиц в компут шейдере, 2 - рисуем их

// ЭТО КОСТЫЛЬ, ОТ НЕГО НУЖНО БУДЕТ ИЗБАВИТЬСЯ
// TODO: избавиться от костыля
//class ComputeParticleGBufferStage : public GBufferPart {
//public:
//  struct StageCreateInfo {
////     yavf::CombinedTask** task;
//
////     yavf::Buffer* uniformBuffer;
//    yavf::Buffer* particlesUniformBuffer;
//    yavf::Buffer* particles;
//    yavf::Buffer* particlesCount;
//    yavf::Buffer* matrixes;
//
//    yavf::DescriptorSet* gbuffer;
//    yavf::DescriptorSetLayout gbufferLayout;
//  };
//  ComputeParticleGBufferStage(const StageCreateInfo &info);
//  ~ComputeParticleGBufferStage();
//
//  void create(const CreateInfo &info) override;
//
//  void recreatePipelines(ImageResourceContainer* data) override;
//
//  void begin() override;
//  bool doWork(RenderContext* context) override;
//
//  // тут мы также должны получить индирект буфер
//private:
//  yavf::Device* device;
//  yavf::Pipeline particlesPipe;
//  yavf::Pipeline sortPipe;
//
//  yavf::CombinedTask** task;
//
//  yavf::Buffer* uniformBuffer;
//  yavf::Buffer* particlesUniformBuffer;
//  yavf::Buffer* particles;
//  yavf::Buffer* particlesCount;
//  yavf::Buffer* matrixes;
//
//  yavf::RenderTarget* target;
//
//  yavf::DescriptorSet* gbuffer;
//  yavf::DescriptorSetLayout gbufferLayout;
//
//  //yavf::DescriptorSet* particles;
//};
//
//// тут все стандартно
//class ParticleGBufferStage : public GBufferPart {
//public:
//  struct StageCreateInfo {
//    yavf::Buffer* particlesUniformBuffer;
//    yavf::Buffer* particles;
//    yavf::Buffer* particlesCount;
//  };
//  ParticleGBufferStage(const StageCreateInfo &info);
//  ~ParticleGBufferStage();
//
//  void create(const CreateInfo &info) override;
//
//  void recreatePipelines(ImageResourceContainer* data) override;
//
//  void begin() override;
//  bool doWork(RenderContext* context) override;
//private:
//  yavf::Device* device;
//  yavf::Pipeline pipe;
//
//  yavf::Buffer* uniformBuffer;
//  yavf::Buffer* particlesUniformBuffer;
//  yavf::Buffer* particles;
//  yavf::Buffer* particlesCount;
//  yavf::RenderTarget* target;
//
//  yavf::DescriptorSet* images;
//  yavf::DescriptorSet* samplers;
//};
//
//#define DECALS_PIPELINE_LAYOUT_NAME "decals_layout"
//#define DECALS_PIPELINE_NAME "decals_pipeline"
//
//class DecalsGBufferStage : public GBufferPart {
//public:
//  DecalsGBufferStage(DecalOptimizer* optimizer);
//  ~DecalsGBufferStage();
//
//  void create(const CreateInfo &info) override;
//
//  void recreatePipelines(ImageResourceContainer* data) override;
//
//  void begin() override;
//  bool doWork(RenderContext* context) override;
//private:
//  yavf::Device* device;
//  yavf::Pipeline pipe;
//
//  // оптимизер декалей? по идее декаль должна вычисляться лишь единожды
//  // здесь нужен просто доступ к системе скорее всего
//  DecalOptimizer* optimizer;
//
//  GPUArray<Vertex> vertices;
//  GPUArray<uint32_t> indices;
//  GPUArray<DecalOptimizer::InstanceData> instances;
//
//  yavf::Buffer* uniformBuffer;
//  yavf::RenderTarget* target;
//
//  yavf::DescriptorSet* images;
//  yavf::DescriptorSet* samplers;
//};

#define WORKGROUP_SIZE 16

class DefferedLightStage : public RenderStage {
public:
  struct CreateInfo {
    yavf::Device* device;
    yavf::Buffer* uniformBuffer;
    yavf::Buffer* matrixBuffer;
    //yavf::RenderTarget* target;
//     yavf::ComputeTask** task;
    
    LightOptimizer* optimizer;
    
    uint32_t width;
    uint32_t height;
    
    yavf::DescriptorSet* gbuffer;
    yavf::DescriptorSetLayout gbufferLayout;
  };
  
  DefferedLightStage(const CreateInfo &info);
  ~DefferedLightStage();
  
  void begin() override;
  void doWork(RenderContext* context) override;
  void recreate(const uint32_t &width, const uint32_t &height) override;
  
  LightOptimizer* getOptimizer();
  yavf::DescriptorSet* getOutputDescriptor() const;
private:
  yavf::Device* device = nullptr;
  yavf::Buffer* uniformBuffer = nullptr;
  yavf::Buffer* matrixBuffer = nullptr;
  yavf::ComputeTask** task;
  
  yavf::DescriptorSet* gbuffer;
  yavf::DescriptorSetLayout gbufferLayout;
  
  yavf::Pipeline pipe;
  
  yavf::Image* output = nullptr;
  
  GPUArray<LightData> lightArray;
  LightOptimizer* optimizer;
  
  // по идее это все что нужно
};

class ToneMappingStage : public RenderStage {
public:
  struct CreateInfo {
    yavf::Device* device;
//     yavf::ComputeTask** task;
    
    uint32_t width;
    uint32_t height;
    
    yavf::DescriptorSet* highResImage;
  };
  
  ToneMappingStage(const CreateInfo &info);
  ~ToneMappingStage();
  
  void begin() override;
  void doWork(RenderContext* context) override;
  void recreate(const uint32_t &width, const uint32_t &height) override;
  
  yavf::Image* getOutputImage() const;
private:
  yavf::Device* device = nullptr;
//   yavf::ComputeTask** task;
  
  yavf::DescriptorSet* highResImage; // мы получаем ее из предыдущих стейджев
  
  yavf::Pipeline pipe; // вычислительный шейдер
  
  yavf::Image* output = nullptr;
  
  // нам бы тут еще гамму как то выводить
};

// неплохо было бы еще эффект зернистости заиметь
class Window;

class CopyStage : public RenderStage {
public:
  struct CreateInfo {
    yavf::Device* device;
//     yavf::GraphicTask** task;
    
    yavf::Image* src;
    yavf::Image* depthSrc;
    
    uint32_t presentFamily;
    Window* window;
  };
  
  CopyStage(const CreateInfo &info);
  ~CopyStage();
  
  void begin() override;
  void doWork(RenderContext* context) override;
  void recreate(const uint32_t &width, const uint32_t &height) override;
private:
  yavf::Device* device = nullptr;
  yavf::GraphicTask** task;
  
  yavf::Image* src;
  yavf::Image* depthSrc;
  uint32_t presentFamily;
  Window* window;
  // нам нужен указатель на окно
};

// скорее всего тут будет пост рендер стейдж
// который будет включать в себя отрисовку гуи, дебага, транспарентов
// что то вроде гбуфер стейдж, только рисуем в один рендер таргет
// нужно будет подумать как совместить эти дела

class PostRenderPart {
public:
  struct CreateInfo {
    yavf::Device* device;
    
//     yavf::RenderTarget* target;
    Window* window;
//     yavf::GraphicTask** task;
  };
  
  virtual ~PostRenderPart() {}
  
  virtual void create(const CreateInfo &info) = 0;
  
  virtual void recreatePipelines(ImageResourceContainer* data) = 0;
  
  virtual void begin() = 0;
  virtual void doWork(RenderContext* context) = 0;
  
//   yavf::GraphicTask* getSecondaryTask() const { return localTask[0]; }
protected:
  //yavf::CombinedTask* localTask = nullptr;
//   yavf::GraphicTask** localTask;
};

class PostRenderStage : public RenderStage, public DynamicPipelineStage {
public:
  struct CreateInfo {
    yavf::Device* device;
//     yavf::GraphicTask** task;
    Window* window; // тут нужно что то придумать
  };
  
  PostRenderStage(const size_t &containerSize, const CreateInfo &info);
  ~PostRenderStage();
  
  template<typename T, typename... Args>
  T* addPart(Args&&... args) {
    T* ptr = container.create<T>(std::forward<Args>(args)...);
    parts.push_back(ptr);
    
    const PostRenderPart::CreateInfo info{
      device,
//       target,
      window,
//       task
    };
    
    parts.back()->create(info);
    
    return ptr;
  }
  
  void recreatePipelines(ImageResourceContainer* data) override;
  
  void begin() override;
  void doWork(RenderContext* context) override;
  void recreate(const uint32_t &width, const uint32_t &height) override;
protected:
  yavf::Device* device = nullptr;
  yavf::GraphicTask** task;
//   yavf::RenderTarget* target = nullptr;
  Window* window;
  
  std::vector<PostRenderPart*> parts;
  TypelessContainer container;
};

//struct nuklear_data;

class GuiStage : public PostRenderPart {
public:
//   struct CreateInfo {
//     yavf::Device* device;
//     yavf::CombinedTask* task;
//     yavf::RenderTarget* target;
//   };
  
  //GuiStage(const CreateInfo &info);
  //GuiStage(nuklear_data* data);
  GuiStage();
  ~GuiStage();
  
  void create(const CreateInfo &info) override;
  
  void recreatePipelines(ImageResourceContainer* data) override;
  
  void begin() override;
  void doWork(RenderContext* context) override;
  //void recreate(const uint32_t &width, const uint32_t &height) override;
private:
  yavf::Device* device = nullptr;
//   yavf::CombinedTask* task = nullptr;
  
//   yavf::RenderTarget* target = nullptr;
  Window* window;
  
  yavf::Pipeline pipe;
  yavf::Buffer* vertexGui;
  yavf::Buffer* indexGui;
  yavf::Buffer* matrix;
  yavf::DescriptorSet* image_set;
  
  // тут у нас по идее меняется рендер таргет
  // + тут нужен контекст гуи
  // 
//   nuklear_data* data;
};

class MonsterDebugStage : public PostRenderPart {
public:
  struct CreateInfo {
    MonsterDebugOptimizer* optimizer;
    MonsterOptimizer* monsterOptimiser;
    
    yavf::Buffer* uniformBuffer;
    GPUArray<MonsterOptimizer::InstanceData>* monData;
  };
  
  MonsterDebugStage(const CreateInfo &info);
  ~MonsterDebugStage();
  
  void create(const PostRenderPart::CreateInfo &info) override;
  
  void recreatePipelines(ImageResourceContainer* data) override;
  
  void begin() override;
  void doWork(RenderContext* context) override;
private:
  yavf::Device* device = nullptr;
//   yavf::CombinedTask* task = nullptr;
  
//   yavf::RenderTarget* target = nullptr;
  Window* window;
  
  yavf::Pipeline pipe;
  
  yavf::Buffer* monsterDebug;
  yavf::Buffer* uniformBuffer;
  
  // оптимизер
  // какие-то для него данные
  GPUArray<MonsterDebugOptimizer::InstanceData> instData;
  GPUArray<MonsterOptimizer::InstanceData>* monData;
  
  MonsterDebugOptimizer* optimizer;
  MonsterOptimizer* monsterOptimiser;
  
  uint32_t instanceCount;
};

class GeometryDebugStage : public PostRenderPart {
public:
  struct CreateInfo {
    GeometryDebugOptimizer* optimizer;
    GeometryOptimizer* geometryOptimiser;
    
    yavf::Buffer* uniformBuffer;
    
    GPUArray<uint32_t>* indices;
    yavf::Buffer* worldMapVertex;
  };
  
  GeometryDebugStage(const CreateInfo &info);
  ~GeometryDebugStage();
  
  void create(const PostRenderPart::CreateInfo &info) override;
  
  void recreatePipelines(ImageResourceContainer* data) override;
  
  void begin() override;
  void doWork(RenderContext* context) override;
private:
  yavf::Device* device = nullptr;
//   yavf::CombinedTask* task = nullptr;
  
//   yavf::RenderTarget* target = nullptr;
  Window* window;
  
  yavf::Pipeline pipe;
  
//   yavf::Buffer* monsterDebug = nullptr;
  yavf::Buffer* uniformBuffer;
  
  // оптимизер
  // какие-то для него данные
  GPUArray<GeometryDebugOptimizer::InstanceData> instData;
  GPUArray<uint32_t>* indices;
  yavf::Buffer* worldMapVertex;
  
  GeometryDebugOptimizer* optimizer;
  GeometryOptimizer* geometryOptimiser;
  
  uint32_t indexCount;
};

#endif
