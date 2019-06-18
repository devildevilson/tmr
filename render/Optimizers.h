#ifndef OPTIMIZERS_H
#define OPTIMIZERS_H

#include "Render.h"
#include "Optimizer.h"
// #include "yavf.h"

#include "PhysicsTemporary.h"
#include "ArrayInterface.h"

class VulkanRender;

// че делать с текстурами? то есть понятное дело что должен быть какой то механизм 
// который нам обновляет все текстурки + вычисляет верный спрайт от положения
// нам даже в принципе для этого только трансформу поменять
// но прекол в том что он должен это делать в зависимости от состояния САМОГО ОБЪЕКТА
// в чем тут беда? только в одном понять что такое состояние объекта и научиться его передавать
// ко всему прочему это состояние нужно будет мне во многих других местах
// как с учетом всего этого организовать собственно все эти состояния и хранения данных по системам?
// видится мне что например для анимаций нужен будет каких то космических масштабов буфер
// struct InputBuffers {
//   ArrayInterface<Transform>* transforms;
//   ArrayInterface<simd::mat4>* matrices;
//   ArrayInterface<uint32_t>* rotationDatasCount;
//   ArrayInterface<RotationData>* rotationDatas;
//   ArrayInterface<glm::uvec4>* textures;
//   
//   ArrayInterface<BroadphasePair>* frustumPairs; // тут скорее всего это все же ненужно
// };

// с текстурами разобрался

// что делать с occlusion culling'ом?
// мне пригодится этот механизм если я хочу сделать карту
// карта понятное дело не вся должна быть доступна и должна открываться постепенно
// можно ли карту сделать по другому? сомневаюсь что выйдет достаточно быстро
// можно ли какнибудь встроить куллинг в основной конвейер? 
// для этого по идее мне просто нужно перекопировать данные в буферы в компьют шейдере
// в принципе это не очень сложно
// для куллинга не потребуются какие то уникальные данные, сверх тех что уже есть
// как насчет оптимизеров на гпу? тоже вообще ничего сложного
// все данные приходят в ArrayInterfac'ах, а значит вполне могут быть запихнуты в гпу

// как же это все устроить? значит все компут шейдера в рендер
// здесь скорее всего ничего
// что после? мне нужно собрать в массив все индексы и на основе них затем отрисовывать карту
// тут нужно что то другое придумать наверное

// мне возможно (скорее всего точно) потребуется указать как то что нужно нарисовать фуллбрайт текстуру
// потребоваться это может из-за наличия источника света рядом с (на, привязанного к, и проч) персонажем
// с другой стороны, если источник света мал, то это может выглядеть так себе
// но честно говоря если попытаться по другому это может вызвать боль
// добавить дополнительные данные в markAsVisible? меня это не убьет конечно
// тут нужно не фулбрайт, а просто отмечать какой пиксель относится к монстру
class MonsterOptimizer : public Optimizer {
public:
  struct InputBuffers {
    ArrayInterface<Transform>* transforms;
    ArrayInterface<simd::mat4>* matrices;
    ArrayInterface<uint32_t>* rotationDatasCount;
    ArrayInterface<RotationData>* rotationDatas;
    //ArrayInterface<glm::uvec4>* textures;
    //ArrayInterface<Texture>* textures;
    ArrayInterface<TextureData>* textures;
    
//     ArrayInterface<BroadphasePair>* frustumPairs; // тут скорее всего это все же ненужно
  };
  
  struct InstanceData {
    simd::mat4 mat;
//     uint32_t imageIndex;
//     uint32_t imageLayer;
//     uint32_t samplerIndex;
    TextureData textureData;
  };
  
  struct OutputBuffers {
    ArrayInterface<InstanceData>* instDatas;
  };
  
  struct GraphicsIndices {
    uint32_t transform;
    uint32_t matrix;
    uint32_t rotation;
    
    uint32_t texture; // ?
  };
  
  MonsterOptimizer();
  ~MonsterOptimizer();
  
//   uint32_t add(const GraphicsIndices &idx);
//   void remove(const uint32_t &index);
//   void markAsVisible(const uint32_t &index);
  void add(const GraphicsIndices &idx);
  
  void setInputBuffers(const InputBuffers &buffers);
  void setOutputBuffers(const OutputBuffers &buffers);
  
  uint32_t getInstanceCount() const;
  
  // еще было бы неплохо добавить сортировку по индексам используемых картинок
  void optimize() override;
  void clear() override;
  size_t size() const override;
private:
  ArrayInterface<Transform>* transforms = nullptr;
  ArrayInterface<simd::mat4>* matrices = nullptr;
  ArrayInterface<uint32_t>* rotationDatasCount = nullptr;
  ArrayInterface<RotationData>* rotationDatas = nullptr;
//   ArrayInterface<glm::uvec4>* textures = nullptr;
  ArrayInterface<TextureData>* textures = nullptr;
  
//   ArrayInterface<BroadphasePair>* frustumPairs = nullptr; 
  
  ArrayInterface<InstanceData>* instDatas = nullptr;
  
//   uint32_t objCount = 0;
//   uint32_t freeIndex = UINT32_MAX;
  std::vector<GraphicsIndices> objs;
//   std::vector<uint32_t> visible;
  
  // данные для оклюжен куллинга
  // для него еще нужно передать оффсет
  // и нужен массив с тем что видно а что нет
};

class GeometryOptimizer : public Optimizer {
public:
  struct InputBuffers {
    ArrayInterface<Transform>* transforms;
    ArrayInterface<simd::mat4>* matrices;
    ArrayInterface<uint32_t>* rotationDatasCount;
    ArrayInterface<RotationData>* rotationDatas;
    //ArrayInterface<glm::uvec4>* textures;
    //ArrayInterface<Texture>* textures;
    ArrayInterface<TextureData>* textures;
    
//     ArrayInterface<BroadphasePair>* frustumPairs; // тут скорее всего это все же ненужно
  };
  
  struct InstanceData {
//     glm::uvec4 textureIndices;
//     Texture texture;
    TextureData textureData;
  };
  
  struct OutputBuffers {
    ArrayInterface<uint32_t>* indices;
    ArrayInterface<InstanceData>* instanceDatas;
  };
  
  struct GraphicsIndices {
//     uint32_t transform;
    uint32_t matrix;
    uint32_t rotation;
    
    uint32_t texture;
    
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t faceIndex;
  };
  
  GeometryOptimizer();
  ~GeometryOptimizer();
  
//   uint32_t add(const GraphicsIndices &idx);
//   void remove(const uint32_t &index);
//   void markAsVisible(const uint32_t &index);
  void add(const GraphicsIndices &idx);
  
  void setInputBuffers(const InputBuffers &buffers);
  void setOutputBuffers(const OutputBuffers &buffers);
  
  uint32_t getIndicesCount() const;
  
  void optimize() override;
  void clear() override;
  size_t size() const override;
private:
  ArrayInterface<Transform>* transforms = nullptr;
  ArrayInterface<simd::mat4>* matrices = nullptr;
  ArrayInterface<uint32_t>* rotationDatasCount = nullptr;
  ArrayInterface<RotationData>* rotationDatas = nullptr;
  //ArrayInterface<Texture>* textures = nullptr;
  ArrayInterface<TextureData>* textures = nullptr;
  
//   ArrayInterface<BroadphasePair>* frustumPairs = nullptr; 
  
  ArrayInterface<uint32_t>* indices = nullptr;
  ArrayInterface<InstanceData>* instanceDatas = nullptr;
  
  uint32_t faceCount;
  uint32_t indicesCount;
//   uint32_t objCount = 0;
//   uint32_t freeIndex = UINT32_MAX;
  std::vector<GraphicsIndices> objs;
//   std::vector<uint32_t> visible;
};

class LightOptimizer : public Optimizer {
public:
  struct LightRegisterInfo {
    glm::vec3 pos;
    float radius;
    glm::vec3 color;
    float cutoff;
    
    uint32_t transformIndex;
  };
  
  // какие входные данные? позиция 100%, радиус
  // цвет?
  // мне нужен еще способ чуть чуть изменять оложение света относительно родительского объекта
  // в принципе тут поможет скорее всего только отдельный энтити на свет (хотя может и нет)
  // прикол в том что света у меня может быть очень много, весь свет нужно кулить по пирамиде
  // это значит что свету нужен физический объект, также свет должен как то реагировать на анимации
  // и в зависимости от этого у меня должна быть возможность нарисовать фуллбрайт текстуру
  // вообще возможно неплохой идеей будет как-то определить что за пиксель я сейчас обрабатываю
  // и для монстров засветлять спрайт вне зависимости от положения источника света
  // это по идее несложно сделать с помощью 8 бит альфа канала, мне нужно просто пометить засветлять или нет
  struct InputBuffers {
    ArrayInterface<Transform>* transforms;
    
  };
  
  // тут еще добавятся другие типы света
  struct OutputBuffers {
    ArrayInterface<LightData>* lights;
  };
  
  LightOptimizer();
  ~LightOptimizer();
  
  uint32_t add(const LightRegisterInfo &info);
  void remove(const uint32_t &index);
  void markAsVisible(const uint32_t &index);
  
  void setInputBuffers(const InputBuffers &buffers);
  void setOutputBuffers(const OutputBuffers &buffers);
  
  // тут тоже должен быть механизм похожий на markAsVisible
  // после прохождения проверки на фрустум, мы должны собирать весь видимый свет
  
  void optimize() override;
  void clear() override;
  size_t size() const override;
private:
  ArrayInterface<Transform>* transforms = nullptr;
  ArrayInterface<LightData>* lights = nullptr;
  
  uint32_t objCount;
  uint32_t freeIndex;
  std::vector<LightRegisterInfo> lightData;
  std::vector<uint32_t> visible;
};

// что со светом?
// входные данные света нужно переделать 200%
// что с тенями? shadow map городить? с другой стороны можно сделать так как это примерно было в думе
// то есть затенять определенные участки карты (тут может возникнуть проблема с поиском пути) (с другой стороны мне это нужно будет для очень немногих вещей)
// в общем для объектов нужны дополнительные данные

class MonsterDebugOptimizer : public Optimizer {
public:
  struct InputBuffers {
//     ArrayInterface<MonsterOptimizer::InstanceData>* instDatas;
    ArrayInterface<Transform>* transforms;
  };
  
  struct InstanceData {
    simd::mat4 mat;
    simd::vec4 color;
  };
  
  struct OutputBuffers {
    ArrayInterface<InstanceData>* instDatas;
  };
  
  MonsterDebugOptimizer();
  ~MonsterDebugOptimizer();
  
  //uint32_t add(const LightRegisterInfo &info);
  //void remove(const uint32_t &index);
  void setDebugColor(const uint32_t &transformIndex, const simd::vec4 &color); // const uint32_t &index, 
  
  void setInputBuffers(const InputBuffers &buffers);
  void setOutputBuffers(const OutputBuffers &buffers);
  
  void optimize() override;
  void clear() override;
  size_t size() const override;
private:
  //ArrayInterface<MonsterOptimizer::InstanceData>* optimizerInstDatas;
  ArrayInterface<Transform>* transforms;
  ArrayInterface<InstanceData>* instDatas;
  
  size_t count;
  // что еще? мы сюда должны передать количество объектов
  // с другой стороны мы сюда передаем цвет
};

class GeometryDebugOptimizer : public Optimizer {
public:
  struct InputBuffers {
    
    ArrayInterface<GeometryOptimizer::InstanceData>* instDatas;
  };
  
  struct InstanceData {
    simd::vec4 color;
  };
  
  struct OutputBuffers {
    ArrayInterface<InstanceData>* instDatas;
  };
  
  GeometryDebugOptimizer();
  ~GeometryDebugOptimizer();
  
  //uint32_t add(const LightRegisterInfo &info);
  //void remove(const uint32_t &index);
  void setDebugColor(const uint32_t &index, const simd::vec4 &color); // const uint32_t &index, 
  
  void setInputBuffers(const InputBuffers &buffers);
  void setOutputBuffers(const OutputBuffers &buffers);
  
  void optimize() override;
  void clear() override;
  size_t size() const override;
private:
  size_t count;
  
  ArrayInterface<GeometryOptimizer::InstanceData>* optimizerInstDatas;
  ArrayInterface<InstanceData>* instDatas;
};

#endif
