#include "CPUSolver.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <glm/gtx/norm.hpp>
#include <cstring>

#include "HelperFunctions.h"

// float getAngle(const glm::vec4 &first, const glm::vec4 &second);
// void clipVelocity(const glm::vec4 &clipNormal, const float &bounce, glm::vec4 &vel);
// glm::vec4 transform(const glm::vec4 &p, const glm::vec4 &translation, const glm::mat4 &orientation);
// 
// bool isNormalAdequate(const ArrayInterface<glm::vec4>* verts, const uint32_t &offset, const uint32_t &normalIndex, const glm::vec4 &normal);

CPUSolver::CPUSolver(const float &threshold) {
  this->threshold = threshold;
  //lastVelocities.resize(4000);
}
CPUSolver::~CPUSolver() {}

// void CPUSolver::setBuffers(const SolverBuffers &buffers, void* indirectIslandCount, void* indirectPairCount) {
//   objects = buffers.objects;
//   verts = buffers.verts;
//   systems = buffers.systems;
//   datas = buffers.datas;
//   transforms = buffers.transforms;
//   staticPhysDatas = buffers.staticPhysDatas;
//   rotationDatas = buffers.rotationDatas;
//   
//   pairs = buffers.pairs;
//   staticPairs = buffers.staticPairs;
//   
//   islands = buffers.islands;
//   staticIslands = buffers.staticIslands;
//   
//   indicies = buffers.indicies;
//   velocities = buffers.velocities;
//   gravity = buffers.gravity;
// 
//   rays = buffers.rays;
//   rayPairs = buffers.rayPairs;
// }

void CPUSolver::setInputBuffers(const InputBuffers &buffers, void* indirectIslandCount, void* indirectPairCount) {
  objects = buffers.objects;
  verts = buffers.verts;
  systems = buffers.systems;
  datas = buffers.datas;
  transforms = buffers.transforms;
  staticPhysDatas = buffers.staticPhysDatas;
  rotationDatas = buffers.rotationDatas;
  
  pairs = buffers.pairs;
  staticPairs = buffers.staticPairs;
  
  islands = buffers.islands;
  staticIslands = buffers.staticIslands;
  
  indicies = buffers.indicies;
  velocities = buffers.velocities;
  gravity = buffers.gravity;

  rays = buffers.rays;
  rayPairs = buffers.rayPairs;
}

void CPUSolver::setOutputBuffers(const OutputBuffers &buffers) {
  overlappingData = buffers.overlappingData;
  dataIndices = buffers.dataIndices;

  raysData = buffers.raysData;
  raysIndices = buffers.raysIndices;

  triggerIndices = buffers.triggerIndices;
}

void CPUSolver::updateBuffers(const uint32_t &pairsCount, const uint32_t &rayPairs) {
  if (overlappingData->size() < pairsCount * 1.5f) overlappingData->resize(pairsCount * 1.5f);
  if (raysData->size() < rayPairs) raysData->resize(rayPairs);
  //if (tmpBuffer.size() < pairsCount+1) tmpBuffer.resize(pairsCount+1);
}

void CPUSolver::calculateData() {
  throw std::runtime_error("Not implemented yet");
}

void CPUSolver::calculateRayData() {
  throw std::runtime_error("Not implemented yet");
}

void CPUSolver::solve() {
  const uint32_t iterationCount = 3;
  
  //memset(lastVelocities.data(), 0, lastVelocities.size() * sizeof(lastVelocities[0]));
  
  for (uint32_t iteration = 0; iteration < iterationCount; ++iteration) {
    // тут у нас перевычисление позиции остается таким же
    
    const float koef = 1.0f / float(iterationCount);
    const uint32_t objCount = indicies->at(0);
    for (uint32_t j = 0; j < objCount; ++j) {
      const uint32_t objIndex = j;

      const uint32_t staticPhysDataIndex = objects->at(indicies->at(objIndex+1)).staticPhysicDataIndex;
      const uint32_t physDataIndex = staticPhysDatas->at(staticPhysDataIndex).physDataIndex;

      if (physDataIndex == UINT32_MAX) continue;

      const uint32_t transformIndex = datas->at(physDataIndex).transformIndex;
      const float dt = MCS_TO_SEC(gravity->data()->time);

      if (iteration == 0) {
        //datas->at(physDataIndex).onGroundBits = datas->at(physDataIndex).onGroundBits | ((datas->at(physDataIndex).onGroundBits & 0x1) << 1);
        datas->at(physDataIndex).onGroundBits <<= 1;
        datas->at(physDataIndex).onGroundBits &= 0x2;
        datas->at(physDataIndex).onGroundBits &= 0x3;
        transforms->at(transformIndex).pos = datas->at(physDataIndex).oldPos;
      }

      // здесь мы перевычисляем позиции объектов
      // нам нужна общая скорость (тип скорость объекта + скорость земли)
      // по всей видимости тут нужен будет еще один буфер, где мы будем хранить скорость вычисленную еще в первом шаге
      // скорости менять от столкновений нам придется видимо две
      // куда поз сохранить? сразу в трансформы? или лучше в какой дополнительный буфер?
      transforms->at(transformIndex).pos += koef * velocities->at(physDataIndex) * dt;
      //velocities->at(physDataIndex) -= lastVelocities[physDataIndex];
    }
    
    struct IslandAdditionalData {
      glm::uvec4 octreeDepth;
      glm::uvec4 octreeLevels[10];
      glm::uvec4 islandCount;
    };
    
    if (staticPairs->at(0).firstIndex > 0) {
      const IslandAdditionalData* data = staticIslands->structure_from_begin<IslandAdditionalData>();
      const IslandData* islandsPtr = staticIslands->data_from<IslandAdditionalData>();
      
      //assert(data->islandCount.x == 0);
      
      // и начинаем вычисление пар
      // сначала у нас идет параллельное вычисление статиков
      // для статических пар тоже нужны острова так то (по индексам первых объектов)
      //std::cout << "static pair count " << staticPairs->at(0).firstIndex << '\n';
      
      //std::cout << "island count " << data->islandCount.x << "\n";
      glm::vec4 normal;
      
      for (uint32_t i = 0; i < data->islandCount.x; ++i) {
        const uint32_t islandIndex = i;
        const IslandData &island = islandsPtr[islandIndex];
        
        // здесь последовательно вычисляем пары в острове
        for (uint32_t j = 0; j < island.size; ++j) {
          const uint32_t pairIndex = island.offset + j + 1;
          const BroadphasePair &pair = staticPairs->at(pairIndex);
          
          const uint32_t objIndex2 = pair.secondIndex;
          
          const uint32_t obj2Type = objects->at(objIndex2).objType.getObjType();
          if (obj2Type == POLYGON_TYPE && objects->at(objIndex2).vertexCount + 1 == objects->at(objIndex2).faceCount) {
            const uint32_t faceOffset = objects->at(objIndex2).vertexOffset + objects->at(objIndex2).vertexCount + 1;
            normal = verts->at(faceOffset);
            
            const float planeAngle = getAngle(normal, -gravity->data()->gravityNormal);
            
            if (planeAngle > PI_Q) continue;
          } else continue;
          
          computePairWithGround(pair, normal);
          
          staticPairs->at(pairIndex).islandIndex = UINT32_MAX;
        }
        
        for (uint32_t j = 0; j < island.size; ++j) {
          const uint32_t pairIndex = island.offset + j + 1;
          const BroadphasePair &pair = staticPairs->at(pairIndex);
          
          //if (pair.secondIndex == 5100) throw std::runtime_error("5100");
          
          if (pair.islandIndex == UINT32_MAX) continue;
          
          computePair(pair);
        }
      }
    }
    
    // а затем вычисление динамиков, которых мы разбили по уровням октодерева
    if (pairs->at(0).firstIndex > 0) {
      const IslandAdditionalData* data = islands->structure_from_begin<IslandAdditionalData>();
      const IslandData* islandsPtr = islands->data_from<IslandAdditionalData>();
      
      for (uint32_t depth = 0; depth < data->octreeDepth.x; ++depth) {
        const glm::uvec4 &octreeLevel = data->octreeLevels[depth];
        
        // затем параллелим по островам
        for (uint32_t i = 0; i < octreeLevel.z; ++i) {
          const IslandData &island = islandsPtr[octreeLevel.y + i];
          
          // и здесь последовательно вычисляем каждую пару внутри острова
          for (uint32_t j = 0; j < island.size; ++j) {
            const uint32_t pairIndex = island.offset + j + 1;
            const BroadphasePair &pair = pairs->at(pairIndex);
            
            computePair(pair);
          }
        }
      }
    }
    
  }
  
//   const float koef = 1.0f / float(iterationCount);
//   const uint32_t objCount = indicies->at(0);
//   for (uint32_t j = 0; j < objCount; ++j) {
//     const uint32_t objIndex = j;
// 
//     const uint32_t staticPhysDataIndex = objects->at(indicies->at(objIndex+1)).staticPhysicDataIndex;
//     const uint32_t physDataIndex = staticPhysDatas->at(staticPhysDataIndex).physDataIndex;
// 
//     if (physDataIndex == UINT32_MAX) continue;
// 
//     const uint32_t transformIndex = datas->at(physDataIndex).transformIndex;
//     const float dt = MCS_TO_SEC(gravity->data()->time);
// 
//     // здесь мы перевычисляем позиции объектов
//     // нам нужна общая скорость (тип скорость объекта + скорость земли)
//     // по всей видимости тут нужен будет еще один буфер, где мы будем хранить скорость вычисленную еще в первом шаге
//     // скорости менять от столкновений нам придется видимо две
//     // куда поз сохранить? сразу в трансформы? или лучше в какой дополнительный буфер?
//     //transforms->at(transformIndex).pos += koef * velocities->at(physDataIndex) * dt;
//     transforms->at(transformIndex).pos += lastVelocities[physDataIndex] * dt;
//   }
}

// ArrayInterface<OverlappingData>* CPUSolver::getOverlappingData() {
//   return &overlappingData;
// }
// 
// const ArrayInterface<OverlappingData>* CPUSolver::getOverlappingData() const {
//   return &overlappingData;
// }
// 
// ArrayInterface<DataIndices>* CPUSolver::getDataIndices() {
//   throw std::runtime_error("Not implemented yet");
// }
// 
// const ArrayInterface<DataIndices>* CPUSolver::getDataIndices() const {
//   throw std::runtime_error("Not implemented yet");
// }
// 
// ArrayInterface<OverlappingData>* CPUSolver::getRayIntersectData() {
//   return &rayData;
// }
// 
// const ArrayInterface<OverlappingData>* CPUSolver::getRayIntersectData() const {
//   return &rayData;
// }
// 
// ArrayInterface<DataIndices>* CPUSolver::getRayIndices() {
//   throw std::runtime_error("Not implemented yet");
// }
// 
// const ArrayInterface<DataIndices>* CPUSolver::getRayIndices() const {
//   throw std::runtime_error("Not implemented yet");
// }

void CPUSolver::printStats() {
  std::cout << "CPU solver" << '\n';
}

glm::vec4 CPUSolver::getNormalCloseToGravity(const glm::mat4 &orn, const glm::vec4 &gravityNorm) const {
  float maxVal = glm::dot(orn[0], gravityNorm);
  uint32_t index = 0;

  for (uint32_t i = 1; i < 6; ++i) {
    if (i < 3) {
      const float tmp = glm::dot(orn[i], gravityNorm);
      maxVal = glm::max(maxVal, tmp);
      index = maxVal == tmp ? i : index;
    } else {
      const float tmp = glm::dot(-orn[i%3], gravityNorm);
      maxVal = glm::max(maxVal, tmp);
      index = maxVal == tmp ? i : index;
    }
  }

  return index < 3 ? orn[index] : -orn[index%3];
}

glm::vec4 CPUSolver::getNormalCloseToGravity(const glm::mat4 &orn, const uint32_t &offset, const uint32_t &facesCount, const glm::vec4 &gravityNorm, uint32_t &retIndex) const {
  const glm::vec4 &first = verts->at(offset);
  float maxVal = glm::dot(orn * glm::vec4(first.x, first.y, first.z, 0.0f), gravityNorm);
  uint32_t index = offset;

  for (uint32_t i = 1; i < facesCount; ++i) {
    const glm::vec4 &vert = verts->at(offset+i);
    const float tmp = glm::dot(orn * glm::vec4(vert.x, vert.y, vert.z, 0.0f), gravityNorm);
    maxVal = glm::max(maxVal, tmp);
    index = maxVal == tmp ? offset+i : index;
  }
  
  retIndex = index;
  const glm::vec4 &last = verts->at(index);
  return orn * glm::vec4(last.x, last.y, last.z, 0.0f);
}

// glm::vec4 getVertex(const glm::vec4 &pos, const glm::vec4 &ext, const glm::mat4 &orn, const uint32_t &index) {
//   glm::vec4 p = pos;
//   p = (index & 1) == 1 ? p + orn[0]*ext.x : p - orn[0]*ext.x;
//   p = (index & 2) == 2 ? p - orn[1]*ext.y : p + orn[1]*ext.y;
//   p = (index & 4) == 4 ? p + orn[2]*ext.z : p - orn[2]*ext.z;
//   p.w = 1.0f;
// 
//   return p;
// }

void CPUSolver::project(const glm::vec4 &axis, const uint32_t &offset, const uint32_t &size, const glm::vec4 &pos, const glm::mat4 &invOrn, float &minRet, float &maxRet) const {
  const glm::vec4 &localAxis = invOrn * axis; // по идее это должно работать
  const float offsetF = glm::dot(pos, axis);
  const glm::vec4 &first = verts->at(offset);
  minRet = maxRet = glm::dot(first, localAxis);

  for (uint32_t i = 1; i < size; ++i) {
    const glm::vec4 &vert = verts->at(offset + i);
    const float d = glm::dot(vert, localAxis);

    minRet = glm::min(minRet, d);
    maxRet = glm::max(maxRet, d);
  }

  if (minRet > maxRet) {
    const float tmp = minRet;
    minRet = maxRet;
    maxRet = tmp;
  }

  minRet += offsetF;
  maxRet += offsetF;
}

void CPUSolver::project(const glm::vec4 &axis, const glm::vec4 &pos, const glm::vec4 &ext, const glm::mat4 &orn, float &minRet, float &maxRet) const {
  const glm::vec4 &localAxis = /*orn **/ axis;
  //const float offsetF = dot(pos, axis);
  const glm::vec4 &first = getVertex(pos, ext, orn, 0);
  minRet = maxRet = glm::dot(first, localAxis);

  for (uint32_t i = 1; i < 8; ++i) {
    const glm::vec4 &vert = getVertex(pos, ext, orn, i);
    const float d = glm::dot(vert, localAxis);

    minRet = glm::min(minRet, d);
    maxRet = glm::max(maxRet, d);
  }

  // if (minRet > maxRet) {
  //   const float tmp = minRet;
  //   minRet = maxRet;
  //   maxRet = tmp;
  // }
  //
  // minRet += offsetF;
  // maxRet += offsetF;
}

void CPUSolver::project(const glm::vec4 &axis, const glm::vec4 &pos, const float &radius, float &minRet, float &maxRet) const {
  minRet = maxRet = glm::dot(pos, axis);

  minRet -= radius;
  maxRet += radius;
}

// bool overlap(const float &min1, const float &max1, const float &min2, const float &max2, const glm::vec4 &axis, glm::vec4 &mtv, float &dist) {
//   const float test1 = min1 - max2;
//   const float test2 = min2 - max1;
// 
//   if (test1 > 0.0f || test2 > 0.0f) return false;
// 
//   const float d = glm::min(glm::abs(test1), glm::abs(test2));
//   const float half1 = (min1 + max1);// / 2.0f;
//   const float half2 = (min2 + max2);// / 2.0f;
//   if (d < dist) {
//     // std::cout << "half1 " << half1 << '\n';
//     // std::cout << "half2 " << half2 << '\n';
//     //mtv = half1 < half2 ? axis : -axis;
//     mtv = axis;
//     // if (half1 < half2) {
//     //   std::cout << "half1 < half2" << '\n';
//     //   mtv = axis;
//     // } else {
//     //   std::cout << "half1 >= half2" << '\n';
//     //   mtv = -axis;
//     // }
//     dist = d;
//   }
// 
//   return true; //test1 < 0.0f && test2 < 0.0f;
// }

// glm::vec4 getBoxBoxFace(const glm::mat4 &orn1, const glm::mat4 &orn2, const uint32_t &index) {
//   // СЧЕТ НАЧИНАЕТСЯ С НУЛЯ (0!!!) СЛЕДОВАТЕЛЬНО 3-ИЙ ВЕКТОР ИМЕЕТ ИНДЕКС 2
//   // А ЗНАЧИТ ВСЕ ЧТО БОЛЬШЕ 2 (ДВУХ) (А НЕ ТРЕХ) УЖЕ ДРУГАЯ МАТРИЦА
//   return index > 2 ? orn1[index%3] : orn2[index];
// }

bool CPUSolver::BoxBoxSAT(const Object &first,  const uint32_t &transFirst,
                          const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const {
  // тут по идее нужен еще вариант когда у нас в этих индексах стоит 0xFFFFFFFF
  // но думаю этот вариант добавлю позже (нужен он вообще? для него скорее всего придется делать более нормальный солвер)
  const glm::mat4 &sys1 = systems->at(first.coordinateSystemIndex);
  //const glm::mat4 sys1 = systems[0];
  const glm::mat4 &sys2 = systems->at(second.coordinateSystemIndex);
  //const glm::mat4 sys2 = systems[0];

  float minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;

  const glm::vec4 &firstPos  = transforms->at(transFirst).pos;
  const glm::vec4 &secondPos = transforms->at(transSecond).pos;

  const glm::vec4 &firstExt  = verts->at(first.vertexOffset);
  const glm::vec4 &secondExt = verts->at(second.vertexOffset);

  const glm::vec4 &delta = firstPos - secondPos;

  bool col = true;
  for (uint32_t i = 0; i < 3+3; ++i) {
    const uint32_t index = i;
    const glm::vec4 &axis = getBoxBoxFace(sys1, sys2, index);

    project(axis, firstPos,  firstExt,  sys1,  minFirst,  maxFirst);
    project(axis, secondPos, secondExt, sys2, minSecond, maxSecond);

    col = col && overlap(0.0f, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist);
  }

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return col;
}

// glm::vec4 getBoxSphereFace(const glm::mat4 &orn, const glm::vec4 &pos1, const glm::vec4 &pos2, const uint32_t &index) {
//   return index > 0 ? orn[index%3] : glm::normalize(pos2 - pos1);
// }

bool CPUSolver::BoxSphereSAT(const Object &first,  const uint32_t &transFirst,
                             const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const {
  const glm::mat4 &sys = systems->at(first.coordinateSystemIndex);
  float minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;

  const glm::vec4 &firstPos  = transforms->at(transFirst).pos;
  const glm::vec4 &secondPos = glm::vec4(transforms->at(transSecond).pos.x, transforms->at(transSecond).pos.y, transforms->at(transSecond).pos.z, 1.0f);

  const glm::vec4 &firstExt  = verts->at(first.vertexOffset);
  const float radius = transforms->at(transSecond).pos.w;
  const glm::vec4 &axis = glm::normalize(secondPos - firstPos);

  const glm::vec4 &delta = firstPos - secondPos;

  bool col = true;
  for (uint32_t i = 0; i < 3+1; ++i) {
    const uint32_t index = i;
    const glm::vec4 &axis = getBoxSphereFace(sys, firstPos, secondPos, index);

    project(axis, firstPos,  firstExt,  sys,  minFirst,  maxFirst);
    project(axis, secondPos, radius, minSecond, maxSecond);

    col = col && overlap(0.0f, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist);
  }

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return col;
}

// glm::vec4 getBoxPolyFace(const ArrayInterface<glm::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const glm::mat4 &polyOrn, const glm::mat4 &orn, const uint32_t &index) {
//   const glm::vec4 &polyNormal = verts->at(face+(index%faceSize));
//   return index > 2 ? glm::vec4(polyNormal.x, polyNormal.y, polyNormal.z, 0.0f) : orn[index];
// }

bool CPUSolver::BoxPolySAT(const Object &first,  const uint32_t &transFirst,
                           const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const {
  float minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;

  // first = box always

  const glm::mat4 &sys = systems->at(first.coordinateSystemIndex);

  const glm::vec4 &firstPos  = transforms->at(transFirst).pos;
  const glm::vec4 &secondPos = transSecond != UINT32_MAX ? transforms->at(transSecond).pos : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

  const glm::vec4 &firstExt  = verts->at(first.vertexOffset);

  const uint32_t vert     = second.vertexOffset;
  const uint32_t vertSize = second.vertexCount;
  const uint32_t face     = vert + vertSize + 1;
  const uint32_t faceSize = second.faceCount;

  const glm::mat4 &orn = second.rotationDataIndex == UINT32_MAX ?
                          systems->at(second.coordinateSystemIndex) : 
                          rotationDatas->at(second.rotationDataIndex).matrix * systems->at(second.coordinateSystemIndex);
  const glm::mat4 &invOrn = glm::inverse(orn);

  glm::vec4 &localCenter = verts->at(vert + vertSize);
  localCenter = transform(localCenter, secondPos, orn);

  //std::cout << "Face size " << faceSize << '\n';
  assert(faceSize < 10);

  const glm::vec4 &delta = firstPos - localCenter;

//   std::cout << "firstPos    x: " << firstPos.x << " y: " << firstPos.y << " z: " << firstPos.z << "\n";
//   std::cout << "localCenter x: " << localCenter.x << " y: " << localCenter.y << " z: " << localCenter.z << "\n";
//   std::cout << "delta       x: " << delta.x << " y: " << delta.y << " z: " << delta.z << "\n";
// 
//   std::cout << '\n';

  bool col = true;
  for (uint32_t i = 0; i < faceSize+3; ++i) {
    const uint32_t index = i;
    const glm::vec4 &axis = getBoxPolyFace(verts, face, faceSize, orn, sys, index);

    project(axis, firstPos,  firstExt,  sys,  minFirst,  maxFirst);
    project(axis, vert, vertSize, secondPos, invOrn,  minSecond, maxSecond);

//     std::cout << '\n';
//     std::cout << "Face " << i << '\n';
//     std::cout << "axis x: " << axis.x << " y: " << axis.y << " z: " << axis.z << " w: " << axis.w << (index > 2 ? " Poly axis" : " Box axis") << '\n';
//     std::cout << "min1 " << minFirst << " max1 " << maxFirst << '\n';
//     std::cout << "min2 " << minSecond << " max2 " << maxSecond << '\n';

//     const float test1 = minFirst - maxSecond;
//     const float test2 = minSecond - maxFirst;
//     const float d = glm::min(glm::abs(test1), glm::abs(test2));

//     std::cout << "dist " << dist << '\n';
//     std::cout << "d    " << d << " " << (dist > d) << '\n';

    col = col && overlap(0.0f, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist);
  }

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return col;
}

bool CPUSolver::SphereSphereSAT(const Object &first,  const uint32_t &transFirst,
                                const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const {
  float minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;

  const glm::vec4 &tmpPos1 = transforms->at(transFirst).pos;
  const glm::vec4 &firstPos = glm::vec4(tmpPos1.x, tmpPos1.y, tmpPos1.z, 1.0f);
  const float firstRadius = transforms->at(transFirst).pos.w;

  const glm::vec4 &tmpPos2 = transforms->at(transFirst).pos;
  const glm::vec4 &secondPos = glm::vec4(tmpPos2.x, tmpPos2.y, tmpPos2.z, 1.0f);
  const float secondRadius = transforms->at(transSecond).pos.w;

  const glm::vec4 &axis = glm::normalize(secondPos - firstPos);

  const glm::vec4 &delta = firstPos - secondPos;

  project(axis, firstPos, firstRadius, minSecond, maxSecond);
  project(axis, secondPos, secondRadius, minSecond, maxSecond);

  if (!overlap(0.0f, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist)) return false;

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return true;
}

// glm::vec4 getPolySphereFace(const ArrayInterface<glm::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const glm::mat4 &ornFirst, const glm::vec4 &pos1, const glm::vec4 &pos2, const uint32_t &index) {
//   const glm::vec4 &polyNormal = verts->at(face+(index%faceSize));
//   return index > 0 ? ornFirst * glm::vec4(polyNormal.x, polyNormal.y, polyNormal.z, 0.0f) : glm::normalize(pos2 - pos1);
// }

bool CPUSolver::PolySphereSAT(const Object &first,  const uint32_t &transFirst,
                              const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const {
  float minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;

  const glm::vec4 &firstPos = transFirst != UINT32_MAX ? transforms->at(transFirst).pos : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  const glm::vec4 &tmpPos = transforms->at(transSecond).pos;
  const glm::vec4 &secondPos = glm::vec4(tmpPos.x, tmpPos.y, tmpPos.z, 1.0f);

  const float secondRadius = transforms->at(transSecond).pos.w;

  const uint32_t vert     = first.vertexOffset;
  const uint32_t vertSize = first.vertexCount;
  const uint32_t face     = vert + vertSize + 1;
  const uint32_t faceSize = first.faceCount;

  const glm::mat4 &ornFirst = first.rotationDataIndex == UINT32_MAX ?
                                systems->at(first.coordinateSystemIndex) : 
                                rotationDatas->at(first.rotationDataIndex).matrix * systems->at(first.coordinateSystemIndex);
  const glm::mat4 &invOrn = glm::inverse(ornFirst);

  glm::vec4 &localCenter = verts->at(vert + vertSize);
  localCenter = transform(localCenter, firstPos, ornFirst);
  const glm::vec4 &delta = localCenter - secondPos;

  bool col = true;
  for (uint32_t i = 0; i < faceSize+1; ++i) {
    const uint32_t index = i;
    const glm::vec4 &axis = getPolySphereFace(verts, face, faceSize, ornFirst, firstPos, secondPos, index);

    project(axis, vert, vertSize, firstPos, invOrn, minSecond, maxSecond);
    project(axis, secondPos, secondRadius, minSecond, maxSecond);

    col = col && overlap(0.0f, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist);
  }

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return col;
}

// glm::vec4 getPolyPolyFace(const ArrayInterface<glm::vec4>* verts,
//                           const uint32_t &firstFace,  const uint32_t &firstFaceSize,  const glm::mat4 &firstOrn,
//                           const uint32_t &secondFace, const uint32_t &secondFaceSize, const glm::mat4 &secondOrn,
//                           const uint32_t &index) {
//   const glm::vec4 &polyNormal1 = verts->at(firstFace+index);
//   const glm::vec4 &polyNormal2 = verts->at(secondFace+(index%secondFaceSize));
//   
//   return index < firstFaceSize ?
//            firstOrn  * glm::vec4(polyNormal1.x, polyNormal1.y, polyNormal1.z, 0.0f) : 
//            secondOrn * glm::vec4(polyNormal2.x, polyNormal2.y, polyNormal2.z, 0.0f);
// }

bool CPUSolver::PolyPolySAT(const Object &first,  const uint32_t &transFirst,
                            const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const {
  float minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;

  const glm::vec4 &firstPos  = transFirst  != UINT32_MAX ? transforms->at(transFirst).pos  : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  const glm::vec4 &secondPos = transSecond != UINT32_MAX ? transforms->at(transSecond).pos : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

  const uint32_t firstVert     = first.vertexOffset;
  const uint32_t firstVertSize = first.vertexCount;
  const uint32_t firstFace     = firstVert + firstVertSize + 1;
  const uint32_t firstFaceSize = first.faceCount;

  const uint32_t secondVert     = second.vertexOffset;
  const uint32_t secondVertSize = second.vertexCount;
  const uint32_t secondFace     = secondVert + secondVertSize + 1;
  const uint32_t secondFaceSize = second.faceCount;

  const glm::mat4 &ornFirst = first.rotationDataIndex == UINT32_MAX ?
                                systems->at(first.coordinateSystemIndex) : 
                                rotationDatas->at(first.rotationDataIndex).matrix * systems->at(first.coordinateSystemIndex);
  const glm::mat4 &invOrnFirst = glm::inverse(ornFirst);

  const glm::mat4 &ornSecond = second.rotationDataIndex == UINT32_MAX ?
                                systems->at(second.coordinateSystemIndex) : 
                                rotationDatas->at(second.rotationDataIndex).matrix * systems->at(second.coordinateSystemIndex);
  const glm::mat4 &invOrnSecond = glm::inverse(ornSecond);

  glm::vec4 &localCenter1 = verts->at(firstVert + firstVertSize);
  localCenter1 = transform(localCenter1, firstPos, ornFirst);

  glm::vec4 &localCenter2 = verts->at(secondVert + secondVertSize);
  localCenter2 = transform(localCenter2, firstPos, ornSecond);

  const glm::vec4 &delta = localCenter1 - localCenter2;

  // нужно ли нормализовать полученные нормали?
  // мы аж 2 операции проводим, вряд ли вектора остаются нормализованными после этого
  // с другой стороны в буллете не проводится нормализация (хотя там не используются данные отсюда напрямую)
  // я так полагаю мне придется нормализовывать, нужно проверить!!!

  // у булета в проджектах используется Инверсе Ротайт
  // скорее всего если я инверсирую матрицу, я получу похожий результат

  bool col = true;
  for (uint32_t i = 0; i < firstFaceSize+secondFaceSize; ++i) {
    const uint32_t index = i;
    const glm::vec4 &axis = getPolyPolyFace(verts, firstFace, firstFaceSize, ornFirst, secondFace, secondFaceSize, ornSecond, index);

    project(axis, firstVert, firstVertSize, firstPos, invOrnFirst, minSecond, maxSecond);
    project(axis, secondVert, secondVertSize, secondPos, invOrnSecond, minSecond, maxSecond);

    col = col && overlap(0.0f, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist);
  }

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return col;
}

bool CPUSolver::SAT(const uint32_t &objectIndexFirst,  const uint32_t &transformIndexFirst, 
                    const uint32_t &objectIndexSecond, const uint32_t &transformIndexSecond, glm::vec4 &mtv, float &dist) const {
  const Object &first = objects->at(objectIndexFirst);
  const Object &second = objects->at(objectIndexSecond);
  bool col;

  //col = BoxBoxSAT(first, transformIndexFirst, second, transformIndexSecond, mtvLocal, distLocal);

  switch(first.objType.getObjType()) {
    case BBOX_TYPE:
      switch(second.objType.getObjType()) {
        case BBOX_TYPE:
          col = BoxBoxSAT(first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
          //powerOfTwo = 1;
        break;
        case SPHERE_TYPE:
          col = BoxSphereSAT(first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
        break;
        case POLYGON_TYPE:
          col = BoxPolySAT(first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
//           std::cout << "BBOX " << objectIndexFirst << " intersect POLY " << objectIndexSecond << '\n';
        break;
      }
    break;
    case SPHERE_TYPE:
      switch(second.objType.getObjType()) {
        case BBOX_TYPE:
          col = BoxSphereSAT(second, transformIndexSecond, first, transformIndexFirst, mtv, dist);
          mtv = -mtv;
        break;
        case SPHERE_TYPE:
          col = SphereSphereSAT(first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
        break;
        case POLYGON_TYPE:
          col = PolySphereSAT(second, transformIndexSecond, first, transformIndexFirst, mtv, dist);
          mtv = -mtv;
        break;
      }
    break;
    case POLYGON_TYPE:
      switch(second.objType.getObjType()) {
        case BBOX_TYPE:
          col = BoxPolySAT(second, transformIndexSecond, first, transformIndexFirst, mtv, dist);
          mtv = -mtv;
        break;
        case SPHERE_TYPE:
          col = PolySphereSAT(first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
        break;
        case POLYGON_TYPE:
          col = PolyPolySAT(first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
        break;
      }
    break;
  }

  return col;
}

float CPUSolver::SATOneAxis(const uint32_t &objectIndexFirst,  const uint32_t &transformIndexFirst, 
                            const uint32_t &objectIndexSecond, const uint32_t &transformIndexSecond, const glm::vec4 &axis) const {
  float minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
  const Object &first = objects->at(objectIndexFirst);
  const Object &second = objects->at(objectIndexSecond);

  switch(first.objType.getObjType()) {
    case BBOX_TYPE: {
      project(axis, transforms->at(transformIndexFirst).pos, verts->at(first.vertexOffset), systems->at(first.coordinateSystemIndex), minFirst, maxFirst);
    }
    break;
    case SPHERE_TYPE: {
      const glm::vec4 &tmpPos1 = transforms->at(transformIndexFirst).pos;
      const glm::vec4 pos = glm::vec4(tmpPos1.x, tmpPos1.y, tmpPos1.z, 1.0f);
      const float radius = tmpPos1.w;
      project(axis, pos, radius, minFirst, maxFirst);
    }
    break;
    case POLYGON_TYPE: {
      const glm::vec4 &pos = transformIndexFirst == UINT32_MAX ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(transformIndexFirst).pos;
      const uint32_t vert1     = first.vertexOffset;
      const uint32_t vertSize1 = first.vertexCount;
      glm::mat4 orn1 = first.rotationDataIndex == UINT32_MAX ?
                         systems->at(first.coordinateSystemIndex) : 
                         rotationDatas->at(first.rotationDataIndex).matrix * systems->at(first.coordinateSystemIndex);
              
      orn1 = glm::inverse(orn1);

      project(axis, vert1, vertSize1, pos, orn1, minFirst, maxFirst);
    }
    break;
  }

  switch(second.objType.getObjType()) {
    case BBOX_TYPE:
      project(axis, transforms->at(transformIndexSecond).pos, verts->at(second.vertexOffset), systems->at(second.coordinateSystemIndex), minSecond, maxSecond);
    break;
    case SPHERE_TYPE: {
      const glm::vec4 &tmpPos2 = transforms->at(transformIndexSecond).pos;
      const glm::vec4 pos = glm::vec4(tmpPos2.x, tmpPos2.y, tmpPos2.z, 1.0f);
      const float radius = tmpPos2.w;
      project(axis, pos, radius, minSecond, maxSecond);
    }
    break;
    case POLYGON_TYPE: {
      const glm::vec4 &pos = transformIndexSecond == UINT32_MAX ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(transformIndexSecond).pos;
      const uint32_t vert2     = second.vertexOffset;
      const uint32_t vertSize2 = second.vertexCount;
      glm::mat4 orn2 = second.rotationDataIndex == UINT32_MAX ?
                         systems->at(second.coordinateSystemIndex) : 
                         rotationDatas->at(second.rotationDataIndex).matrix * systems->at(second.coordinateSystemIndex);
              
      orn2 = glm::inverse(orn2);

      project(axis, vert2, vertSize2, pos, orn2, minSecond, maxSecond);
    }
    break;
  }

  const float test1 = minFirst - maxSecond;
  const float test2 = minSecond - maxFirst;
  
//   std::cout << "minFirst  " << minFirst << "  maxFirst  " << maxFirst << '\n';
//   std::cout << "minSecond " << minSecond << " maxSecond " << maxSecond << '\n';
//   std::cout << "test1 " << test1 << "\n";
//   std::cout << "test2 " << test2 << "\n";

  if (test1 > 0.0f || test2 > 0.0f) return 10000.0f;

  return glm::min(glm::abs(test1), glm::abs(test2));
}

void CPUSolver::computePair(const BroadphasePair& pair) {
  const uint32_t objIndex[2] = {pair.firstIndex, pair.secondIndex};

  const uint32_t staticPhysDataIndex[2] = {objects->at(objIndex[0]).staticPhysicDataIndex, objects->at(objIndex[1]).staticPhysicDataIndex};

  const uint32_t physDataIndex[2] = {staticPhysDatas->at(staticPhysDataIndex[0]).physDataIndex, staticPhysDatas->at(staticPhysDataIndex[1]).physDataIndex};

  const uint32_t transformIndex[2] = {objects->at(objIndex[0]).transformIndex, objects->at(objIndex[1]).transformIndex};

  const bool wasOnGround[2] = {physDataIndex[0] == UINT32_MAX ? false : bool(datas->at(physDataIndex[0]).onGroundBits & 0x2),
                              physDataIndex[1] == UINT32_MAX ? false : bool(datas->at(physDataIndex[1]).onGroundBits & 0x2)};

  bool col = false;
  //uint32_t dist = glm::floatBitsToUint(100000.0f);
  float dist = 100000.0f;
  glm::vec4 mtv = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  // здесь мы используем распараллеленный SAT
  col = SAT(objIndex[0], transformIndex[0], objIndex[1], transformIndex[1], mtv, dist);

  // col = bool(sharedBool);
  // if (dist == sharedDist) sharedMtv = mtv;

  // mtv = sharedMtv;
  const float distFloat = dist; //glm::uintBitsToFloat(sharedDist);

  const float mtvAngle = getAngle(-mtv, gravity->data()->gravityNormal);
  
  OverlappingDataForSolver data;

  data.pairData.x = pair.firstIndex;
  data.pairData.y = pair.secondIndex;
  data.pairData.z = uint32_t(col);
  data.pairData.w = glm::floatBitsToUint(mtvAngle);
  
  if (!bool(data.pairData.z) || distFloat < threshold) return; // выходить мы можем и раньше

  data.mtvDist.x = mtv.x;
  data.mtvDist.y = mtv.y;
  data.mtvDist.z = mtv.z;
  data.mtvDist.w = glm::max(distFloat - threshold, 0.0f);
  
  // начинаем поиск ступеньки
  const uint32_t taskCount = 2;
  for (uint32_t i = 0; i < taskCount; ++i) {
    const uint32_t index = i;

    glm::vec4 normal = gravity->data()->gravityNormal;// = getNormalCloseToGravity(A);
    
//     std::cout << "\n";
//     std::cout << "objIndex[1-index] " << objIndex[1-index] << "\n";
    // мне нужно взять нормаль ДРУГОГО объекта
    const uint32_t objType = objects->at(objIndex[1-index]).objType.getObjType();
    if (objType == BBOX_TYPE) {
      const uint32_t systemIndex = objects->at(objIndex[1-index]).coordinateSystemIndex;
      normal = getNormalCloseToGravity(systems->at(systemIndex), -gravity->data()->gravityNormal);
    } else if (objType == POLYGON_TYPE) {
      const uint32_t facesOffset = objects->at(objIndex[1-index]).vertexOffset + objects->at(objIndex[1-index]).vertexCount + 1;
      const uint32_t facesSize = objects->at(objIndex[1-index]).faceCount;
      
      const uint32_t systemIndex = objects->at(objIndex[1-index]).coordinateSystemIndex;
      const uint32_t rotationDataIndex = objects->at(objIndex[1-index]).rotationDataIndex;
      const glm::mat4 &orn = rotationDataIndex == UINT32_MAX ? systems->at(systemIndex) : rotationDatas->at(rotationDataIndex).matrix * systems->at(systemIndex);
      
      if (objects->at(objIndex[1-index]).vertexCount + 1 == facesSize) {
        const glm::vec4 &tmp = verts->at(facesOffset);
        normal = orn * glm::vec4(tmp.x, tmp.y, tmp.z, 0.0f);
      } else {
        uint32_t normalIndex;
        normal = getNormalCloseToGravity(orn, facesOffset, facesSize, -gravity->data()->gravityNormal, normalIndex);
      }
    }

    data.normals[index] = normal;

    // тут нам нужно решить какой из объектов может взобраться на другой
    // ну то есть ... что сделать? сравнить stairDist'ы?
    // тут со знаками может быть какая то хрень!!!!
    const float normalAngle = getAngle(normal, -(gravity->data()->gravityNormal));
    data.satAngleStair[index] = normalAngle;
    
//     std::cout << "normal x: " << normal.x << " y: " << normal.y << " z: " << normal.z << " w: " << normal.w << "\n";
//     std::cout << "normal angle " << normalAngle << "\n";

    float stairDist = 100000.0f;
    if (normalAngle < PI_Q && wasOnGround[index]) {
      stairDist = SATOneAxis(objIndex[index], transformIndex[index], objIndex[1-index], transformIndex[1-index], normal);
    }

    data.satAngleStair[index+2] = glm::max(stairDist - threshold, 0.0f);

    data.stairsMoves[index] = uint32_t(stairDist < (staticPhysDatas->at(staticPhysDataIndex[index]).stairHeight - EPSILON));
  }

//   const glm::vec4 &pos1 = transformIndex[0] == UINT32_MAX ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(transformIndex[0]).pos;
//   const glm::vec4 &pos2 = transformIndex[1] == UINT32_MAX ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(transformIndex[1]).pos;
  
  const glm::vec4 &vel1 = physDataIndex[0] == 0xFFFFFFFF ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : velocities->at(physDataIndex[0]);
  const glm::vec4 &vel2 = physDataIndex[1] == 0xFFFFFFFF ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : velocities->at(physDataIndex[1]);
  
  //const bool moves1 = glm::dot(pos2 - pos1, vel1) > 0.0f;
  //const bool moves2 = glm::dot(pos1 - pos2, vel2) > 0.0f;
  
  const bool moves1 = glm::dot(vel1, vel1) > 0.0f;
  const bool moves2 = glm::dot(vel2, vel2) > 0.0f;

  data.stairsMoves[2] = uint32_t(objects->at(objIndex[0]).objType.isDynamic() && moves1);
  data.stairsMoves[3] = uint32_t(objects->at(objIndex[1]).objType.isDynamic() && moves2);
  
  applyChanges(data);
}

void CPUSolver::computePairWithGround(const BroadphasePair &pair, const glm::vec4 &normal) {
  const uint32_t objIndex[2] = {pair.firstIndex, pair.secondIndex};

  const uint32_t staticPhysDataIndex[2] = {objects->at(objIndex[0]).staticPhysicDataIndex, objects->at(objIndex[1]).staticPhysicDataIndex};

  const uint32_t physDataIndex[2] = {staticPhysDatas->at(staticPhysDataIndex[0]).physDataIndex, staticPhysDatas->at(staticPhysDataIndex[1]).physDataIndex};

  const uint32_t transformIndex[2] = {objects->at(objIndex[0]).transformIndex, objects->at(objIndex[1]).transformIndex};

  const bool wasOnGround[2] = {physDataIndex[0] == UINT32_MAX ? false : bool(datas->at(physDataIndex[0]).onGroundBits & 0x2),
                              physDataIndex[1] == UINT32_MAX ? false : bool(datas->at(physDataIndex[1]).onGroundBits & 0x2)};

  bool col = false;
  //uint32_t dist = glm::floatBitsToUint(100000.0f);
  float dist = 100000.0f;
  glm::vec4 mtv = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  // здесь мы используем распараллеленный SAT
  col = SAT(objIndex[0], transformIndex[0], objIndex[1], transformIndex[1], mtv, dist);

  // col = bool(sharedBool);
  // if (dist == sharedDist) sharedMtv = mtv;

  // mtv = sharedMtv;
  const float distFloat = dist; //glm::uintBitsToFloat(sharedDist);

  const float mtvAngle = getAngle(-mtv, gravity->data()->gravityNormal);
  
  OverlappingDataForSolver data;

  data.pairData.x = pair.firstIndex;
  data.pairData.y = pair.secondIndex;
  data.pairData.z = uint32_t(col);
  data.pairData.w = glm::floatBitsToUint(mtvAngle);
  
  if (!bool(data.pairData.z) || distFloat < threshold) return; // выходить мы можем и раньше

  data.mtvDist.x = mtv.x;
  data.mtvDist.y = mtv.y;
  data.mtvDist.z = mtv.z;
  data.mtvDist.w = glm::max(distFloat - threshold, 0.0f);
  
  // начинаем поиск ступеньки
  data.normals[0] = normal;

  // тут нам нужно решить какой из объектов может взобраться на другой
  // ну то есть ... что сделать? сравнить stairDist'ы?
  // тут со знаками может быть какая то хрень!!!!
  const float normalAngle = getAngle(normal, -(gravity->data()->gravityNormal));
  data.satAngleStair[0] = normalAngle;
  
//   std::cout << "normal x: " << normal.x << " y: " << normal.y << " z: " << normal.z << " w: " << normal.w << "\n";
//   std::cout << "normal angle " << normalAngle << "\n";

  float stairDist = 100000.0f;
  if (normalAngle < PI_Q && wasOnGround[0]) {
    stairDist = SATOneAxis(objIndex[0], transformIndex[0], objIndex[1], transformIndex[1], normal);
  }

  data.satAngleStair[2] = glm::max(stairDist - threshold, 0.0f);
  data.satAngleStair[3] = 100000.0f;

  data.stairsMoves[0] = uint32_t(stairDist < (staticPhysDatas->at(staticPhysDataIndex[0]).stairHeight - EPSILON));
  data.stairsMoves[1] = 0;
  
//   std::cout << "data.satAngleStair[index+2] " << data.satAngleStair[2] << "\n";
//   std::cout << "data.stairsMoves[index] " << data.stairsMoves[0] << "\n";

//   const glm::vec4 &pos1 = transformIndex[0] == UINT32_MAX ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(transformIndex[0]).pos;
//   const glm::vec4 &pos2 = transformIndex[1] == UINT32_MAX ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(transformIndex[1]).pos;
// 
//   const bool moves1 = glm::dot(pos2 - pos1, (physDataIndex[0] == 0xFFFFFFFF ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : velocities->at(physDataIndex[0]))) > 0.0f;
//   const bool moves2 = glm::dot(pos1 - pos2, (physDataIndex[1] == 0xFFFFFFFF ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : velocities->at(physDataIndex[1]))) > 0.0f;

  data.stairsMoves[2] = uint32_t(objects->at(objIndex[0]).objType.isDynamic());// && moves1);
  data.stairsMoves[3] = uint32_t(objects->at(objIndex[1]).objType.isDynamic());// && moves2);
  
  applyChanges(data);
}

void CPUSolver::applyChanges(const OverlappingDataForSolver &data) {
  const uint32_t objIndex[2] = {data.pairData.x, data.pairData.y};

  const uint32_t staticPhysDataIndex[2] = {objects->at(objIndex[0]).staticPhysicDataIndex, objects->at(objIndex[1]).staticPhysicDataIndex};

  const uint32_t physDataIndex[2] = {staticPhysDatas->at(staticPhysDataIndex[0]).physDataIndex, staticPhysDatas->at(staticPhysDataIndex[1]).physDataIndex};

  const uint32_t transformIndex[2] = {objects->at(objIndex[0]).transformIndex, objects->at(objIndex[1]).transformIndex};

  const bool wasOnGround[2] = {physDataIndex[0] == UINT32_MAX ? false : bool(datas->at(physDataIndex[0]).onGroundBits & 0x2),
                              physDataIndex[1] == UINT32_MAX ? false : bool(datas->at(physDataIndex[1]).onGroundBits & 0x2)};
  
  // тут мы должны в одном потоке сделать следующие действия
  const uint32_t objType1 = objects->at(objIndex[0]).objType.getObjType();
  const uint32_t objType2 = objects->at(objIndex[1]).objType.getObjType();
//     if ((objType1 == BBOX_TYPE && objType2 == POLYGON_TYPE)) {
//       std::cout << "BBOX " << objIndex[0] << " POLY " << objIndex[1] << " intersect" << '\n';
//       std::cout << "Angle " << glm::uintBitsToFloat(data.pairData.w) << '\n';
//       std::cout << "Mtv           x: " << data.mtvDist.x << " y: " << data.mtvDist.y << " z: " << data.mtvDist.z << " w: " << data.mtvDist.w << '\n';
//       std::cout << "SatAngleStair x: " << data.satAngleStair.x << " y: " << data.satAngleStair.y << " z: " << data.satAngleStair.z << " w: " << data.satAngleStair.w << '\n';
//       std::cout << "StairsMoves   x: " << data.stairsMoves.x << " y: " << data.stairsMoves.y << " z: " << data.stairsMoves.z << " w: " << data.stairsMoves.w << '\n';
// 
//       //if (objIndex2 == 5010) throw std::runtime_error("5010");
//       //if (objIndex2 == 5106) throw std::runtime_error("5106");
//       if (data.pairData.y == 5100) throw std::runtime_error("5100");
//     }
// 
//         if ((objType1 == POLYGON_TYPE && objType2 == BBOX_TYPE)) {
//           std::cout << "POLY BBOX intersect" << '\n';
//           std::cout << "Angle " << glm::uintBitsToFloat(data.pairData.w) << '\n';
//           std::cout << "Mtv           x: " << data.mtvDist.x << " y: " << data.mtvDist.y << " z: " << data.mtvDist.z << " w: " << data.mtvDist.w << '\n';
//           std::cout << "SatAngleStair x: " << data.satAngleStair.x << " y: " << data.satAngleStair.y << " z: " << data.satAngleStair.z << " w: " << data.satAngleStair.w << '\n';
//           std::cout << "StairsMoves   x: " << data.stairsMoves.x << " y: " << data.stairsMoves.y << " z: " << data.stairsMoves.z << " w: " << data.stairsMoves.w << '\n';
// 
//           //exit(0);
//         }
// 
//         if ((objType1 == BBOX_TYPE && objType2 == BBOX_TYPE)) {
//           std::cout << "BBOX BBOX intersect" << '\n';
//           std::cout << "Angle " << glm::uintBitsToFloat(data.pairData.w) << '\n';
//           std::cout << "Mtv           x: " << data.mtvDist.x << " y: " << data.mtvDist.y << " z: " << data.mtvDist.z << " w: " << data.mtvDist.w << '\n';
//           std::cout << "SatAngleStair x: " << data.satAngleStair.x << " y: " << data.satAngleStair.y << " z: " << data.satAngleStair.z << " w: " << data.satAngleStair.w << '\n';
//           std::cout << "StairsMoves   x: " << data.stairsMoves.x << " y: " << data.stairsMoves.y << " z: " << data.stairsMoves.z << " w: " << data.stairsMoves.w << '\n';
// 
//           //exit(0);
//         }

  if ((glm::uintBitsToFloat(data.pairData.w) < PI_Q) && physDataIndex[0] != UINT32_MAX) {
    // это пол

    // мы его должны запомнить и пометить что объект на полу сейчас
    // по идее мы должны запомить индекс физдаты, но я хочу попробовать сократить количество данных
    // нужных каждому объекту, поэтому лучше всего конечно использовать индекс
    // Object'а, но мне нужно будет видимо кое-что переделать в предыдущих шагах

    // запоминать мы будем в физдате? или лучше в обжектах?
    // в обжекте тогда нужно дополнительные переменные вводить
    // но это куда лучше чем тащить физдату + трансформу + инпут для каждого объекта
    // как все же делать лифт? (по идее платформа + статик + скорость это и есть лифт)

    // если мы попали сюда то значит А стоит на B
    //if (physDataIndex1 != 0xFFFFFFFF) {
      objects->at(objIndex[0]).groundObjIndex = objIndex[1];
      datas->at(physDataIndex[0]).groundIndex = staticPhysDataIndex[1]; //physDataIndex2;
      datas->at(physDataIndex[0]).onGroundBits |= 0x1; // = uint32_t(true);
    //}
  } else if ((glm::abs(glm::uintBitsToFloat(data.pairData.w) - PI) < PI_Q) && physDataIndex[1] != UINT32_MAX) {
    // здесь учтем вариант когда B стоит на A
    //if (physDataIndex2 != 0xFFFFFFFF) {
      objects->at(objIndex[1]).groundObjIndex = objIndex[0];
      datas->at(physDataIndex[1]).groundIndex = staticPhysDataIndex[0]; //physDataIndex1;
      datas->at(physDataIndex[1]).onGroundBits |= 0x1; //uint32_t(true);
    //}
  } else if ((wasOnGround[0] || wasOnGround[1]) && glm::abs(glm::uintBitsToFloat(data.pairData.w) - PI_H) < PI_Q) {
    // возможно мы попали в вариант со ступенькой
    const float scalar1 = physDataIndex[0] == UINT32_MAX ? 0.0f : datas->at(physDataIndex[0]).scalar;
    const float scalar2 = physDataIndex[1] == UINT32_MAX ? 0.0f : datas->at(physDataIndex[1]).scalar;

    const uint32_t index = !bool(data.stairsMoves.x) ?
      (!bool(data.stairsMoves.y) ?
        (scalar2 < scalar1 ?
          objIndex[0] : objIndex[1])
        : objIndex[1])
      : objIndex[0];
    
    const bool objStair1 = (objIndex[0] == index) && wasOnGround[0] && bool(data.stairsMoves.x);
    const bool objStair2 = (objIndex[1] == index) && wasOnGround[1] && bool(data.stairsMoves.y);
    
    // что-то из этого должно быть правдой в итоге
    // аккуратно с индексами нормалей и стаиров
    // transforms[transformIndex1].pos = objStair1 ?
    //   transforms[transformIndex1].pos + data.normals[1]*data.satAngleStair.z - vec4(data.mtvDist.xyz, 0.0f)*0.01f :
    //   transforms[transformIndex1].pos;
    //
    // transforms[transformIndex2].pos = objStair2 ?
    //   transforms[transformIndex2].pos + data.normals[0]*data.satAngleStair.w + vec4(data.mtvDist.xyz, 0.0f)*0.01f :
    //   transforms[transformIndex2].pos;
    
    if (objStair1) {
      const glm::vec4 &mtvDist = data.mtvDist;
      const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
      transforms->at(transformIndex[0]).pos += data.normals[0]*data.satAngleStair.z - mtv*0.01f;

      objects->at(objIndex[0]).groundObjIndex = objIndex[1];
      datas->at(physDataIndex[0]).groundIndex = staticPhysDataIndex[1]; //physDataIndex2;
      datas->at(physDataIndex[0]).onGroundBits |= 0x1; //uint32_t(true);
    }
    
    if (objStair2) {
      const glm::vec4 &mtvDist = data.mtvDist;
      const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
      transforms->at(transformIndex[1]).pos += data.normals[1]*data.satAngleStair.w + mtv*0.01f;

      objects->at(objIndex[1]).groundObjIndex = objIndex[0];
      datas->at(physDataIndex[1]).groundIndex = staticPhysDataIndex[0]; //physDataIndex1;
      datas->at(physDataIndex[1]).onGroundBits |= 0x1; //uint32_t(true);
    }
    
//       std::cout << "Normal 0 x: " << data.normals[0].x << " y: " << data.normals[0].y << " z: " << data.normals[0].z << " w: " << data.normals[0].w << '\n';
//       std::cout << "Stair 0 " << data.satAngleStair.z << "\n";
//       
//       std::cout << "Normal 1 x: " << data.normals[1].x << " y: " << data.normals[1].y << " z: " << data.normals[1].z << " w: " << data.normals[1].w << '\n';
//       std::cout << "Stair 1 " << data.satAngleStair.w << "\n";
    
    if (objStair1 || objStair2) {
      //throw std::runtime_error("stair");
      
      return; // тут выходим если есть ступенька!
    }
  }

  uint32_t divider = 0;
  divider += data.stairsMoves.z + data.stairsMoves.w;
  if (divider == 0) return;

  const float move = data.mtvDist.w / float(divider);
  
  const float dt = MCS_TO_SEC(gravity->data()->time);
  
  // в принципе неплохо работает со скоростями (иногда вылетает, точнее пока что был только один случай)
  // пока что вылетает тогда когда челик стоит на мне, опять с индексами где-то напортачил
  // от поверхностей отскакивает, но не сильно. нужно ли это исправить? думаю что пока не будет отскакивает от пола можно с этим повременить
  // отскакивания появлялись потому что я прибавлял к обычной скорости эту штуку
  // в этом случае мы рисуем объект до изменения положения от столкновения, а значит мы каждый раз чутка входим в стенку и снова возвращаемся в следующем
  // как это исправить? впринципе ничего предугадать невозможно так как мы вычисляем скорость до вычислений пересечений
  // вычислять скорость в другом месте? нужно попробовать (чет пока ничего не вышло)
  
  // мне нужно сделать так чтобы я либо мог двигать объекты, но они не проходили сквозь стены
  // либо я не мог двигать объекты
  // возможно как раз мелкую скорость и можно придавать
  
  const float bounce = glm::min(staticPhysDatas->at(staticPhysDataIndex[0]).overbounce, staticPhysDatas->at(staticPhysDataIndex[1]).overbounce);
  if (bool(data.stairsMoves.z)) {
    const glm::vec4 &mtvDist = data.mtvDist;
    const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
    transforms->at(transformIndex[0]).pos += mtv * move;

    glm::vec4 vel1 = glm::vec4(datas->at(physDataIndex[0]).velocity, 0.0f);
    glm::vec4 vel1Global = velocities->at(physDataIndex[0]);

    clipVelocity( mtv, bounce, vel1);
    clipVelocity( mtv, bounce, vel1Global);

    datas->at(physDataIndex[0]).velocity = glm::vec3(vel1.x, vel1.y, vel1.z);// + glm::vec3((mtv * move) / dt);
    velocities->at(physDataIndex[0]) = vel1Global;// + (mtv * move) / dt;
    
    //lastVelocities[physDataIndex[0]] = (mtv * move) / dt;
  }

  if (bool(data.stairsMoves.w)) {
    const glm::vec4 &mtvDist = data.mtvDist;
    const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
    transforms->at(transformIndex[1]).pos -= mtv * move;

    glm::vec4 vel2 = glm::vec4(datas->at(physDataIndex[1]).velocity, 0.0f);
    glm::vec4 vel2Global = velocities->at(physDataIndex[1]);

    clipVelocity(-mtv, bounce, vel2);
    clipVelocity(-mtv, bounce, vel2Global);

    datas->at(physDataIndex[1]).velocity = glm::vec3(vel2.x, vel2.y, vel2.z);// - glm::vec3((mtv * move) / dt);
    velocities->at(physDataIndex[1]) = vel2Global;// - (mtv * move) / dt;
    
    //lastVelocities[physDataIndex[1]] = -(mtv * move) / dt;
  }
}


// float getAngle(const glm::vec4 &first, const glm::vec4 &second) {
//   const float dotV = glm::dot(first, second);
//   const float lenSq1 = glm::length2(first);
//   const float lenSq2 = glm::length2(second);
// 
//   return glm::acos(dotV / glm::sqrt(lenSq1 * lenSq2));
// }
// 
// void clipVelocity(const glm::vec4 &clipNormal, const float &bounce, glm::vec4 &vel) {
//   const float backoff = glm::dot(vel, clipNormal) * bounce;
// 
//   vel = vel - clipNormal * backoff;
// }
// 
// glm::vec4 transform(const glm::vec4 &p, const glm::vec4 &translation, const glm::mat4 &orientation) {
//   return orientation * p + translation;
// }
// 
// bool isNormalAdequate(const ArrayInterface<glm::vec4>* verts, const uint32_t &offset, const uint32_t &normalIndex, const glm::vec4 &normal) {
//   const uint32_t vertexIndex = glm::floatBitsToUint(verts->at(normalIndex).w);
//   const glm::vec4 &normalVertex = verts->at(vertexIndex);
//   
//   for (uint32_t i = offset; i < offset+3; ++i) {
//     const glm::vec4 &vert = verts->at(i);
//     
//     std::cout << "vert     x: " << vert.x << " y: " << vert.y << " z: " << vert.z << " w: " << vert.w << "\n" ;
//     
//     const glm::vec4 &vectorOP = vert - normalVertex;
//     
//     std::cout << "vectorOP x: " << vectorOP.x << " y: " << vectorOP.y << " z: " << vectorOP.z << " w: " << vectorOP.w << "\n" ;
//     
//     const float dist = glm::dot(vectorOP, normal);
//     
//     std::cout << "dist " << dist << '\n';
//     
//     if (glm::abs(dist) > EPSILON) return false;
//   }
//   
//   return true;
// }

// void CPUSolver::solve() {
//   auto start = std::chrono::steady_clock::now();
// 
//   const uint32_t iterationCount = 2;
// 
//   for (uint32_t iteration = 0; iteration < iterationCount; ++iteration) {
//     const float koef = 1.0f / float(iterationCount);
//     const uint32_t objCount = indicies->at(0);
// 
//     // все же сначало мы наверное пересчитаем позицию
//     for (uint32_t j = 0; j < objCount; ++j) {
//       const uint32_t objIndex = j;
// 
//       const uint32_t staticPhysDataIndex = objects->at(indicies->at(objIndex+1)).staticPhysicDataIndex;
//       const uint32_t physDataIndex = staticPhysDatas->at(staticPhysDataIndex).physDataIndex;
// 
//       if (physDataIndex == UINT32_MAX) continue;
// 
//       const uint32_t transformIndex = datas->at(physDataIndex).transformIndex;
//       const float dt = MCS_TO_SEC(gravity->data()->time);
// 
//       if (iteration == 0) {
//         datas->at(physDataIndex).onGroundBits = datas->at(physDataIndex).onGroundBits | ((datas->at(physDataIndex).onGroundBits & 0x1) << 1);
//         //datas->at(physDataIndex).onGroundBits &= 0x2;
//         transforms->at(transformIndex).pos = datas->at(physDataIndex).oldPos;
//       }
// 
//       // здесь мы перевычисляем позиции объектов
//       // нам нужна общая скорость (тип скорость объекта + скорость земли)
//       // по всей видимости тут нужен будет еще один буфер, где мы будем хранить скорость вычисленную еще в первом шаге
//       // скорости менять от столкновений нам придется видимо две
//       // куда поз сохранить? сразу в трансформы? или лучше в какой дополнительный буфер?
//       transforms->at(transformIndex).pos += koef * velocities->at(physDataIndex) * dt;
//     }
// 
// //     std::cout << "Pairs count " << pairs->at(0).firstIndex << '\n';
// 
//     for (uint32_t i = 1; i < pairs->at(0).secondIndex+1; ++i) {
//       const uint32_t pairIndex = i;
//       const BroadphasePair &pair = pairs->at(pairIndex);
// 
//       const uint32_t objIndex[2] = {pair.firstIndex, pair.secondIndex};
// 
//       const uint32_t staticPhysDataIndex[2] = {objects->at(objIndex[0]).staticPhysicDataIndex, objects->at(objIndex[1]).staticPhysicDataIndex};
// 
//       const uint32_t physDataIndex[2] = {staticPhysDatas->at(staticPhysDataIndex[0]).physDataIndex, staticPhysDatas->at(staticPhysDataIndex[1]).physDataIndex};
// 
//       const uint32_t transformIndex[2] = {objects->at(objIndex[0]).transformIndex, objects->at(objIndex[1]).transformIndex};
// 
//       const bool wasOnGround[2] = {physDataIndex[0] == UINT32_MAX ? false : bool(datas->at(physDataIndex[0]).onGroundBits & 0x2),
//                                    physDataIndex[1] == UINT32_MAX ? false : bool(datas->at(physDataIndex[1]).onGroundBits & 0x2)};
// 
//       bool col = false;
//       //uint32_t dist = glm::floatBitsToUint(100000.0f);
//       float dist = 100000.0f;
//       glm::vec4 mtv = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
//       // здесь мы используем распараллеленный SAT
//       col = SAT(objIndex[0], transformIndex[0], objIndex[1], transformIndex[1], mtv, dist);
// 
//       // col = bool(sharedBool);
//       // if (dist == sharedDist) sharedMtv = mtv;
// 
//       // mtv = sharedMtv;
//       float distFloat = dist; //glm::uintBitsToFloat(sharedDist);
// 
//       const float mtvAngle = getAngle(-mtv, gravity->data()->gravityNormal);
// 
//       tmpBuffer[pairIndex].pairData.x = pair.firstIndex;
//       tmpBuffer[pairIndex].pairData.y = pair.secondIndex;
//       tmpBuffer[pairIndex].pairData.z = uint32_t(col);
//       tmpBuffer[pairIndex].pairData.w = glm::floatBitsToUint(mtvAngle);
// 
//       tmpBuffer[pairIndex].mtvDist.x = mtv.x;
//       tmpBuffer[pairIndex].mtvDist.y = mtv.y;
//       tmpBuffer[pairIndex].mtvDist.z = mtv.z;
//       tmpBuffer[pairIndex].mtvDist.w = distFloat;
// 
//       const uint32_t taskCount = 2;
//       for (uint32_t i = 0; i < taskCount; ++i) {
//         const uint32_t index = i;
// 
//         glm::vec4 normal = gravity->data()->gravityNormal;// = getNormalCloseToGravity(A);
// 
//         // мне нужно взять нормаль ДРУГОГО объекта
//         const uint32_t objType = objects->at(objIndex[1-index]).objType.getObjType();
//         if (objType == BBOX_TYPE) {
//           const uint32_t systemIndex = objects->at(objIndex[1-index]).coordinateSystemIndex;
//           normal = getNormalCloseToGravity(systems->at(systemIndex), -gravity->data()->gravityNormal);
//         } else if (objType == POLYGON_TYPE) {
//           const uint32_t facesOffset = objects->at(objIndex[1-index]).vertexOffset + objects->at(objIndex[1-index]).vertexCount + 1;
//           const uint32_t facesSize = objects->at(objIndex[1-index]).faceCount;
// 
//           const uint32_t systemIndex = objects->at(objIndex[1-index]).coordinateSystemIndex;
//           const uint32_t rotationDataIndex = objects->at(objIndex[1-index]).rotationDataIndex;
//           const glm::mat4 &orn = rotationDataIndex == UINT32_MAX ? systems->at(systemIndex) : rotationDatas->at(rotationDataIndex).matrix * systems->at(systemIndex);
//           // что тут делать?
//           uint32_t normalIndex;
//           normal = getNormalCloseToGravity(orn, facesOffset, facesSize, -gravity->data()->gravityNormal, normalIndex);
//         }
// 
//         tmpBuffer[pairIndex].normals[index] = normal;
// 
//         // тут нам нужно решить какой из объектов может взобраться на другой
//         // ну то есть ... что сделать? сравнить stairDist'ы?
//         // тут со знаками может быть какая то хрень!!!!
//         const float normalAngle = getAngle(normal, -(gravity->data()->gravityNormal));
//         tmpBuffer[pairIndex].satAngleStair[index] = normalAngle;
// 
//         float stairDist = 100000.0f;
//         if (normalAngle < PI_Q) {
//           stairDist = SATOneAxis(objIndex[index], transformIndex[index], objIndex[1-index], transformIndex[1-index], normal);
//           
// //           std::cout << "stairDist " << stairDist << "\n";
//         }
// 
//         tmpBuffer[pairIndex].satAngleStair[index+2] = stairDist;
// 
//         tmpBuffer[pairIndex].stairsMoves[index] = uint32_t(stairDist < (staticPhysDatas->at(staticPhysDataIndex[index]).stairHeight - EPSILON));
//         
// //         std::cout << "tmpBuffer[pairIndex].satAngleStair[index+2] " << tmpBuffer[pairIndex].satAngleStair[index+2] << "\n";
// //         std::cout << "tmpBuffer[pairIndex].stairsMoves[index] " << tmpBuffer[pairIndex].stairsMoves[index] << "\n";
//         
//         if (objIndex[1-index] == 5100) throw std::runtime_error(std::to_string(normalAngle));
//       }
// 
//       const glm::vec4 &pos1 = transformIndex[0] == UINT32_MAX ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(transformIndex[0]).pos;
//       const glm::vec4 &pos2 = transformIndex[1] == UINT32_MAX ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(transformIndex[1]).pos;
// 
//       const bool moves1 = glm::dot(pos2 - pos1, (physDataIndex[0] == 0xFFFFFFFF ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : velocities->at(physDataIndex[0]))) > 0.0f;
//       const bool moves2 = glm::dot(pos1 - pos2, (physDataIndex[1] == 0xFFFFFFFF ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : velocities->at(physDataIndex[1]))) > 0.0f;
// 
//       tmpBuffer[pairIndex].stairsMoves[2] = uint32_t(objects->at(objIndex[0]).objType.isDynamic());// && moves1);
//       tmpBuffer[pairIndex].stairsMoves[3] = uint32_t(objects->at(objIndex[1]).objType.isDynamic());// && moves2);
//     }
// 
//     for (uint32_t i = 1; i < islands->at(0).islandIndex+1; ++i) {
//       const uint32_t islandIndex = i;
// 
//       for (uint32_t j = 0; j < islands->at(islandIndex).size; ++j) {
//         const uint32_t pairIndex = islands->at(islandIndex).offset + j + 1; // +1 tmpBuffer здесь начинается с 1 !!
//         const OverlappingDataForSolver &data = tmpBuffer[pairIndex];
// 
//         if (!bool(data.pairData.z)) continue;
// 
//         const uint32_t objIndex1 = data.pairData.x;
//         const uint32_t objIndex2 = data.pairData.y;
// 
//         const uint32_t staticPhysDataIndex1 = objects->at(objIndex1).staticPhysicDataIndex;
//         const uint32_t staticPhysDataIndex2 = objects->at(objIndex2).staticPhysicDataIndex;
// 
//         const uint32_t physDataIndex1 = staticPhysDatas->at(staticPhysDataIndex1).physDataIndex;
//         const uint32_t physDataIndex2 = staticPhysDatas->at(staticPhysDataIndex2).physDataIndex;
// 
//         const uint32_t transformIndex1 = objects->at(objIndex1).transformIndex; //floatBitsToUint(datas[physDataIndex1].constants.z);
//         const uint32_t transformIndex2 = objects->at(objIndex2).transformIndex; //floatBitsToUint(datas[physDataIndex2].constants.z);
// 
//         // const bool wasOnGround1 = physDataIndex1 == 0xFFFFFFFF ? false : bool(datas[physDataIndex1].additionalData.w);
//         // const bool wasOnGround2 = physDataIndex2 == 0xFFFFFFFF ? false : bool(datas[physDataIndex2].additionalData.w);
// 
//         const bool wasOnGround1 = physDataIndex1 == UINT32_MAX ? false : bool(datas->at(physDataIndex1).onGroundBits & 0x2);
//         const bool wasOnGround2 = physDataIndex2 == UINT32_MAX ? false : bool(datas->at(physDataIndex2).onGroundBits & 0x2);
// 
//         const uint32_t objType1 = objects->at(objIndex1).objType.getObjType();
//         const uint32_t objType2 = objects->at(objIndex2).objType.getObjType();
// //         if ((objType1 == BBOX_TYPE && objType2 == POLYGON_TYPE)) {
// //           std::cout << "BBOX " << objIndex1 << " POLY " << objIndex2 << " intersect" << '\n';
// //           std::cout << "Angle " << glm::uintBitsToFloat(data.pairData.w) << '\n';
// //           std::cout << "Mtv           x: " << data.mtvDist.x << " y: " << data.mtvDist.y << " z: " << data.mtvDist.z << " w: " << data.mtvDist.w << '\n';
// //           std::cout << "SatAngleStair x: " << data.satAngleStair.x << " y: " << data.satAngleStair.y << " z: " << data.satAngleStair.z << " w: " << data.satAngleStair.w << '\n';
// //           std::cout << "StairsMoves   x: " << data.stairsMoves.x << " y: " << data.stairsMoves.y << " z: " << data.stairsMoves.z << " w: " << data.stairsMoves.w << '\n';
// // 
// //           //if (objIndex2 == 5010) throw std::runtime_error("5010");
// //           //if (objIndex2 == 5106) throw std::runtime_error("5106");
// //           if (data.pairData.y == 5100) throw std::runtime_error("5100");
// //         }
// // 
// //         if ((objType1 == POLYGON_TYPE && objType2 == BBOX_TYPE)) {
// //           std::cout << "POLY BBOX intersect" << '\n';
// //           std::cout << "Angle " << glm::uintBitsToFloat(data.pairData.w) << '\n';
// //           std::cout << "Mtv           x: " << data.mtvDist.x << " y: " << data.mtvDist.y << " z: " << data.mtvDist.z << " w: " << data.mtvDist.w << '\n';
// //           std::cout << "SatAngleStair x: " << data.satAngleStair.x << " y: " << data.satAngleStair.y << " z: " << data.satAngleStair.z << " w: " << data.satAngleStair.w << '\n';
// //           std::cout << "StairsMoves   x: " << data.stairsMoves.x << " y: " << data.stairsMoves.y << " z: " << data.stairsMoves.z << " w: " << data.stairsMoves.w << '\n';
// // 
// //           //exit(0);
// //         }
// // 
// //         if ((objType1 == BBOX_TYPE && objType2 == BBOX_TYPE)) {
// //           std::cout << "BBOX BBOX intersect" << '\n';
// //           std::cout << "Angle " << glm::uintBitsToFloat(data.pairData.w) << '\n';
// //           std::cout << "Mtv           x: " << data.mtvDist.x << " y: " << data.mtvDist.y << " z: " << data.mtvDist.z << " w: " << data.mtvDist.w << '\n';
// //           std::cout << "SatAngleStair x: " << data.satAngleStair.x << " y: " << data.satAngleStair.y << " z: " << data.satAngleStair.z << " w: " << data.satAngleStair.w << '\n';
// //           std::cout << "StairsMoves   x: " << data.stairsMoves.x << " y: " << data.stairsMoves.y << " z: " << data.stairsMoves.z << " w: " << data.stairsMoves.w << '\n';
// // 
// //           //exit(0);
// //         }
// 
//         if (glm::uintBitsToFloat(data.pairData.w) < PI_Q) {
//           // это пол
// 
//           // мы его должны запомнить и пометить что объект на полу сейчас
//           // по идее мы должны запомить индекс физдаты, но я хочу попробовать сократить количество данных
//           // нужных каждому объекту, поэтому лучше всего конечно использовать индекс
//           // Object'а, но мне нужно будет видимо кое-что переделать в предыдущих шагах
// 
//           // запоминать мы будем в физдате? или лучше в обжектах?
//           // в обжекте тогда нужно дополнительные переменные вводить
//           // но это куда лучше чем тащить физдату + трансформу + инпут для каждого объекта
//           // как все же делать лифт? (по идее платформа + статик + скорость это и есть лифт)
// 
//           // если мы попали сюда то значит А стоит на B
//           //if (physDataIndex1 != 0xFFFFFFFF) {
//             objects->at(objIndex1).groundObjIndex = objIndex2;
//             datas->at(physDataIndex1).groundIndex = staticPhysDataIndex2; //physDataIndex2;
//             datas->at(physDataIndex1).onGroundBits |= 0x1; // = uint32_t(true);
//           //}
//         } else if (glm::abs(glm::uintBitsToFloat(data.pairData.w) - PI) < PI_Q) {
//           // здесь учтем вариант когда B стоит на A
//           //if (physDataIndex2 != 0xFFFFFFFF) {
//             objects->at(objIndex2).groundObjIndex = objIndex1;
//             datas->at(physDataIndex2).groundIndex = staticPhysDataIndex1; //physDataIndex1;
//             datas->at(physDataIndex2).onGroundBits |= 0x1; //uint32_t(true);
//           //}
//         } else if ((wasOnGround1 || wasOnGround2) && glm::abs(glm::uintBitsToFloat(data.pairData.w) - PI_H) < PI_Q) {
//           // возможно мы попали в вариант со ступенькой
//           const float scalar1 = physDataIndex1 == UINT32_MAX ? 0.0f : datas->at(physDataIndex1).scalar;
//           const float scalar2 = physDataIndex2 == UINT32_MAX ? 0.0f : datas->at(physDataIndex2).scalar;
// 
//           const uint32_t index = !bool(data.stairsMoves.x) ?
//             (!bool(data.stairsMoves.y) ?
//               (scalar2 < scalar1 ?
//                 objIndex1 : objIndex2)
//               : objIndex2)
//             : objIndex1;
//           
//           const bool objStair1 = (objIndex1 == index) && wasOnGround1 && bool(data.stairsMoves.x);
//           const bool objStair2 = (objIndex2 == index) && wasOnGround2 && bool(data.stairsMoves.y);
//           
//           // что-то из этого должно быть правдой в итоге
//           // аккуратно с индексами нормалей и стаиров
//           // transforms[transformIndex1].pos = objStair1 ?
//           //   transforms[transformIndex1].pos + data.normals[1]*data.satAngleStair.z - vec4(data.mtvDist.xyz, 0.0f)*0.01f :
//           //   transforms[transformIndex1].pos;
//           //
//           // transforms[transformIndex2].pos = objStair2 ?
//           //   transforms[transformIndex2].pos + data.normals[0]*data.satAngleStair.w + vec4(data.mtvDist.xyz, 0.0f)*0.01f :
//           //   transforms[transformIndex2].pos;
//           
//           if (objStair1) {
//             const glm::vec4 &mtvDist = data.mtvDist;
//             const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
//             transforms->at(transformIndex1).pos += data.normals[0]*data.satAngleStair.z - mtv*0.01f;
// 
//             objects->at(objIndex1).groundObjIndex = objIndex2;
//             datas->at(physDataIndex1).groundIndex = staticPhysDataIndex2; //physDataIndex2;
//             datas->at(physDataIndex1).onGroundBits |= 0x1; //uint32_t(true);
//           }
//           
//           if (objStair2) {
//             const glm::vec4 &mtvDist = data.mtvDist;
//             const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
//             transforms->at(transformIndex2).pos += data.normals[1]*data.satAngleStair.w + mtv*0.01f;
// 
//             objects->at(objIndex2).groundObjIndex = objIndex1;
//             datas->at(physDataIndex2).groundIndex = staticPhysDataIndex1; //physDataIndex1;
//             datas->at(physDataIndex2).onGroundBits |= 0x1; //uint32_t(true);
//           }
//           
// //           std::cout << "Normal 0 x: " << data.normals[0].x << " y: " << data.normals[0].y << " z: " << data.normals[0].z << " w: " << data.normals[0].w << '\n';
// //           std::cout << "Stair 0 " << data.satAngleStair.z << "\n";
// //           
// //           std::cout << "Normal 1 x: " << data.normals[1].x << " y: " << data.normals[1].y << " z: " << data.normals[1].z << " w: " << data.normals[1].w << '\n';
// //           std::cout << "Stair 1 " << data.satAngleStair.w << "\n";
//           
//           if (objStair1 || objStair2) {
//             //throw std::runtime_error("stair");
//             
//             continue; // тут выходим если есть ступенька!
//           }
//         }
// 
//         uint32_t divider = 0;
//         divider += data.stairsMoves.z + data.stairsMoves.w;
//         if (divider == 0) continue;
// 
//         const float move = data.mtvDist.w / float(divider);
// 
//         const float bounce = glm::min(staticPhysDatas->at(staticPhysDataIndex1).overbounce, staticPhysDatas->at(staticPhysDataIndex2).overbounce);
//         if (bool(data.stairsMoves.z)) {
//           const glm::vec4 &mtvDist = data.mtvDist;
//           const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
//           transforms->at(transformIndex1).pos += mtv * move;
// 
//           glm::vec4 vel1 = glm::vec4(datas->at(physDataIndex1).velocity, 0.0f);
//           glm::vec4 vel1Global = velocities->at(physDataIndex1);
// 
//           clipVelocity( mtv, bounce, vel1);
//           clipVelocity( mtv, bounce, vel1Global);
// 
//           datas->at(physDataIndex1).velocity = glm::vec3(vel1.x, vel1.y, vel1.z);
//           velocities->at(physDataIndex1) = vel1Global;
//         }
// 
//         if (bool(data.stairsMoves.w)) {
//           const glm::vec4 &mtvDist = data.mtvDist;
//           const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
//           transforms->at(transformIndex2).pos -= mtv * move;
// 
//           glm::vec4 vel2 = glm::vec4(datas->at(physDataIndex2).velocity, 0.0f);
//           glm::vec4 vel2Global = velocities->at(physDataIndex2);
// 
//           clipVelocity(-mtv, bounce, vel2);
//           clipVelocity(-mtv, bounce, vel2Global);
// 
//           datas->at(physDataIndex2).velocity = glm::vec3(vel2.x, vel2.y, vel2.z);
//           velocities->at(physDataIndex2) = vel2Global;
//         }
//       }
//     }
//   }
//   
// //   static bool tmp = false;
// //   if (islands->at(0).islandIndex != 0) {
// //     tmp = true;
// //   }
// //   
// //   if (tmp) {
// //     throw std::runtime_error("Island size not 0");
// //   }
// 
//   auto end = std::chrono::steady_clock::now() - start;
//   auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
// //   std::cout << "Solver time : " << mcs << " mcs" << '\n';
// }
