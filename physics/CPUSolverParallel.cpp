#include "CPUSolverParallel.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <glm/gtx/norm.hpp>
#include <cstring>

#include <HelperFunctions.h>

#define ONLY_TRIGGER_ID 0xFFFFFFFF-1

// float getAngle(const glm::vec4 &first, const glm::vec4 &second);
// void clipVelocity(const glm::vec4 &clipNormal, const float &bounce, glm::vec4 &vel);
// glm::vec4 transform(const glm::vec4 &p, const glm::vec4 &translation, const glm::mat4 &orientation);
//
// bool isNormalAdequate(const ArrayInterface<glm::vec4>* verts, const uint32_t &offset, const uint32_t &normalIndex, const glm::vec4 &normal);

template <typename T>
uint32_t binarySearch(T* begin, T* end, const uint32_t &firstObjIndex);
template <typename T>
uint32_t getToStart(T* arr, const uint32_t &firstObjIndex, const uint32_t &foundedIndex);

CPUSolverParallel::CPUSolverParallel(dt::thread_pool* pool, const float &threshold/*, const uint32_t &iterationCount*/) {
  this->pool = pool;
  this->threshold = threshold;
//   this->iterationCount = iterationCount;

  //lastVelocities.resize(4000);
}
CPUSolverParallel::~CPUSolverParallel() {}

void CPUSolverParallel::setInputBuffers(const InputBuffers &buffers, void* indirectIslandCount, void* indirectPairCount) {
  (void)indirectIslandCount;
  (void)indirectPairCount;

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

void CPUSolverParallel::setOutputBuffers(const OutputBuffers &buffers) {
  overlappingData = buffers.overlappingData;
  dataIndices = buffers.dataIndices;

  raysData = buffers.raysData;
  raysIndices = buffers.raysIndices;

  triggerIndices = buffers.triggerIndices;
}

void CPUSolverParallel::updateBuffers(const uint32_t &pairsCount, const uint32_t &rayPairs) {
  if (overlappingData->size() < pairsCount * 1.5f) overlappingData->resize(pairsCount * 1.5f);
  if (triggerIndices->size() < overlappingData->size()) triggerIndices->resize(overlappingData->size());
  if (raysData->size() < rayPairs) raysData->resize(rayPairs);

  //if (tmpBuffer.size() < pairsCount+1) tmpBuffer.resize(pairsCount+1);
}

void CPUSolverParallel::calculateData() {
  //static const auto searchAndAddDynamic = [&] (const BroadphasePair &pair, std::atomic<uint32_t> &counter) {
  static const auto searchAndAddDynamic = [&] (const size_t &start, const size_t &count, const BroadphasePair* pairs, std::atomic<uint32_t> &counter) {
    for (size_t i = start; i < start+count; ++i) {
      const BroadphasePair &pair = pairs[i+1];

      const uint32_t overlappingDataCount = dataIndices->data()->count;

      bool found = false;
      const uint32_t foundedIndex = binarySearch(overlappingData->data(), overlappingData->data()+overlappingDataCount, pair.firstIndex);

      if (foundedIndex != UINT32_MAX) {
  //       std::cout << "first index " << pair.firstIndex << "\n";
  //       std::cout << "first index found " << foundedIndex << "\n";

        // мы вполне можем позволить себе сделать здесь цикл, так как объектов 250% будет около 3-ех (это скорее всего даже максимум)
        uint32_t startIndex = getToStart(overlappingData->data(), pair.firstIndex, foundedIndex);
        uint32_t pairFirstIndex = overlappingData->at(startIndex).firstIndex;
  //       std::cout << "start index " << startIndex << "\n";

        while (pairFirstIndex == pair.firstIndex) {
          // может ли интересующий меня индекс быть вторым? может (тип нельзя гарантировать порядок в динамических парах)
          if (pair.firstIndex == overlappingData->at(startIndex).firstIndex && pair.secondIndex == overlappingData->at(startIndex).secondIndex) {
            //throw std::runtime_error("founded " + std::to_string(pair.firstIndex) + " " + std::to_string(pair.secondIndex));
            // здесь по идее нужно выйти
            found = true;
            break;
          }

          ++startIndex;
          if (startIndex >= overlappingDataCount) break;
          pairFirstIndex = overlappingData->at(startIndex).firstIndex;
        }

        if (found) {
          //overlappingDatas[startIndex].pairData.w = 0;
          return;
        }
      }

      uint32_t foundedIndex2 = UINT32_MAX;
      if (!found) foundedIndex2 = binarySearch(overlappingData->data(), overlappingData->data()+overlappingDataCount, pair.secondIndex);

      if (foundedIndex2 != UINT32_MAX) {
  //       std::cout << "second index " << pair.secondIndex << "\n";
  //       std::cout << "second index found " << foundedIndex2 << "\n";
        // мы вполне можем позволить себе сделать здесь цикл, так как объектов 250% будет около 3-ех (это скорее всего даже максимум)
        uint32_t startIndex = getToStart(overlappingData->data(), pair.secondIndex, foundedIndex2);
        uint32_t pairFirstIndex = overlappingData->at(startIndex).firstIndex;
  //       std::cout << "start index " << startIndex << "\n";

        while (pairFirstIndex == pair.secondIndex) {
          // может ли интересующий меня индекс быть вторым? может (тип нельзя гарантировать порядок в динамических парах)
          if (pair.secondIndex == overlappingData->at(startIndex).firstIndex && pair.firstIndex == overlappingData->at(startIndex).secondIndex) {
            // здесь по идее нужно выйти
            //throw std::runtime_error("founded " + std::to_string(pair.secondIndex) + " " + std::to_string(pair.firstIndex));
            found = true;
            break;
          }

          ++startIndex;
          if (startIndex >= overlappingDataCount) break;
          pairFirstIndex = overlappingData->at(startIndex).secondIndex;
        }

        if (found) {
          //overlappingDatas[startIndex].pairData.w = 0;
          //pair.islandIndex
          return;
        }
      }

      // добавляем пару
      const uint32_t id = counter.fetch_add(1);

      //if (foundedIndex != UINT32_MAX || foundedIndex2 != UINT32_MAX) throw std::runtime_error("something goes wrong");

  //     std::cout << "Could not find " << pair.firstIndex << " " << pair.secondIndex << "\n";

      overlappingData->at(id).firstIndex  = foundedIndex2 == UINT32_MAX ? pair.firstIndex  : pair.secondIndex;
      overlappingData->at(id).secondIndex = foundedIndex2 == UINT32_MAX ? pair.secondIndex : pair.firstIndex;
      overlappingData->at(id).hasCollision = 0;
      overlappingData->at(id).dummy = pair.islandIndex == ONLY_TRIGGER_ID ? 1 : 0;
    }
  };

  //static const auto computeOverlappingData = [&] (const uint32_t &index, std::atomic<uint32_t> &counter) {
  static const auto computeOverlappingData = [&] (const size_t &start, const size_t &count, std::atomic<uint32_t> &counter, std::atomic<uint32_t> &triggerCounter) {
    for (size_t index = start; index < start+count; ++index) {
      // тут нужны дополнительные данные + заполнить triggerIndices

      const OverlappingData &data = overlappingData->at(index);

      const uint32_t objIndex1 = data.firstIndex;
      const uint32_t objIndex2 = data.secondIndex;

      const uint32_t transformIndex1 = objects->at(objIndex1).transformIndex;
      const uint32_t transformIndex2 = objects->at(objIndex2).transformIndex;

      bool col = false;
      float dist = 100000.0f;
      glm::vec4 mtv = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
      // при вычислении на гпу мы используем распараллеленный SAT
      // здесь он обычный, его можно как нибудь ускорить?
      col = SAT(0.0f, objIndex1, transformIndex1, objIndex2, transformIndex2, mtv, dist);

      if (!col) {
        counter.fetch_add(-1);

        overlappingData->at(index).firstIndex  = UINT32_MAX;
        overlappingData->at(index).secondIndex = UINT32_MAX;
        overlappingData->at(index).hasCollision = 0;
        overlappingData->at(index).dummy = UINT32_MAX;
      } else {
        overlappingData->at(index).vec = glm::vec3(mtv);
        overlappingData->at(index).dist = dist;

        // нужно добавить пары, которые либо являются триггерами, либо могут быть триггерами
        const bool triggerAdd = bool(overlappingData->at(index).dummy);
        if (triggerAdd) {
          const uint32_t id = triggerCounter.fetch_add(1);
          triggerIndices->at(id) = index;
        }
      }
    }
  };

  //throw std::runtime_error("Not implemented yet");

  // сюда приходят пары
  // мне нужно заполнить overlappingData
  // причем мне нужно отсюда удалить старые пары и добавить новые, то есть

  // RegionLog rl("CPUSolverParallel::calculateData()");

  dataIndices->data()->indirectX = 1;
  dataIndices->data()->indirectY = 1;
  dataIndices->data()->indirectZ = 1;

//   for (uint32_t i = 0; i < dataIndices->data()->count; ++i) {
//     std::cout << "Pair " << i << " ids: " << overlappingData->at(i).firstIndex << " " << overlappingData->at(i).secondIndex << "\n";
//   }

  dataIndices->data()->count = dataIndices->data()->temporaryCount;

  std::atomic<uint32_t> counter(dataIndices->data()->count);

  // приходящие пары мне нужно найти и если их нет, то нужно добавить
  // причем у меня теперь есть как статики так и динамики

  const uint32_t dynamicPairsCount = pairs->at(0).secondIndex + glm::floatBitsToUint(pairs->at(0).dist);
  const uint32_t staticPairsCount  = staticPairs->at(0).firstIndex;

//   for (uint32_t i = 0; i < dynamicPairsCount; ++i) {
//     const BroadphasePair &pair = pairs->at(i+1);
//     if (pair.islandIndex == UINT32_MAX) continue;
//
//     pool->submitnr(searchAndAddDynamic, pair, std::ref(counter));
//   }

  {
    const size_t count = std::ceil(float(dynamicPairsCount) / float(pool->size()+1));
    size_t start = 0;
    for (size_t i = 0; i < pool->size()+1; ++i) {
      const uint32_t jobCount = std::min(count, dynamicPairsCount-start);
      if (jobCount == 0) break;

      pool->submitnr(searchAndAddDynamic, start, jobCount, pairs->data(), std::ref(counter));

      start += jobCount;
    }
  }

//   for (uint32_t i = 0; i < staticPairsCount; ++i) {
//     const BroadphasePair &pair = staticPairs->at(i+1);
//     if (pair.islandIndex == UINT32_MAX) continue;
//
//     pool->submitnr(searchAndAddDynamic, pair, std::ref(counter));
//   }

  {
    const size_t count = std::ceil(float(staticPairsCount) / float(pool->size()+1));
    size_t start = 0;
    for (size_t i = 0; i < pool->size()+1; ++i) {
      const uint32_t jobCount = std::min(count, staticPairsCount-start);
      if (jobCount == 0) break;

      pool->submitnr(searchAndAddDynamic, start, jobCount, staticPairs->data(), std::ref(counter));

      start += jobCount;
    }
  }

  pool->compute();
  pool->wait();

  dataIndices->data()->count = counter;
  dataIndices->data()->indirectX = counter;
  const uint32_t newSize = counter;

//   //std::atomic<uint32_t> triggerCounter(0);
//   std::cout << "new size " << newSize << "\n";
//   std::cout << "dynamicPairsCount " << dynamicPairsCount << "\n";
//   std::cout << "staticPairsCount " << staticPairsCount << "\n";
//
//   for (uint32_t i = 0; i < dataIndices->data()->count; ++i) {
//     std::cout << "Pair " << i << " ids: " << overlappingData->at(i).firstIndex << " " << overlappingData->at(i).secondIndex << "\n";
//   }
//
//   throw std::runtime_error("computeOverlappingData");

//   for (uint32_t i = 0; i < newSize; ++i) {
//     pool->submitnr(computeOverlappingData, i, std::ref(counter));
//   }

  std::atomic<uint32_t> triggerCounter(0);
  {
    const size_t count = std::ceil(float(newSize) / float(pool->size()+1));
    size_t start = 0;
    for (size_t i = 0; i < pool->size()+1; ++i) {
      const uint32_t jobCount = std::min(count, newSize-start);
      if (jobCount == 0) break;

      pool->submitnr(computeOverlappingData, start, jobCount, std::ref(counter), std::ref(triggerCounter));

      start += jobCount;
    }
  }

  pool->compute();
  pool->wait();

  dataIndices->data()->temporaryCount = counter;
  dataIndices->data()->triggerIndicesCount = triggerCounter;
//   triggerIndices->at(0) = triggerCounter;

//   for (uint32_t i = 0; i < dataIndices->data()->count; ++i) {
//     std::cout << "Pair " << i << " ids: " << overlappingData->at(i).firstIndex << " " << overlappingData->at(i).secondIndex << "\n";
//   }
}

void CPUSolverParallel::calculateRayData() {
//   static const auto rayIntersection = [&] (const uint32_t &index, std::atomic<uint32_t> &counter) {
  static const auto rayIntersection = [&] (const size_t &start, const size_t &count, std::atomic<uint32_t> &counter) {
    for (size_t index = start; index < start+count; ++index) {
      const BroadphasePair &pair = rayPairs->at(index+1);

      const uint32_t rayIndex = pair.firstIndex;
      const uint32_t objIndex = pair.secondIndex;

      const uint32_t transformIndex = objects->at(objIndex).transformIndex;

      glm::vec4 point = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
      const bool col = intersect(rayIndex, objIndex, transformIndex, point);
      //const bool col = true;

      if (col) {
        const uint32_t id = counter.fetch_add(1);

        raysData->at(id).firstIndex = rayIndex;
        raysData->at(id).secondIndex = objIndex;
        raysData->at(id).hasCollision = uint32_t(col);
        raysData->at(id).dummy = 0;

        raysData->at(id).vec = glm::vec3(point);
        raysData->at(id).dist = glm::distance2(rays->at(rayIndex).pos, point);
      }
    }
  };

  // RegionLog rl("CPUSolverParallel::calculateRayData()", true);

  raysIndices->data()->indirectX = 1;
  raysIndices->data()->indirectY = 1;
  raysIndices->data()->indirectZ = 1;
  raysIndices->data()->count = 0;
  raysIndices->data()->temporaryCount = 0;
  raysIndices->data()->powerOfTwo = 0;

  std::atomic<uint32_t> counter(0);

  //if (rayPairs->at(0).firstIndex == 0) throw std::runtime_error("rayPairs->at(0).firstIndex == 0");

//   for (uint32_t i = 0; i < rayPairs->at(0).firstIndex; ++i) {
//     pool->submitnr(rayIntersection, i+1, std::ref(counter));
//   }

  const size_t count = std::ceil(float(rayPairs->at(0).firstIndex) / float(pool->size()+1));
  size_t start = 0;
  for (size_t i = 0; i < pool->size()+1; ++i) {
    const uint32_t jobCount = std::min(count, rayPairs->at(0).firstIndex-start);
    if (jobCount == 0) break;

    pool->submitnr(rayIntersection, start, jobCount, std::ref(counter));

    start += jobCount;
  }

  pool->compute();
  pool->wait();

  raysIndices->data()->count = counter;
  raysIndices->data()->temporaryCount = counter;
}

void CPUSolverParallel::solve() {
  // тут наверное быстрее сработает так
  // static const auto calcPos = [&] (const float &koef, const uint32_t &iteration, const uint32_t &index) {
  //   const uint32_t objIndex = indicies->at(index+1);
  //   const uint32_t staticPhysDataIndex = objects->at(objIndex).staticPhysicDataIndex;
  //   const uint32_t physDataIndex = staticPhysDatas->at(staticPhysDataIndex).physDataIndex;
  //
  //   if (physDataIndex == UINT32_MAX) return;
  //
  //   const uint32_t transformIndex = datas->at(physDataIndex).transformIndex;
  //   const float dt = MCS_TO_SEC(gravity->data()->time);
  //
  //   if (iteration == 0) {
  //     //datas->at(physDataIndex).onGroundBits = datas->at(physDataIndex).onGroundBits | ((datas->at(physDataIndex).onGroundBits & 0x1) << 1);
  //     datas->at(physDataIndex).onGroundBits <<= 1;
  //     datas->at(physDataIndex).onGroundBits &= 0x2;
  //     datas->at(physDataIndex).onGroundBits &= 0x3;
  //     datas->at(physDataIndex).groundIndex = UINT32_MAX;
  //     objects->at(objIndex).groundObjIndex = UINT32_MAX;
  //     transforms->at(transformIndex).pos = datas->at(physDataIndex).oldPos;
  //   }
  //
  //   // здесь мы перевычисляем позиции объектов
  //   // нам нужна общая скорость (тип скорость объекта + скорость земли)
  //   // по всей видимости тут нужен будет еще один буфер, где мы будем хранить скорость вычисленную еще в первом шаге
  //   // скорости менять от столкновений нам придется видимо две
  //   // куда поз сохранить? сразу в трансформы? или лучше в какой дополнительный буфер?
  //   transforms->at(transformIndex).pos += koef * velocities->at(physDataIndex) * dt;
  // };

  static const auto prepareObjects = [&] (const size_t &start, const size_t &count) {
    for (size_t index = start; index < start+count; ++index) {
      const uint32_t objIndex = indicies->at(index+1);
      const uint32_t staticPhysDataIndex = objects->at(objIndex).staticPhysicDataIndex;
      const uint32_t physDataIndex = staticPhysDatas->at(staticPhysDataIndex).physDataIndex;

      if (physDataIndex == UINT32_MAX) return;

//       const uint32_t transformIndex = datas->at(physDataIndex).transformIndex;

      //datas->at(physDataIndex).onGroundBits = datas->at(physDataIndex).onGroundBits | ((datas->at(physDataIndex).onGroundBits & 0x1) << 1);
      datas->at(physDataIndex).onGroundBits <<= 1;
      datas->at(physDataIndex).onGroundBits &= 0x2;
      datas->at(physDataIndex).onGroundBits &= 0x3;
      datas->at(physDataIndex).groundIndex = UINT32_MAX;
      objects->at(objIndex).groundObjIndex = UINT32_MAX;
//       transforms->at(transformIndex).pos = datas->at(physDataIndex).oldPos;
    }
  };

  //static const auto solveStatic = [&] (const IslandData &island) {
  static const auto solveStatic = [&] (const size_t &start, const size_t &count, const IslandData* islandsPtr) {
    for (size_t i = start; i < start+count; ++i) {
      const IslandData &island = islandsPtr[i];

      // здесь последовательно вычисляем пары в острове
      glm::vec4 normal;

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
  };

  //static const auto solveDynamic = [&] (const IslandData &island) {
  static const auto solveDynamic = [&] (const size_t &start, const size_t &count, const IslandData* islandsPtr) {
    for (size_t i = start; i < start+count; ++i) {
      const IslandData &island = islandsPtr[i];

      // и здесь последовательно вычисляем каждую пару внутри острова
      for (uint32_t j = 0; j < island.size; ++j) {
        const uint32_t pairIndex = island.offset + j + 1;
        const BroadphasePair &pair = pairs->at(pairIndex);

        computePair(pair);
      }
    }
  };

//   const float koef = 1.0f / float(iterationCount);

  // RegionLog rl("CPUSolverParallel::solve()");

//   for (uint32_t iteration = 0; iteration < iterationCount; ++iteration) {
//     std::cout << "iteration " << iteration << '\n';
    // перевычисление позиции возможно стоит сделать линейным
//     const uint32_t objCount = indicies->at(0);
//     for (uint32_t j = 0; j < objCount; ++j) {
//       pool->submitnr(calcPos, koef, iteration, j+1);
//     }

//     pool->compute();
//     pool->wait();

  {
    const uint32_t objCount = indicies->at(0);
    const size_t count = std::ceil(float(objCount) / float(pool->size()+1));
    size_t start = 0;
    for (size_t i = 0; i < pool->size()+1; ++i) {
      const uint32_t jobCount = std::min(count, objCount-start);
      if (jobCount == 0) break;

      pool->submitnr(prepareObjects, start, jobCount);

      start += jobCount;
    }
  }

    struct IslandAdditionalData {
      glm::uvec4 octreeDepth;
      glm::uvec4 octreeLevels[10];
      glm::uvec4 islandCount;
    };

    if (staticPairs->at(0).firstIndex > 0) {
      const IslandAdditionalData* data = staticIslands->structure_from_begin<IslandAdditionalData>();
      const IslandData* islandsPtr = staticIslands->data_from<IslandAdditionalData>();

      // и начинаем вычисление пар
      // сначала у нас идет параллельное вычисление статиков
      // для статических пар тоже нужны острова так то (по индексам первых объектов)

      const size_t count = std::ceil(float(data->islandCount.x) / float(pool->size()+1));
      size_t start = 0;
      //for (uint32_t i = 0; i < data->islandCount.x; ++i) {
      for (size_t i = 0; i < pool->size()+1; ++i) {
        const uint32_t jobCount = std::min(count, data->islandCount.x-start);
        if (jobCount == 0) break;

//         const uint32_t islandIndex = i;
//         const IslandData &island = islandsPtr[islandIndex];

//         pool->submitnr(solveStatic, island);
        pool->submitnr(solveStatic, start, jobCount, islandsPtr);

        start += jobCount;
      }

      pool->compute();
      pool->wait();
    }

    // а затем вычисление динамиков, которых мы разбили по уровням октодерева
    if (pairs->at(0).secondIndex > 0) {
      const IslandAdditionalData* data = islands->structure_from_begin<IslandAdditionalData>();
      const IslandData* islandsPtr = islands->data_from<IslandAdditionalData>();

      for (uint32_t depth = 0; depth < data->octreeDepth.x; ++depth) {
        const glm::uvec4 &octreeLevel = data->octreeLevels[depth];

        // затем параллелим по островам
        if (octreeLevel.z > 0) {
//           std::cout << "octree level size " << octreeLevel.z << '\n';
          const size_t count = std::ceil(float(octreeLevel.z) / float(pool->size()+1));
          size_t start = 0;
          //for (uint32_t i = 0; i < octreeLevel.z; ++i) {
          for (size_t i = 0; i < pool->size()+1; ++i) {
//             const IslandData &island = islandsPtr[octreeLevel.y + i];

//             std::cout << "dynamic pair index " << (octreeLevel.y + i) << '\n';

//             pool->submitnr(solveDynamic, island);

            const uint32_t jobCount = std::min(count, octreeLevel.z-start);
            if (jobCount == 0) break;

            pool->submitnr(solveDynamic, start, jobCount, islandsPtr);

            start += jobCount;
          }

          pool->compute();
          pool->wait();
        }
      }
    }
}

void CPUSolverParallel::printStats() {
  std::cout << "CPU solver" << '\n';
}

glm::vec4 CPUSolverParallel::getNormalCloseToGravity(const glm::mat4 &orn, const glm::vec4 &gravityNorm) const {
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

glm::vec4 CPUSolverParallel::getNormalCloseToGravity(const glm::mat4 &orn, const uint32_t &offset, const uint32_t &facesCount, const glm::vec4 &gravityNorm, uint32_t &retIndex) const {
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

void CPUSolverParallel::project(const glm::vec4 &axis, const uint32_t &offset, const uint32_t &size, const glm::vec4 &pos, const glm::mat4 &invOrn, float &minRet, float &maxRet) const {
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

void CPUSolverParallel::project(const glm::vec4 &axis, const glm::vec4 &pos, const glm::vec4 &ext, const glm::mat4 &orn, float &minRet, float &maxRet) const {
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
}

void CPUSolverParallel::project(const glm::vec4 &axis, const glm::vec4 &pos, const float &radius, float &minRet, float &maxRet) const {
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
//
//   if (d < dist) {
//     mtv = axis;
//     dist = d;
//   }
//
//   return true;
// }

// glm::vec4 getBoxBoxFace(const glm::mat4 &orn1, const glm::mat4 &orn2, const uint32_t &index) {
//   // СЧЕТ НАЧИНАЕТСЯ С НУЛЯ (0!!!) СЛЕДОВАТЕЛЬНО 3-ИЙ ВЕКТОР ИМЕЕТ ИНДЕКС 2
//   // А ЗНАЧИТ ВСЕ ЧТО БОЛЬШЕ 2 (ДВУХ) (А НЕ ТРЕХ) УЖЕ ДРУГАЯ МАТРИЦА
//   return index > 2 ? orn1[index%3] : orn2[index];
// }

bool CPUSolverParallel::BoxBoxSAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
                          const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const {
  // тут по идее нужен еще вариант когда у нас в этих индексах стоит 0xFFFFFFFF
  // но думаю этот вариант добавлю позже (нужен он вообще? для него скорее всего придется делать более нормальный солвер)
  const glm::mat4 &sys1 = systems->at(first.coordinateSystemIndex);
  const glm::mat4 &sys2 = systems->at(second.coordinateSystemIndex);

  float minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;

  const glm::vec4 &firstPos  = transforms->at(transFirst).pos;
  const glm::vec4 &secondPos = transforms->at(transSecond).pos;

  const glm::vec4 &firstExt  = verts->at(first.vertexOffset);
  const glm::vec4 &secondExt = verts->at(second.vertexOffset);

  const glm::vec4 &delta = firstPos - secondPos;

  for (uint32_t i = 0; i < 3+3; ++i) {
    const uint32_t index = i;
    const glm::vec4 &axis = getBoxBoxFace(sys1, sys2, index);

    project(axis, firstPos,  firstExt,  sys1,  minFirst,  maxFirst);
    project(axis, secondPos, secondExt, sys2, minSecond, maxSecond);

    if (!overlap(treshold, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist)) return false;
  }

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return true;
}

// glm::vec4 getBoxSphereFace(const glm::mat4 &orn, const glm::vec4 &pos1, const glm::vec4 &pos2, const uint32_t &index) {
//   return index > 0 ? orn[index%3] : glm::normalize(pos2 - pos1);
// }

bool CPUSolverParallel::BoxSphereSAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
                                     const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const {
  (void)second;
  const glm::mat4 &sys = systems->at(first.coordinateSystemIndex);
  float minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;

  const glm::vec4 &firstPos  = transforms->at(transFirst).pos;
  const glm::vec4 &secondPos = glm::vec4(transforms->at(transSecond).pos.x, transforms->at(transSecond).pos.y, transforms->at(transSecond).pos.z, 1.0f);

  const glm::vec4 &firstExt  = verts->at(first.vertexOffset);
  const float radius = transforms->at(transSecond).pos.w;

  const glm::vec4 &delta = firstPos - secondPos;

  for (uint32_t i = 0; i < 3+1; ++i) {
    const uint32_t index = i;
    const glm::vec4 &axis = getBoxSphereFace(sys, firstPos, secondPos, index);

    project(axis, firstPos,  firstExt,  sys,  minFirst,  maxFirst);
    project(axis, secondPos, radius, minSecond, maxSecond);

    if (!overlap(treshold, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist)) return false;
  }

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return true;
}

// нужно или не нужно нормализовывать?
// glm::vec4 getBoxPolyFace(const ArrayInterface<glm::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const glm::mat4 &polyOrn, const glm::mat4 &orn, const uint32_t &index) {
//   const glm::vec4 &polyNormal = verts->at(face+(index%faceSize));
//   return index > 2 ? polyOrn * glm::vec4(polyNormal.x, polyNormal.y, polyNormal.z, 0.0f) : orn[index];
// }

bool CPUSolverParallel::BoxPolySAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
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

  const glm::vec4 &delta = firstPos - localCenter;

  for (uint32_t i = 0; i < faceSize+3; ++i) {
    const uint32_t index = i;
    const glm::vec4 &axis = getBoxPolyFace(verts, face, faceSize, orn, sys, index);

    project(axis, firstPos,  firstExt,  sys,  minFirst,  maxFirst);
    project(axis, vert, vertSize, secondPos, invOrn,  minSecond, maxSecond);

    if (!overlap(treshold, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist)) return false;
  }

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return true;
}

bool CPUSolverParallel::SphereSphereSAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
                                        const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const {
  (void)first;
  (void)second;
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

  if (!overlap(treshold, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist)) return false;

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return true;
}

// glm::vec4 getPolySphereFace(const ArrayInterface<glm::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const glm::mat4 &ornFirst, const glm::vec4 &pos1, const glm::vec4 &pos2, const uint32_t &index) {
//   const glm::vec4 &polyNormal = verts->at(face+(index%faceSize));
//   return index > 0 ? ornFirst * glm::vec4(polyNormal.x, polyNormal.y, polyNormal.z, 0.0f) : glm::normalize(pos2 - pos1);
// }

bool CPUSolverParallel::PolySphereSAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
                                      const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const {
  (void)second;
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

  for (uint32_t i = 0; i < faceSize+1; ++i) {
    const uint32_t index = i;
    const glm::vec4 &axis = getPolySphereFace(verts, face, faceSize, ornFirst, firstPos, secondPos, index);

    project(axis, vert, vertSize, firstPos, invOrn, minSecond, maxSecond);
    project(axis, secondPos, secondRadius, minSecond, maxSecond);

    if (!overlap(treshold, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist)) return false;
  }

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return true;
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

bool CPUSolverParallel::PolyPolySAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
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

  for (uint32_t i = 0; i < firstFaceSize+secondFaceSize; ++i) {
    const uint32_t index = i;
    const glm::vec4 &axis = getPolyPolyFace(verts, firstFace, firstFaceSize, ornFirst, secondFace, secondFaceSize, ornSecond, index);

    project(axis, firstVert, firstVertSize, firstPos, invOrnFirst, minSecond, maxSecond);
    project(axis, secondVert, secondVertSize, secondPos, invOrnSecond, minSecond, maxSecond);

    if (!overlap(treshold, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist)) return false;
  }

  if (glm::dot(-delta, mtv) > 0.0f) mtv = -mtv;

  return true;
}

// собственно главная функция из-за чего мы тут все собрались
// она вычисляется гораздо быстрее чем то что у меня было раньше конечно,
// но тут по прежнему много всякой фигни (например инверсы)
// могут ли эти функции быть еще быстрее? хотя на момент 13.11.2018 я еще не замерял
bool CPUSolverParallel::SAT(const float &treshold, const uint32_t &objectIndexFirst,  const uint32_t &transformIndexFirst,
                    const uint32_t &objectIndexSecond, const uint32_t &transformIndexSecond, glm::vec4 &mtv, float &dist) const {
  const Object &first = objects->at(objectIndexFirst);
  const Object &second = objects->at(objectIndexSecond);
  bool col;
  const float threshold1 = treshold;

  switch(first.objType.getObjType()) {
    case BBOX_TYPE:
      switch(second.objType.getObjType()) {
        case BBOX_TYPE:
          col = BoxBoxSAT(threshold1, first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
        break;
        case SPHERE_TYPE:
          col = BoxSphereSAT(threshold1, first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
        break;
        case POLYGON_TYPE:
          col = BoxPolySAT(threshold1, first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
        break;
      }
    break;
    case SPHERE_TYPE:
      switch(second.objType.getObjType()) {
        case BBOX_TYPE:
          col = BoxSphereSAT(threshold1, second, transformIndexSecond, first, transformIndexFirst, mtv, dist);
          mtv = -mtv;
        break;
        case SPHERE_TYPE:
          col = SphereSphereSAT(threshold1, first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
        break;
        case POLYGON_TYPE:
          col = PolySphereSAT(threshold1, second, transformIndexSecond, first, transformIndexFirst, mtv, dist);
          mtv = -mtv;
        break;
      }
    break;
    case POLYGON_TYPE:
      switch(second.objType.getObjType()) {
        case BBOX_TYPE:
          col = BoxPolySAT(threshold1, second, transformIndexSecond, first, transformIndexFirst, mtv, dist);
          mtv = -mtv;
        break;
        case SPHERE_TYPE:
          col = PolySphereSAT(threshold1, first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
        break;
        case POLYGON_TYPE:
          col = PolyPolySAT(threshold1, first, transformIndexFirst, second, transformIndexSecond, mtv, dist);
        break;
      }
    break;
  }

  return col;
}

float CPUSolverParallel::SATOneAxis(const uint32_t &objectIndexFirst,  const uint32_t &transformIndexFirst,
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

void CPUSolverParallel::computePair(const BroadphasePair& pair) {
  const uint32_t objIndex[2] = {pair.firstIndex, pair.secondIndex};

  const uint32_t staticPhysDataIndex[2] = {objects->at(objIndex[0]).staticPhysicDataIndex, objects->at(objIndex[1]).staticPhysicDataIndex};

  const uint32_t physDataIndex[2] = {staticPhysDatas->at(staticPhysDataIndex[0]).physDataIndex, staticPhysDatas->at(staticPhysDataIndex[1]).physDataIndex};

  const uint32_t transformIndex[2] = {objects->at(objIndex[0]).transformIndex, objects->at(objIndex[1]).transformIndex};

  const bool wasOnGround[2] = {physDataIndex[0] == UINT32_MAX ? false : bool(datas->at(physDataIndex[0]).onGroundBits & 0x2),
                              physDataIndex[1] == UINT32_MAX ? false : bool(datas->at(physDataIndex[1]).onGroundBits & 0x2)};

  bool col = false;
  float dist = 100000.0f;
  glm::vec4 mtv = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  // при вычислении на гпу мы используем распараллеленный SAT
  // здесь он обычный, его можно как нибудь ускорить?
  //const float newThreshold = (1.0f / float(iterationCount)) * threshold;
  const float newThreshold = threshold;
  col = SAT(newThreshold, objIndex[0], transformIndex[0], objIndex[1], transformIndex[1], mtv, dist);

  const float mtvAngle = getAngle(-mtv, gravity->data()->gravityNormal);

  OverlappingDataForSolver data;

  data.pairData.x = pair.firstIndex;
  data.pairData.y = pair.secondIndex;
  data.pairData.z = uint32_t(col);
  data.pairData.w = glm::floatBitsToUint(mtvAngle);

  if (!bool(data.pairData.z)/* || dist < threshold*/) return;

  data.mtvDist.x = mtv.x;
  data.mtvDist.y = mtv.y;
  data.mtvDist.z = mtv.z;
  data.mtvDist.w = dist;//glm::max(dist - threshold, 0.0f);

  glm::vec4 objCenter[2] = {glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};

  // начинаем поиск ступеньки
  const uint32_t taskCount = 2;
  for (uint32_t i = 0; i < taskCount; ++i) {
    const uint32_t index = i;

    glm::vec4 normal = gravity->data()->gravityNormal;

    // мне нужно взять нормаль ДРУГОГО объекта
    const uint32_t objType = objects->at(objIndex[1-index]).objType.getObjType();
    if (objType == BBOX_TYPE) {
      const uint32_t systemIndex = objects->at(objIndex[1-index]).coordinateSystemIndex;
      normal = getNormalCloseToGravity(systems->at(systemIndex), -gravity->data()->gravityNormal);

      objCenter[index] = transforms->at(transformIndex[1-index]).pos;
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

      const uint32_t localCenter = objects->at(objIndex[1-index]).vertexOffset + objects->at(objIndex[1-index]).vertexCount;
      const glm::vec4 dir = transformIndex[1-index] == UINT32_MAX ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : transforms->at(transformIndex[1-index]).pos;
      objCenter[index] = transform(verts->at(localCenter), dir, orn);
    } else {
      objCenter[index] = transforms->at(transformIndex[1-index]).pos;
    }

    data.normals[index] = normal;

    const float normalAngle = getAngle(normal, -(gravity->data()->gravityNormal));
    data.satAngleStair[index] = normalAngle;

    float stairDist = 100000.0f;
    if (normalAngle < PI_Q && wasOnGround[index]) {
      stairDist = SATOneAxis(objIndex[index], transformIndex[index], objIndex[1-index], transformIndex[1-index], normal);
    }

    data.satAngleStair[index+2] = glm::max(stairDist - threshold, 0.0f);

    data.stairsMoves[index] = uint32_t(stairDist < (staticPhysDatas->at(staticPhysDataIndex[index]).stairHeight - EPSILON));
  }

  // исправляем знак (я в принципе делаю тоже самое и в САТ'е)
  data.stairsMoves[0] = uint32_t(bool(data.stairsMoves[0]) && glm::dot(objCenter[1] - objCenter[0], data.normals[0]) > 0.0f);
  data.stairsMoves[1] = uint32_t(bool(data.stairsMoves[1]) && glm::dot(objCenter[0] - objCenter[1], data.normals[1]) > 0.0f);

  const glm::vec4 &vel1 = physDataIndex[0] == 0xFFFFFFFF ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : velocities->at(physDataIndex[0]);
  const glm::vec4 &vel2 = physDataIndex[1] == 0xFFFFFFFF ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : velocities->at(physDataIndex[1]);

  const bool moves1 = glm::dot(vel1, vel1) > 0.0f;
  const bool moves2 = glm::dot(vel2, vel2) > 0.0f;

  data.stairsMoves[2] = uint32_t(objects->at(objIndex[0]).objType.isDynamic() && moves1);
  data.stairsMoves[3] = uint32_t(objects->at(objIndex[1]).objType.isDynamic() && moves2);

  applyChanges(data);
}

void CPUSolverParallel::computePairWithGround(const BroadphasePair &pair, const glm::vec4 &normal) {
  const uint32_t objIndex[2] = {pair.firstIndex, pair.secondIndex};

  const uint32_t staticPhysDataIndex[2] = {objects->at(objIndex[0]).staticPhysicDataIndex, objects->at(objIndex[1]).staticPhysicDataIndex};

  const uint32_t physDataIndex[2] = {staticPhysDatas->at(staticPhysDataIndex[0]).physDataIndex, staticPhysDatas->at(staticPhysDataIndex[1]).physDataIndex};

  const uint32_t transformIndex[2] = {objects->at(objIndex[0]).transformIndex, objects->at(objIndex[1]).transformIndex};

  const bool wasOnGround[2] = {physDataIndex[0] == UINT32_MAX ? false : bool(datas->at(physDataIndex[0]).onGroundBits & 0x2),
                              physDataIndex[1] == UINT32_MAX ? false : bool(datas->at(physDataIndex[1]).onGroundBits & 0x2)};

  bool col = false;
  float dist = 100000.0f;
  glm::vec4 mtv = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  // при вычислении на гпу мы используем распараллеленный SAT
  // здесь он обычный, его можно как нибудь ускорить?
  //const float newThreshold = (1.0f / float(iterationCount)) * threshold;
  const float newThreshold = threshold;

  col = SAT(newThreshold, objIndex[0], transformIndex[0], objIndex[1], transformIndex[1], mtv, dist);

  const float mtvAngle = getAngle(-mtv, gravity->data()->gravityNormal);

  OverlappingDataForSolver data;

  data.pairData.x = pair.firstIndex;
  data.pairData.y = pair.secondIndex;
  data.pairData.z = uint32_t(col);
  data.pairData.w = glm::floatBitsToUint(mtvAngle);

  if (!bool(data.pairData.z)/* || dist < threshold*/) return; // выходить мы можем и раньше

  data.mtvDist.x = mtv.x;
  data.mtvDist.y = mtv.y;
  data.mtvDist.z = mtv.z;
  data.mtvDist.w = dist;//glm::max(dist - threshold, 0.0f);

//   glm::vec4 objCenter = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
//   const uint32_t objType = objects->at(objIndex[0]).objType.getObjType();
//     if (objType == BBOX_TYPE) {
//       objCenter = transforms->at(transformIndex[0]).pos;
//     } else if (objType == POLYGON_TYPE) {
//       const uint32_t systemIndex = objects->at(objIndex[0]).coordinateSystemIndex;
//       const uint32_t rotationDataIndex = objects->at(objIndex[0]).rotationDataIndex;
//       const glm::mat4 &orn = rotationDataIndex == UINT32_MAX ? systems->at(systemIndex) : rotationDatas->at(rotationDataIndex).matrix * systems->at(systemIndex);
//
//       const uint32_t localCenter = objects->at(objIndex[0]).vertexOffset + objects->at(objIndex[0]).vertexCount;
//       const glm::vec4 dir = transformIndex[0] == UINT32_MAX ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : glm::vec4(glm::vec3(transforms->at(transformIndex[0]).pos), 0.0f);
//       objCenter = transform(verts->at(localCenter), dir, orn);
//     } else {
//       objCenter = transforms->at(transformIndex[0]).pos;
//     }

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

void CPUSolverParallel::applyChanges(const OverlappingDataForSolver &data) {
  const uint32_t objIndex[2] = {data.pairData.x, data.pairData.y};

  const uint32_t staticPhysDataIndex[2] = {objects->at(objIndex[0]).staticPhysicDataIndex, objects->at(objIndex[1]).staticPhysicDataIndex};

  const uint32_t physDataIndex[2] = {staticPhysDatas->at(staticPhysDataIndex[0]).physDataIndex, staticPhysDatas->at(staticPhysDataIndex[1]).physDataIndex};

  const uint32_t transformIndex[2] = {objects->at(objIndex[0]).transformIndex, objects->at(objIndex[1]).transformIndex};

  const bool wasOnGround[2] = {physDataIndex[0] == UINT32_MAX ? false : bool(datas->at(physDataIndex[0]).onGroundBits & 0x2),
                               physDataIndex[1] == UINT32_MAX ? false : bool(datas->at(physDataIndex[1]).onGroundBits & 0x2)};

  // тут мы должны в одном потоке сделать следующие действия
//   const uint32_t objType1 = objects->at(objIndex[0]).objType.getObjType();
//   const uint32_t objType2 = objects->at(objIndex[1]).objType.getObjType();

  // это условие physDataIndex[1] != UINT32_MAX по идее должно защищать от случаев когда нормаль на карте расставлена неверно
  // вообще я сомневаюсь что я все учел здесь
  if ((glm::uintBitsToFloat(data.pairData.w) < PI_Q) && physDataIndex[0] != UINT32_MAX) {
    // если мы попали сюда то значит А стоит на B

    objects->at(objIndex[0]).groundObjIndex = objIndex[1];
    datas->at(physDataIndex[0]).groundIndex = staticPhysDataIndex[1];
    datas->at(physDataIndex[0]).onGroundBits |= 0x1;
  } else if ((glm::abs(glm::uintBitsToFloat(data.pairData.w) - PI) < PI_Q) && physDataIndex[1] != UINT32_MAX) {
    // здесь учтем вариант когда B стоит на A

    objects->at(objIndex[1]).groundObjIndex = objIndex[0];
    datas->at(physDataIndex[1]).groundIndex = staticPhysDataIndex[0]; //physDataIndex1;
    datas->at(physDataIndex[1]).onGroundBits |= 0x1; //uint32_t(true);
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

    if (objStair1) {
      const glm::vec4 &mtvDist = data.mtvDist;
      const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
      transforms->at(transformIndex[0]).pos += data.normals[0]*data.satAngleStair.z - mtv*0.01f;

      objects->at(objIndex[0]).groundObjIndex = objIndex[1];
      datas->at(physDataIndex[0]).groundIndex = staticPhysDataIndex[1];
      datas->at(physDataIndex[0]).onGroundBits |= 0x1;
    }

    if (objStair2) {
      const glm::vec4 &mtvDist = data.mtvDist;
      const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
      transforms->at(transformIndex[1]).pos += data.normals[1]*data.satAngleStair.w + mtv*0.01f;

      objects->at(objIndex[1]).groundObjIndex = objIndex[0];
      datas->at(physDataIndex[1]).groundIndex = staticPhysDataIndex[0];
      datas->at(physDataIndex[1]).onGroundBits |= 0x1;
    }

    if (objStair1 || objStair2) return; // тут выходим если есть ступенька!
  }

  uint32_t divider = 0;
  divider += data.stairsMoves.z + data.stairsMoves.w;
  if (divider == 0) return;

  const float move = data.mtvDist.w / float(divider);

  //const float dt = MCS_TO_SEC(gravity->data()->time);

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

  // мне в будущем нужно как то перейти к вычислению скоростей здесь
  // скорее всего с изменением этого придет и изменение вычисления островов
  // в принципе скорее всего то как я вычисляю острова сейчас - норм
  // ну а если вычисление констраинтов можно будет разделить на статики и динамики то будет вообще замечательно

  const float bounce = glm::min(staticPhysDatas->at(staticPhysDataIndex[0]).overbounce, staticPhysDatas->at(staticPhysDataIndex[1]).overbounce);
  if (bool(data.stairsMoves.z)) {
    const glm::vec4 &mtvDist = data.mtvDist;
    const glm::vec4 &mtv = glm::vec4(mtvDist.x, mtvDist.y, mtvDist.z, 0.0f);
    transforms->at(transformIndex[0]).pos += mtv * move;

//     if (transforms->at(transformIndex[0]).pos.x != transforms->at(transformIndex[0]).pos.x) {
//       std::cout << "indices " << objIndex[0] << " " << objIndex[1] << "\n";
//
//       const uint32_t vertexOffset = objects->at(objIndex[1]).vertexOffset;
//       const uint32_t dataCount = objects->at(objIndex[1]).vertexCount + objects->at(objIndex[1]).faceCount + 1;
//
//       for (uint32_t i = vertexOffset; i < vertexOffset + dataCount; ++i) {
//         const glm::vec4 vec = verts->at(i);
//         std::cout << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w << "\n";
//       }
//
//       throw std::runtime_error("NAN");
//     }

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

//     if (transforms->at(transformIndex[1]).pos.x != transforms->at(transformIndex[1]).pos.x) {
//       std::cout << "indices " << objIndex[0] << " " << objIndex[1] << "\n";
//
//       const uint32_t vertexOffset = objects->at(objIndex[1]).vertexOffset;
//       const uint32_t dataCount = objects->at(objIndex[1]).vertexCount + objects->at(objIndex[1]).faceCount + 1;
//
//       for (uint32_t i = vertexOffset; i < vertexOffset + dataCount; ++i) {
//         const glm::vec4 vec = verts->at(i);
//         std::cout << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w << "\n";
//       }
//
//       throw std::runtime_error("NAN");
//     }

    glm::vec4 vel2 = glm::vec4(datas->at(physDataIndex[1]).velocity, 0.0f);
    glm::vec4 vel2Global = velocities->at(physDataIndex[1]);

    clipVelocity(-mtv, bounce, vel2);
    clipVelocity(-mtv, bounce, vel2Global);

    datas->at(physDataIndex[1]).velocity = glm::vec3(vel2.x, vel2.y, vel2.z);// - glm::vec3((mtv * move) / dt);
    velocities->at(physDataIndex[1]) = vel2Global;// - (mtv * move) / dt;

    //lastVelocities[physDataIndex[1]] = -(mtv * move) / dt;
  }
}

// код взят: https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/Geometry3D.cpp
bool testRayBox(const RayData &ray, const FastAABB &box, const glm::mat4 &orn, glm::vec4 &point) {
  const glm::vec4 dir = box.center - ray.pos;

  glm::vec4 f = glm::vec4(
    glm::dot(orn[0], ray.dir),
    glm::dot(orn[1], ray.dir),
    glm::dot(orn[2], ray.dir),
    0.0f);

  const glm::vec4 e = glm::vec4(
    glm::dot(orn[0], dir),
    glm::dot(orn[1], dir),
    glm::dot(orn[2], dir),
    0.0f);

  float t[6];
  for (uint32_t i = 0; i < 3; ++i) {
    if (glm::abs(f[i]) < EPSILON) {
      if (-e[i] - box.extent[i] > 0.0f || -e[i] + box.extent[i] < 0.0f) return false;
      f[i] = EPSILON;
    }

    t[i*2+0] = (e[i] + box.extent[i]) / f[i];
    t[i*2+1] = (e[i] - box.extent[i]) / f[i];
  }

  float minf = glm::max(glm::max(glm::min(t[0], t[1]), glm::min(t[2], t[3])), glm::min(t[4], t[5]));
  float maxf = glm::min(glm::min(glm::max(t[0], t[1]), glm::max(t[2], t[3])), glm::max(t[4], t[5]));

  if (maxf < 0.0f) return false;
  if (minf > maxf) return false;

  float res = minf < 0.0f ? maxf : minf;

  point = ray.pos + ray.dir * res;
  return true;
}

bool testRaySphere(const RayData &ray, const glm::vec4 &sphereCenter, const float &radius, glm::vec4 &point) {
  const glm::vec4 dir = sphereCenter - ray.pos;

  const float eSq = glm::length2(dir);
  const float a = glm::dot(dir, ray.dir);
  const float b = radius*radius - (eSq - a*a);

  if (b < 0.0f) return false;

  const float f = glm::sqrt(abs(b));

  const float t = eSq < radius*radius ? a + f : a - f;

  point = ray.pos + ray.dir*t;
  return true;
}

bool testRayTri(const RayData &ray, const glm::vec4 &vert1, const glm::vec4 &vert2, const glm::vec4 &vert3, glm::vec4 &point) {
  const glm::vec4 v0v1 = vert2 - vert1;
  const glm::vec4 v0v2 = vert3 - vert1;
  const glm::vec4 pvec = glm::vec4(glm::cross(glm::vec3(ray.dir), glm::vec3(v0v2)), 0.0f);
  const float invDet = 1.0f/glm::dot(v0v1, pvec); // скорее всего этот det не совпадает с тем что я вычисляю от нормали и направления

  const glm::vec4 tvec = ray.pos - vert1;
  const float u = glm::dot(tvec, pvec) * invDet;
  if (u < 0.0f || u > 1.0f) return false;

  const glm::vec4 qvec = glm::vec4(glm::cross(glm::vec3(tvec), glm::vec3(v0v1)), 0.0f);
  const float v = glm::dot(ray.dir, qvec) * invDet;
  if (v < 0.0f || u + v > 1.0f) return false;

  const float t = glm::dot(v0v2, qvec) * invDet;

  if (t < 0.0f) return false;

  point = ray.pos + ray.dir * t;

  return true;
}

// тут у меня начинаются проблемы, так как я решил запилить вместо обычного полигона
// меш, да еще так что из него не понятно как расположены треугольники (мне это было не нужно)
// что делать? в принципе обычные полигоны
// (то есть плоские объекты с произвольным количеством точек) обрабатываются нормально
// с другой стороны у меня есть фейсы (то есть просто нормали,
// поидее этого должно быть достаточно для того чтобы чекнуть пересечение,
// хотя и точку можно получить наверное), нет, просто фейсов с точками недостаточно
// нужно понять где находится плоскость
// вообще плоскости + 1 точка, должно быть достаточно чтоб узнать пересечение (для мешей)
// для полигона плоскость и одна точка - недостаточно, но там понятно что к чему
// кажется я нашел как можно сделать: http://adrianboeing.blogspot.com/2010/02/intersection-of-convex-hull-with-line.html
bool testRayPoly(const ArrayInterface<glm::vec4>* verts, const RayData &ray, const uint32_t &vertOffset, const uint32_t &vertSize, const uint32_t &faceSize, const glm::vec4 &pos, const glm::mat4 &orn, glm::vec4 &point) {
  //const mat4 transform = mat4(1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  pos.x, pos.y, pos.z, 1.0);
  const glm::vec4 dir = glm::vec4(glm::vec3(pos), 0.0f);

  if (vertSize+1 == faceSize) { // по идее это полигон
    const float det = glm::dot(glm::vec4(glm::vec3(verts->at(vertOffset+vertSize+1)), 0.0f), ray.dir);
    if (glm::abs(det) < EPSILON) return false;

    //throw std::runtime_error("ray intersect poly");

    // const vec4 refP = transform * orn * vertices[vertOffset];
    //const glm::vec4 refP = orn * (dir + verts->at(vertOffset));
    const glm::vec4 refP = transform(verts->at(vertOffset), dir, orn); //orn * (dir + verts->at(vertOffset));
    for (uint32_t i = vertOffset+1; i < vertOffset+vertSize-1; ++i) {
      // трансформа тут неправильно накладывается походу
//       const glm::vec4 vert2 = orn * (dir + verts->at(i));
//       const glm::vec4 vert3 = orn * (dir + verts->at(i+1));

      const glm::vec4 vert2 = transform(verts->at(i), dir, orn);
      const glm::vec4 vert3 = transform(verts->at(i+1), dir, orn);

      if (testRayTri(ray, refP, vert2, vert3, point)) return true;
    }

    return false;
  }

  float tE = 0.0f;
  float tL = 0.0f;
  for (uint32_t i = vertOffset+vertSize+1; i < vertOffset+vertSize+faceSize; ++i) {
    const glm::vec4 normal = orn * glm::vec4(glm::vec3(verts->at(i)), 0.0f);
    // тут нужно получить вершину, СКОРЕЕ ВСЕГО любую которая лежит на плоскости
    // судя по всему достаточно одной вершины которая лежит на плоскости
    const glm::vec4 vertex = verts->at(vertOffset+glm::floatBitsToUint(verts->at(i).w));
    // трансформа тут неправильно накладывается походу
    const glm::vec4 oneNormalVertex = orn * (dir + vertex);
    const float N = -dot(ray.pos - oneNormalVertex, normal);
    const float D = dot(ray.dir, normal);

    // параллельна ли данная нормаль направлению луча?
    if (abs(D) < EPSILON) {
      if (N < 0.0f) return false; // луч не может пересечь объект
      else continue; // луч параллелен одной из плоскостей, но еще может пересечь объект
    }

    const float t = N / D;
    // луч входит в объект минуя эту плоскость
    if (D < 0.0f) {
      tE = glm::max(tE, t);
      if (tE > tL) return false; // входим дальше чем выходим, объект не может быть пересечен
    }
    // луч выходит из объекта минуя эту плоскость
    else if (D > 0.0f) {
      tL = glm::max(tL, t);
      if (tL < tE) return false; // выходит раньше чем входит, объект не может быть пересечен
    }
  }

  point = ray.pos + ray.dir*tE;
  return true;

  // как быть с полигонами? тип если vertSize+1 == faceSize то это полигон?
  // может ли тут где нибудь быть мина?
}

bool CPUSolverParallel::intersect(const uint32_t &rayIndex, const uint32_t &objIndex, const uint32_t &transformIndex, glm::vec4 &point) const {
  const RayData &ray = rays->at(rayIndex);
  const Object &object = objects->at(objIndex);

  const glm::vec4 pos = transformIndex != 0xFFFFFFFF ? transforms->at(transformIndex).pos : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

  switch(object.objType.getObjType()) {
    case BBOX_TYPE: {
      //std::cout << "Ray intersect " << objIndex << "\n";
      const FastAABB box = {pos, verts->at(object.vertexOffset)};
      return testRayBox(ray, box, systems->at(object.coordinateSystemIndex), point);
    }
    break;
    case SPHERE_TYPE:
      return testRaySphere(ray, glm::vec4(glm::vec3(pos), 1.0f), pos.w, point);
    break;
    case POLYGON_TYPE: {
      //std::cout << "Ray intersect " << objIndex << "\n";
      const glm::mat4 orn = object.rotationDataIndex == 0xFFFFFFFF ?
                              systems->at(object.coordinateSystemIndex) : rotationDatas->at(object.rotationDataIndex).matrix * systems->at(object.coordinateSystemIndex);
      return testRayPoly(verts, ray, object.vertexOffset, object.vertexCount, object.faceCount, pos, orn, point);
    }
    break;
  }

  return false;
}

template <typename T>
uint32_t binarySearch(T* begin, T* end, const uint32_t &firstObjIndex) {
  int low = 0;
  //uint high = overlappingBufferData.x-1;
  int high = int(end-begin)-1;

  while (low <= high) {
    const uint32_t mid = (low + high) / 2;

    const int lowest = int(firstObjIndex < begin[mid].firstIndex);
    const int highest = int(firstObjIndex > begin[mid].firstIndex);

    high -= lowest;
    low += highest;

    if (lowest + highest == 0) return mid;
  }

  return UINT32_MAX;
}

template <typename T>
uint32_t getToStart(T* arr, const uint32_t &firstObjIndex, const uint32_t &foundedIndex) {
  uint32_t retIndex = foundedIndex;
  while (retIndex > 0) {
    if (arr[retIndex-1].firstIndex != firstObjIndex) break;

    --retIndex;
  }

  return retIndex;
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
