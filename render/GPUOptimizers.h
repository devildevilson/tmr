#ifndef GPU_OPTIMIZERS_H
#define GPU_OPTIMIZERS_H

#include "RenderStage.h"
#include "ArrayInterface.h"
#include "GPUArray.h"
#include "Utility.h"
#include "RenderStructures.h"
#include "PhysicsTemporary.h"
#include "Optimizer.h"

#include <mutex>

class MonsterGPUOptimizer : public RenderStage, public Optimizer {
public:
  struct InputBuffers {
    ArrayInterface<Transform>* transforms;
    ArrayInterface<simd::mat4>* matrices;
//     ArrayInterface<uint32_t>* rotationDatasCount;
//     ArrayInterface<RotationData>* rotationDatas;
    ArrayInterface<TextureData>* textures;
  };
  
  struct InstanceData {
    glm::mat4 mat;
    TextureData textureData;
    uint32_t dummy[3];
  };
  
  struct OutputBuffers {
    ArrayInterface<InstanceData>* instDatas;
  };
  
  struct GraphicsIndices {
    uint32_t transform;
    uint32_t matrix;
    uint32_t rotation;
    uint32_t texture;
  };
  
  struct CreateInfo {
    yavf::Device* device;
//     yavf::ComputeTask** tasks;
    yavf::Buffer* uniform;
  };
  MonsterGPUOptimizer(const CreateInfo &info);
  ~MonsterGPUOptimizer();
  
  void setInputBuffers(const InputBuffers &buffers);
  void setOutputBuffers(const OutputBuffers &buffers);
  
  void add(const GraphicsIndices &idx);
  
  uint32_t getInstanceCount() const;
  
  void begin() override;
  void doWork(RenderContext* context) override;
  void recreate(const uint32_t &width, const uint32_t &height) override;

  void optimize() override;
  void clear() override;
  size_t size() const override;
private:
//   ArrayInterface<Transform>* transforms;
//   ArrayInterface<simd::mat4>* matrices;
//   ArrayInterface<uint32_t>* rotationDatasCount;
//   ArrayInterface<RotationData>* rotationDatas;
//   ArrayInterface<TextureData>* textures;
//   ArrayInterface<InstanceData>* instDatas;
  
//   yavf::Buffer* transforms;
//   yavf::Buffer* matrices;
//   yavf::Buffer* textures;
//   yavf::Buffer* instDatas;
  ArrayInterface<Transform>* transforms;
  ArrayInterface<simd::mat4>* matrices;
  ArrayInterface<TextureData>* textures;
  ArrayInterface<InstanceData>* instDatasArray;
  
  GPUArray<GraphicsIndices> indices;
  
  yavf::Device* device;
  yavf::Pipeline pipe;
//   yavf::ComputeTask** tasks;
  
  // этот юниформ буффер нужен только для того чтобы умножить на матрицу вида
  // мне нужен второй юниформ буффер с количеством объектов
  yavf::Buffer* uniform;
  yavf::Buffer* objCount;
  
  std::mutex mutex;
};

class GeometryGPUOptimizer : public RenderStage, public Optimizer {
public:
  struct InputBuffers {
    ArrayInterface<Transform>* transforms;
    ArrayInterface<simd::mat4>* matrices;
    ArrayInterface<uint32_t>* rotationDatasCount;
    ArrayInterface<RotationData>* rotationDatas;
    ArrayInterface<TextureData>* textures;
  };
  
  struct InstanceData {
//     glm::mat4 mat; // добавится обязательно
    TextureData textureData;
  };
  
  struct OutputBuffers {
    ArrayInterface<uint32_t>* indices;
    ArrayInterface<InstanceData>* instanceDatas;
  };
  
  struct GraphicsIndices {
    uint32_t transform;
    uint32_t matrix;
    uint32_t rotation;
    uint32_t texture;
    
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t faceIndex;
    uint32_t dummy;
  };
  
  struct CreateInfo {
    yavf::Device* device;
//     yavf::ComputeTask** tasks;
    yavf::Buffer* uniform;
  };
  GeometryGPUOptimizer(const CreateInfo &info);
  ~GeometryGPUOptimizer();
  
  void setInputBuffers(const InputBuffers &buffers);
  void setOutputBuffers(const OutputBuffers &buffers);
  
  void add(const GraphicsIndices &idx);
  
  uint32_t getIndicesCount() const;
  
  void begin() override;
  void doWork(RenderContext* context) override;
  void recreate(const uint32_t &width, const uint32_t &height) override;

  void optimize() override;
  void clear() override;
  size_t size() const override;
private:
  ArrayInterface<Transform>* transforms;
  ArrayInterface<simd::mat4>* matrices;
  ArrayInterface<uint32_t>* rotationDatasCount;
  ArrayInterface<RotationData>* rotationDatas;
  ArrayInterface<TextureData>* textures;
  ArrayInterface<uint32_t>* indicesArray;
  ArrayInterface<InstanceData>* instanceDatas;
  
  GPUArray<GraphicsIndices> objs;
  
  uint32_t faceCount;
  uint32_t indicesCount;
  
  yavf::Device* device;
  yavf::Pipeline pipe;
//   yavf::ComputeTask** tasks;
  
  yavf::Buffer* uniform;
  yavf::Buffer* objCount;
  
  std::mutex mutex;
};

#endif
