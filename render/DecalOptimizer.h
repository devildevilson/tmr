#ifndef DECAL_OPTIMIZER_H
#define DECAL_OPTIMIZER_H

#include "Optimizer.h"
#include "ArrayInterface.h"
#include "RenderStructures.h"
#include "PhysicsTemporary.h"

#include <vector>

namespace yavf {
  class Buffer;
}

class DecalOptimizer : public Optimizer {
public:
  struct InputBuffers {
    ArrayInterface<Texture>* textures;
    ArrayInterface<Transform>* transforms;
    ArrayInterface<simd::mat4>* matrices;
    ArrayInterface<RotationData>* rotations;
  };
  
  struct InstanceData {
    simd::mat4 matrix;
    Texture textureData;
  };
  
  struct OutputBuffers {
    // может буфер?
//     yavf::Buffer* verts;
//     yavf::Buffer* idx;
    
    ArrayInterface<Vertex>* vertices;
    ArrayInterface<uint32_t>* indices;
    ArrayInterface<InstanceData>* instances; // инстансы, можно будет потом добавить сюда что нибудь еще
  };
  
  struct Data {
    uint32_t transformIndex;
    uint32_t textureIndex;
    uint32_t matrixIndex;
    uint32_t rotationIndex;
//     uint32_t faceIndex;
    
    std::vector<Vertex> vertices;
  };
  
  DecalOptimizer();
  ~DecalOptimizer();
  
  void setInputBuffers(const InputBuffers &buffers);
  void setOutputBuffers(const OutputBuffers &buffers);
  
  void add(const Data &data);
  
  uint32_t getIndicesCount() const;
  
  // здесь тогда считаем матрицы, копируем текстурку
  // мы можем здесь пересчитывать вершины, например для того чтобы не считать их позже
  // тут же мы можем копировать в буфер на гпу
  void optimize() override;
  void clear() override;
  size_t size() const override;
private:
  struct Indices {
    uint32_t transformIndex;
    uint32_t textureIndex;
    uint32_t matrixIndex;
    uint32_t rotationIndex;
    uint32_t faceIndex;
    
    size_t vertOffset;
    size_t vertCount;
  };
  
  ArrayInterface<Texture>* textures;
  ArrayInterface<Transform>* transforms;
  ArrayInterface<simd::mat4>* matrices;
  ArrayInterface<RotationData>* rotations;
  
  ArrayInterface<Vertex>* vertices;
  ArrayInterface<uint32_t>* indices;
  ArrayInterface<InstanceData>* instances;
  
  std::vector<Indices> datas;
  
  size_t vertSize;
  size_t idxSize;
  uint32_t maxFaces;
};

#endif
