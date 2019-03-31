#include "CPUNarrowphaseParallel.h"

#include <stdexcept>

#include <chrono>
#include <iostream>
#include <cstring>

#define ONLY_TRIGGER_ID 0xFFFFFFFF-1

struct IslandAdditionalData {
  glm::uvec4 octreeDepth;
  glm::uvec4 octreeLevels[10];
  glm::uvec4 islandCount;
};

CPUNarrowphaseParallel::CPUNarrowphaseParallel(dt::thread_pool* pool, const uint32_t &octreeDepth) {
  this->pool = pool;
  this->octreeDepth = octreeDepth;

  octreeLevels.resize(10);

  uint32_t tmp = 0;
  for (uint32_t i = 0; i < octreeDepth; ++i) {
    const uint32_t powEight = glm::pow(8, octreeDepth - i - 1);
    tmp += powEight;

    octreeLevels[i].x = tmp;
    octreeLevels[i].y = UINT32_MAX;
    octreeLevels[i].z = 0;
    octreeLevels[i].w = 0;
  }
}

CPUNarrowphaseParallel::~CPUNarrowphaseParallel() {}

void CPUNarrowphaseParallel::setInputBuffers(const InputBuffers &inputs, void* indirectPairCount) {
  (void)indirectPairCount;

  this->pairs = inputs.dynPairs;
  this->staticPairs = inputs.statPairs;
}

void CPUNarrowphaseParallel::setOutputBuffers(const OutputBuffers &outputs, void* indirectIslandCount) {
  (void)indirectIslandCount;

  this->islands = outputs.dynIslands;
  this->staticIslands = outputs.statIslands;

  islands->resize(1000);
  staticIslands->resize(1000);
}

void CPUNarrowphaseParallel::updateBuffers(const uint32_t &lastPairCount, const uint32_t &lastStaticPairCount) {
  if (islands->size() < 12 + lastPairCount*1.5f) {
    islands->resize(12 + lastPairCount*1.5f);
  }

  if (staticIslands->size() < 12 + lastStaticPairCount*1.5f) {
    staticIslands->resize(12 + lastStaticPairCount*1.5f);
  }
}

void CPUNarrowphaseParallel::calculateIslands() {
  throw std::runtime_error("Not implemented yet");
}

void CPUNarrowphaseParallel::calculateBatches() {
  throw std::runtime_error("Not implemented yet");
}

void CPUNarrowphaseParallel::checkIdenticalPairs() {
  // здесь мне нужно посчитать одинаковые пары ТОЛЬКО в динамических парах
  // мне неважно в каком порядке индексы указаны в самой паре
  // а значит я могу прежде отсортировать их по индексам
  // затем пройти итеративно и отметить повторяющиеся пары
  // как пройти их параллельно? параллельно обрабатывать разные первые индексы пары?
  // можно ли это пройти вообще параллельно?

  if (pairs->at(0).firstIndex < 1) return;

  RegionLog rl("CPUNarrowphaseParallel::checkIdenticalPairs()");

  uint32_t uniqueFirstIndex = UINT32_MAX;
  uint32_t uniqueSecondIndex = UINT32_MAX;
  for (uint32_t i = 1; i < pairs->at(0).firstIndex+1; ++i) {

    if (uniqueFirstIndex != pairs->at(i).firstIndex) {
      uniqueFirstIndex = pairs->at(i).firstIndex;
      uniqueSecondIndex = pairs->at(i).secondIndex;
    } else {
      pairs->at(i).islandIndex = uniqueSecondIndex == pairs->at(i).secondIndex ? UINT32_MAX : pairs->at(i).islandIndex;
      uniqueSecondIndex = uniqueSecondIndex == pairs->at(i).secondIndex ? uniqueSecondIndex : pairs->at(i).secondIndex;
    }

    if (pairs->at(i).islandIndex == ONLY_TRIGGER_ID) pairs->at(0).dist = glm::uintBitsToFloat(glm::floatBitsToUint(pairs->at(0).dist) + 1);
    if (pairs->at(i).islandIndex <  ONLY_TRIGGER_ID) ++pairs->at(0).secondIndex;
  }
}

void CPUNarrowphaseParallel::postCalculation() {
  static const auto calcIslandCount = [&] (const ArrayInterface<BroadphasePair>* constPairs, const uint32_t &valPairCount, bool dynamic, IslandAdditionalData* data, IslandData* islandsPtr) {
    uint32_t islandCountLocal = uint32_t(valPairCount > 0);
    uint32_t pairIslandIndex = constPairs->at(1).islandIndex;
    for (uint32_t i = 1; i < valPairCount+1; ++i) {
      if (constPairs->at(i).islandIndex == pairIslandIndex) {
        const uint32_t islandIndex = islandCountLocal-1;

        ++islandsPtr[islandIndex].size;
        islandsPtr[islandIndex].islandIndex = constPairs->at(i).islandIndex;
        continue;
      }

      ++islandCountLocal;
      pairIslandIndex = constPairs->at(i).islandIndex;

      const uint32_t islandIndex = islandCountLocal-1;
      ++islandsPtr[islandIndex].size;
      islandsPtr[islandIndex].islandIndex = constPairs->at(i).islandIndex;
    }

    data->islandCount.x = islandCountLocal;

    uint32_t newOffset = 0;
    for (uint32_t i = 0; i < islandCountLocal; ++i) {
      islandsPtr[i].offset = newOffset;
      newOffset += islandsPtr[i].size;
    }

    if (dynamic) {
      for (uint32_t i = 0; i < islandCountLocal; ++i) {
        for (uint32_t j = 0; j < this->octreeDepth; ++j) {
          if (islandsPtr[i].islandIndex < data->octreeLevels[j].x) {
            ++data->octreeLevels[j].z;
            break;
          }
        }
      }

      newOffset = 0;
      for (uint32_t i = 0; i < this->octreeDepth; ++i) {
        data->octreeLevels[i].y = newOffset;
        newOffset += data->octreeLevels[i].z;
      }
    }
  };

  RegionLog rl("CPUNarrowphaseParallel::postCalculation()");

  // имеет ли смысл тут параллелить на тяжелых тредах цпу?
  // тут можно раскидать задачи по тредам и с помощью фьючеров синхронизировать
  // ну вообще в этом тоже не особо много смысла, зато параллелим вычисления островов для статиков и динамиков

  // здесь фьючеры выходят из скоупа и умирают
  if (pairs->at(0).secondIndex > 0) {
    const auto constPairs = pairs;

    IslandAdditionalData* data = islands->structure_from_begin<IslandAdditionalData>();
    IslandData* islandsPtr = islands->data_from<IslandAdditionalData>();

    data->octreeDepth.x = octreeDepth;
    memcpy(data->octreeLevels, octreeLevels.data(), 10*sizeof(glm::uvec4));
    memset(islandsPtr, 0, islands->size() * sizeof(IslandData) - sizeof(IslandAdditionalData));

//     for (uint32_t i = 0; i < octreeDepth; ++i) {
//       std::cout << "data size " << data->octreeLevels[i].z << " default size " << octreeLevels[i].z << "\n";
//       if (data->octreeLevels[i].z > 0) throw std::runtime_error("wrong init level " + std::to_string(i));
//     }

    data->islandCount.x = 0;

    const uint32_t valPairCount = constPairs->at(0).secondIndex;

    pool->submitnr(calcIslandCount, constPairs, valPairCount, true, data, islandsPtr);

//     uint32_t islandCountLocal = uint32_t(valPairCount > 0);
//
//     auto future1 = pool->submit([&] () {
//       uint32_t pairIslandIndex = constPairs->at(1).islandIndex;
//       for (uint32_t i = 1; i < valPairCount+1; ++i) {
//         if (constPairs->at(i).islandIndex == pairIslandIndex) continue;
//
//         ++islandCountLocal;
//         pairIslandIndex = constPairs->at(i).islandIndex;
//       }
//
//       data->islandCount.x = islandCountLocal;
//     });
//
//     auto future2 = pool->submit([&] () {
//       future1.get();
//
//       for (uint32_t i = 0; i < islandCountLocal; ++i) {
//         islandsPtr[i].islandIndex = UINT32_MAX;
//         islandsPtr[i].offset = UINT32_MAX;
//         islandsPtr[i].size = 0;
//       }
//     });
//
//     auto future3 = pool->submit([&] () {
//       future2.get();
//
//       uint32_t indexIsland = 0;
//       for (uint32_t i = 1; i < constPairs->at(0).secondIndex+1; ++i) {
//         uint32_t pairIslandIndex = constPairs->at(1).islandIndex;
//         for (uint32_t j = 1; j < i; ++j) {
//           if (constPairs->at(j).islandIndex == pairIslandIndex) continue;
//
//           ++indexIsland;
//           pairIslandIndex = constPairs->at(j).islandIndex;
//         }
//
//         ++islandsPtr[indexIsland].size;
//         islandsPtr[indexIsland].islandIndex = glm::min(islandsPtr[indexIsland].islandIndex, constPairs->at(i).islandIndex);
//         islandsPtr[indexIsland].offset = glm::min(islandsPtr[indexIsland].offset, i-1);
//       }
//     });
//
//     auto future4 = pool->submit([&] () {
//       for (uint32_t i = 0; i < this->octreeDepth; ++i) {
//         octreeLevels[i].y = UINT32_MAX;
//         octreeLevels[i].z = 0;
//       }
//     });
//
//     auto future5 = pool->submit([&] () {
//       future3.get();
//       future4.get();
//
//       for (uint32_t i = 0; i < islandCountLocal; ++i) {
//         for (uint32_t j = 0; j < this->octreeDepth; ++j) {
//           if (islandsPtr[i].islandIndex < octreeLevels[j].x) {
//             ++octreeLevels[j].z;
//             break;
//           }
//         }
//       }
//     });
//
//     pool->submitnr([&] () {
//       future5.get();
//
//       uint32_t newOffset = 0;
//       for (uint32_t i = 0; i < this->octreeDepth; ++i) {
//         octreeLevels[i].y = newOffset;
//         newOffset += octreeLevels[i].z;
//       }
//     });
  }

  {
    const auto constPairs = staticPairs;

    IslandAdditionalData* data = staticIslands->structure_from_begin<IslandAdditionalData>();
    IslandData* islandsPtr = staticIslands->data_from<IslandAdditionalData>();

    memset(islandsPtr, 0, islands->size() * sizeof(IslandData) - sizeof(IslandAdditionalData));

    data->islandCount.x = 0;

    const uint32_t valPairCount = constPairs->at(0).firstIndex;

    pool->submitnr(calcIslandCount, constPairs, valPairCount, false, data, islandsPtr);

//     uint32_t islandCountLocal = uint32_t(valPairCount > 0);
//
//     auto future1 = pool->submit([&] () {
//       uint32_t pairIslandIndex = constPairs->at(1).islandIndex;
//       for (uint32_t i = 1; i < valPairCount+1; ++i) {
//         if (constPairs->at(i).islandIndex == pairIslandIndex) continue;
//
//         ++islandCountLocal;
//         pairIslandIndex = constPairs->at(i).islandIndex;
//       }
//
//       data->islandCount.x = islandCountLocal;
//     });
//
//     auto future2 = pool->submit([&] () {
//       future1.get();
//
//       for (uint32_t i = 0; i < islandCountLocal; ++i) {
//         islandsPtr[i].islandIndex = UINT32_MAX;
//         islandsPtr[i].offset = UINT32_MAX;
//         islandsPtr[i].size = 0;
//       }
//     });
//
//     pool->submitnr([&] () {
//       future2.get();
//
//       uint32_t indexIsland = 0;
//       for (uint32_t i = 1; i < valPairCount+1; ++i) {
//         uint32_t pairIslandIndex = constPairs->at(1).islandIndex;
//         for (uint32_t j = 1; j < i; ++j) {
//           if (constPairs->at(j).islandIndex == pairIslandIndex) continue;
//
//           ++indexIsland;
//           pairIslandIndex = constPairs->at(j).islandIndex;
//         }
//
//         ++islandsPtr[indexIsland].size;
//         islandsPtr[indexIsland].islandIndex = glm::min(islandsPtr[indexIsland].islandIndex, constPairs->at(i).islandIndex);
//         islandsPtr[indexIsland].offset = glm::min(islandsPtr[indexIsland].offset, i-1);
//       }
//     });
  }

  pool->compute();
  pool->wait();

//   IslandAdditionalData* data = islands->structure_from_begin<IslandAdditionalData>();
//   IslandData* islandsPtr = islands->data_from<IslandAdditionalData>();
//
//   for (uint32_t i = 0; i < data->octreeDepth.x; ++i) {
//     std::cout << "Level " << i << " offset " << data->octreeLevels[i].y << " size " << data->octreeLevels[i].z << "\n";
//   }
}

void CPUNarrowphaseParallel::printStats() {
  std::cout << "cpu narrowphase parallel" << "\n";
  // тут нужно еще придумать че вообще выводить
}
