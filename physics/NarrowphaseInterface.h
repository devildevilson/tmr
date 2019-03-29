#ifndef NARROWPHASE_INTERFACE_H
#define NARROWPHASE_INTERFACE_H

#include "ArrayInterface.h"
#include "PhysicsUtils.h"

// что должно быть в нарров фазе?
// сейчас пока это 100% вычисление островов или батчей и сортировка пар по ним
// затем еще добавится вычисление констраинтов

// struct Constraint {
//   uint32_t first;
//   uint32_t second;

//   glm::vec4 normal;
// };

class Narrowphase {
public:
  struct InputBuffers {
    ArrayInterface<BroadphasePair>* dynPairs;
    ArrayInterface<BroadphasePair>* statPairs;
  };
  
  struct OutputBuffers {
    ArrayInterface<IslandData>* dynIslands;
    ArrayInterface<IslandData>* statIslands;
  };
  
  virtual ~Narrowphase() {}

  //virtual void setPairBuffer(ArrayInterface<BroadphasePair>* buffer, void* indirectPairCount = nullptr) = 0;
  virtual void setInputBuffers(const InputBuffers &inputs, void* indirectPairCount = nullptr) = 0;
  virtual void setOutputBuffers(const OutputBuffers &outputs, void* indirectIslandCount = nullptr) = 0;
  //virtual void setIndirectBuffer(void* buffer) = 0;
  // для вычисления констреинтов еще потребуются буферы с данными объекта и прочие

  virtual void updateBuffers(const uint32_t &lastPairCount, const uint32_t &lastStaticPairCount) = 0;

  virtual void calculateIslands() = 0; // либо острова либо батчи
  virtual void calculateBatches() = 0;
  virtual void checkIdenticalPairs() = 0;
  //virtual void sortPairs() = 0;
  virtual void postCalculation() = 0;

  // здесь возвращать будем буфер констраинтов (пока нет)
//   virtual ArrayInterface<IslandData>* getIslandDataBuffer() = 0;
//   virtual const ArrayInterface<IslandData>* getIslandDataBuffer() const = 0;

  virtual void* getIndirectIslandCount() { return nullptr; }

  virtual void printStats() = 0;
};

#endif /* !NARROWPHASE_INTERFACE_H */
