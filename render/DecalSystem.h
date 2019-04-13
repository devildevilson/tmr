#ifndef DECAL_SYSTEM_H
#define DECAL_SYSTEM_H

#include "Engine.h"
#include "Utility.h"
#include "RenderStructures.h"
#include "ArrayInterface.h"

#include <vector>

// тут нам нужно указать позицию, размер и ориентацию декали
// с помощью этого мы вычислим пересечение декали с объектами
struct PendingDecalData {
  simd::vec4 pos;
  simd::mat4 orn;
  simd::vec4 scale;
  
  TextureData texture;
};

class DecalSystem : public Engine {
public:
  static void setContainer(Container<simd::mat4>* matrixContainer);
  
  DecalSystem();
  ~DecalSystem();
  
  void update(const uint64_t &time) override;
  
  void add(const PendingDecalData &data);
private:
  // pending decals
  std::vector<PendingDecalData> pendingDecals;
  
  // computed decals
  // вычисленные декали состоят из вершин и текстур, видимо придется еще заполнить вершинные индексы
  // это достаточно для декали, еще наверное нужна нормаль для того чтобы чуть чуть пододвинуть декаль
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<TextureData> textures;
  
  static Container<simd::mat4>* matrix;
};

#endif
