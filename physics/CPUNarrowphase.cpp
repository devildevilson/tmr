#include "CPUNarrowphase.h"

#include <iostream>
#include <chrono>
#include <cstring>

#define ONLY_TRIGGER_ID 0xFFFFFFFF-1

struct IslandAdditionalData {
  glm::uvec4 octreeDepth;
  glm::uvec4 octreeLevels[10];
  glm::uvec4 islandCount;
};

CPUNarrowphase::CPUNarrowphase(const uint32_t &octreeDepth) {
  //islands.resize(1000);
  octreeLevels.resize(10);
  this->octreeDepth = octreeDepth;
  
  uint32_t tmp = 0;
  for (uint32_t i = 0; i < octreeDepth; ++i) {
    const uint32_t powEight = glm::pow(8, octreeDepth - i - 1);
    tmp += powEight;
    
    octreeLevels[i].x = tmp;
    octreeLevels[i].y = 0;
    octreeLevels[i].z = 0;
    octreeLevels[i].w = 0;
  }
  
  // НУЖНО СЛЕДИТЬ ЧТОБЫ В БУФЕРАХ ОСТРОВОВ БЫЛИ ВСЕГДА НУЖНЫЕ ДАННЫЕ О КОЛИЧЕСТВЕ НОДОВ НА УРОВНЯХ!!!
}

CPUNarrowphase::~CPUNarrowphase() {}

// void CPUNarrowphase::setPairBuffer(ArrayInterface<BroadphasePair>* buffer, void* indirectPairCount) {
//   (void)indirectPairCount;
//   pairs = buffer;
// }

void CPUNarrowphase::setInputBuffers(const InputBuffers &inputs, void* indirectPairCount) {
  (void)indirectPairCount;
  
  this->pairs = inputs.dynPairs;
  this->staticPairs = inputs.statPairs;
}

void CPUNarrowphase::setOutputBuffers(const OutputBuffers &outputs, void* indirectIslandCount) {
  (void)indirectIslandCount;
  
  this->islands = outputs.dynIslands;
  this->staticIslands = outputs.statIslands;
  
  islands->resize(1000);
  staticIslands->resize(1000);
}

void CPUNarrowphase::updateBuffers(const uint32_t &lastPairCount, const uint32_t &lastStaticPairCount) {
  if (islands->size() < 12 + lastPairCount*1.5f) {
    islands->resize(12 + lastPairCount*1.5f);
  }
  
  if (staticIslands->size() < 12 + lastStaticPairCount*1.5f) {
    staticIslands->resize(12 + lastStaticPairCount*1.5f);
  }
}

void CPUNarrowphase::calculateIslands() {
  auto start = std::chrono::steady_clock::now();

  uint32_t islandId = 0;
  uint32_t changesCount = UINT32_MAX;
  pairs->at(0).secondIndex = 0;
  pairs->at(0).dist = glm::uintBitsToFloat(0);

  for (uint32_t i = 1; i < pairs->at(0).firstIndex+1; ++i) {
    if (pairs->at(i).islandIndex >= ONLY_TRIGGER_ID) continue;
    const uint32_t id = islandId;
    ++islandId;

    pairs->at(i).islandIndex = id;
  }

  uint32_t iteration = 0;
  while (changesCount > 0 && iteration < iterationCount) {
    changesCount = 0;

    for (uint32_t i = 1; i < pairs->at(0).firstIndex+1; ++i) {
      const uint32_t index = i;
      const uint32_t firstIndex = pairs->at(index).firstIndex;
      const uint32_t secondIndex = pairs->at(index).secondIndex;
      uint32_t minIslandIndex = pairs->at(index).islandIndex;

      if (minIslandIndex < ONLY_TRIGGER_ID && firstIndex != UINT32_MAX && secondIndex != UINT32_MAX) {
        for (uint32_t j = 1; j < pairs->at(0).firstIndex+1; ++j) {
          if (index == j) continue;

          const bool samePair = (firstIndex == pairs->at(j).firstIndex && secondIndex == pairs->at(j).secondIndex) ||
                                (firstIndex == pairs->at(j).secondIndex && secondIndex == pairs->at(j).firstIndex);

          if (samePair && index > j) {
            minIslandIndex = UINT32_MAX;
            changesCount += 1;
            break;
          }

          const uint32_t currentIndex = pairs->at(j).islandIndex;
          minIslandIndex = glm::min(minIslandIndex, currentIndex);
          if (minIslandIndex != currentIndex) changesCount += 1;
        }
      }

      pairs->at(index).islandIndex = minIslandIndex;
    }

    ++iteration;
  }

  for (uint32_t i = 1; i < pairs->at(0).firstIndex+1; ++i) {
    if (pairs->at(i).islandIndex == UINT32_MAX) continue;

    if (pairs->at(i).islandIndex == ONLY_TRIGGER_ID) {
      pairs->at(0).dist = glm::uintBitsToFloat(glm::floatBitsToUint(pairs->at(0).dist) + 1);
      continue;
    }

    ++pairs->at(0).secondIndex;
  }
  
//   std::cout << "Valuable pairs " << pairs->at(0).secondIndex << "\n";
  if (pairs->at(0).firstIndex != 0 && pairs->at(0).secondIndex == 0) {
    throw std::runtime_error("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa");
  }

  auto end = std::chrono::steady_clock::now() - start;
  auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
//   std::cout << "Island calc time : " << mcs << " mcs" << '\n';
}

void CPUNarrowphase::calculateBatches() {
  // батчи у меня пока не работают =(
  throw std::runtime_error("Not implemented yet");
}

void CPUNarrowphase::checkIdenticalPairs() {
  //throw std::runtime_error("Not implemented yet");
  
  // здесь мне нужно посчитать одинаковые пары ТОЛЬКО в динамических парах
  // мне неважно в каком порядке индексы указаны в самой паре
  // а значит я могу прежде отсортировать их по индексам
  // затем пройти итеративно и отметить повторяющиеся пары
  // как пройти их параллельно? параллельно обрабатывать разные первые индексы пары?
  // можно ли это пройти вообще параллельно?
  
  if (pairs->at(0).firstIndex < 1) return;
  
//   std::cout << "pairs count " << pairs->at(0).firstIndex << "\n";
  
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
  
//   std::cout << "val pairs count     " << pairs->at(0).secondIndex << "\n";
//   std::cout << "trigger pairs count " << glm::floatBitsToUint(pairs->at(0).dist) << "\n";
}

void CPUNarrowphase::postCalculation() {
  auto start = std::chrono::steady_clock::now();

  //assert(pairs->at(0).secondIndex != 0);

  // всего у меня например 585 нодов
  // мне нужно их распределить по уровням
  // то есть уровень первый (0) это до 512
  // второй 64
  // третий 8
  // и четверый 1
  // ну и мне тип надо пройтись по островам и просто поплюсовать количество

  //islands[0].islandIndex = 0;

  // uint32_t indexIsland = 0;
  // for (uint32_t i = 1; i < pairs->at(0).secondIndex+1; ++i) {
  //   uint32_t pairIslandIndex = pairs->at(1).islandIndex;
  //   for (uint32_t j = 1; j < i; ++j) {
  //     if (pairs->at(j).islandIndex == pairIslandIndex) continue;

  //     ++indexIsland;
  //     pairIslandIndex = pairs->at(j).islandIndex;
  //   }

  //   ++islands[indexIsland+1].size;
  //   islands[indexIsland+1].islandIndex = glm::min(islands[indexIsland+1].islandIndex, pairs->at(i).islandIndex);
  //   islands[indexIsland+1].offset = glm::min(islands[indexIsland+1].offset, i);

  //   const uint32_t islandSize = indexIsland+1;
  //   islands[0].islandIndex = glm::max(islands[0].islandIndex, islandSize);
  // }

  // это еще вполне можно переделать!!!
  
  // нужно будет потом убрать этот костыль (убрал)
  if (pairs->at(0).secondIndex > 0) {
//     glm::uvec4* octreeDepth = nullptr;
//     glm::uvec4* octreeLevels = nullptr;
//     glm::uvec4* islandCount = nullptr;
//     IslandData* islandsPtr = nullptr;
//     
//     IslandData* ptr = islands->data();
//     octreeDepth = reinterpret_cast<glm::uvec4*>(ptr);
//     ptr++;
//     octreeLevels = reinterpret_cast<glm::uvec4*>(ptr);
//     ptr += 10;
//     islandCount = reinterpret_cast<glm::uvec4*>(ptr);
//     ptr++;
//     islandsPtr = reinterpret_cast<IslandData*>(ptr);
    
    IslandAdditionalData* data = islands->structure_from_begin<IslandAdditionalData>();
    IslandData* islandsPtr = islands->data_from<IslandAdditionalData>();
    
    data->octreeDepth.x = this->octreeDepth;
    memcpy(data->octreeLevels, this->octreeLevels.data(), 10*sizeof(glm::uvec4));
    
    data->islandCount.x = 0;

    const uint32_t valPairCount = pairs->at(0).secondIndex;
    uint32_t islandCountLocal = uint32_t(pairs->at(0).secondIndex > 0);
    uint32_t pairIslandIndex = pairs->at(1).islandIndex;
    for (uint32_t i = 1; i < valPairCount+1; ++i) {
      if (pairs->at(i).islandIndex == pairIslandIndex) continue;

      ++islandCountLocal;
      pairIslandIndex = pairs->at(i).islandIndex;
    }

    data->islandCount.x = islandCountLocal;

    for (uint32_t i = 0; i < islandCountLocal; ++i) {
      islandsPtr[i].islandIndex = UINT32_MAX;
      islandsPtr[i].offset = UINT32_MAX;
      islandsPtr[i].size = 0;
    }

    uint32_t indexIsland = 0;
    for (uint32_t i = 1; i < pairs->at(0).secondIndex+1; ++i) {
      uint32_t pairIslandIndex = pairs->at(1).islandIndex;
      for (uint32_t j = 1; j < i; ++j) {
        if (pairs->at(j).islandIndex == pairIslandIndex) continue;

        ++indexIsland;
        pairIslandIndex = pairs->at(j).islandIndex;
      }

      ++islandsPtr[indexIsland].size;
      islandsPtr[indexIsland].islandIndex = glm::min(islandsPtr[indexIsland].islandIndex, pairs->at(i).islandIndex);
      islandsPtr[indexIsland].offset = glm::min(islandsPtr[indexIsland].offset, i-1);
    }
    
    for (uint32_t i = 0; i < this->octreeDepth; ++i) {
      octreeLevels[i].y = UINT32_MAX;
      octreeLevels[i].z = 0;
    }

    for (uint32_t i = 0; i < islandCountLocal; ++i) {
//       std::cout << "island size   " << islandsPtr[i].size << "\n";
//       std::cout << "island offset " << islandsPtr[i].offset << "\n";
//       std::cout << "island index  " << islandsPtr[i].islandIndex << "\n";
      
      for (uint32_t j = 0; j < this->octreeDepth; ++j) {
        if (islandsPtr[i].islandIndex < octreeLevels[j].x) {
          ++octreeLevels[j].z;
          break;
        }
      }
    }
    
    uint32_t newOffset = 0;
    for (uint32_t i = 0; i < this->octreeDepth; ++i) {
      octreeLevels[i].y = newOffset;
      newOffset += octreeLevels[i].z;
    }
  }
  
  {
    //assert(staticPairs->at(0).firstIndex == 0);
    
    IslandAdditionalData* data = staticIslands->structure_from_begin<IslandAdditionalData>();
    IslandData* islandsPtr = staticIslands->data_from<IslandAdditionalData>();
    
    data->islandCount.x = 0;

    const uint32_t valPairCount = staticPairs->at(0).firstIndex;
    uint32_t islandCountLocal = uint32_t(valPairCount > 0);
    uint32_t pairIslandIndex = staticPairs->at(1).islandIndex;
    for (uint32_t i = 1; i < valPairCount+1; ++i) {
      if (staticPairs->at(i).islandIndex == pairIslandIndex) continue;

      ++islandCountLocal;
      pairIslandIndex = staticPairs->at(i).islandIndex;
    }

    data->islandCount.x = islandCountLocal;

    for (uint32_t i = 0; i < islandCountLocal; ++i) {
      islandsPtr[i].islandIndex = UINT32_MAX;
      islandsPtr[i].offset = UINT32_MAX;
      islandsPtr[i].size = 0;
    }

    uint32_t indexIsland = 0;
    for (uint32_t i = 1; i < valPairCount+1; ++i) {
      uint32_t pairIslandIndex = staticPairs->at(1).islandIndex;
      for (uint32_t j = 1; j < i; ++j) {
        if (staticPairs->at(j).islandIndex == pairIslandIndex) continue;

        ++indexIsland;
        pairIslandIndex = staticPairs->at(j).islandIndex;
      }

      ++islandsPtr[indexIsland].size;
      islandsPtr[indexIsland].islandIndex = glm::min(islandsPtr[indexIsland].islandIndex, staticPairs->at(i).islandIndex);
      islandsPtr[indexIsland].offset = glm::min(islandsPtr[indexIsland].offset, i-1);
    }
  }
  
//   if (pairs->at(0).secondIndex != 0 && islands[0].islandIndex == 0) {
//     throw std::runtime_error("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaaa");
//   }
  
//   std::cout << "island count " << islands[0].islandIndex << "\n";

  auto end = std::chrono::steady_clock::now() - start;
  auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
//   std::cout << "Post calc time : " << mcs << " mcs" << '\n';
}

// ArrayInterface<IslandData>* CPUNarrowphase::getIslandDataBuffer() {
//   return &islands;
// }
// 
// const ArrayInterface<IslandData>* CPUNarrowphase::getIslandDataBuffer() const {
//   return &islands;
// }

void CPUNarrowphase::printStats() {
  std::cout << '\n';
  std::cout << "Cpu narrowphase data" << '\n';
  std::cout << "Additional data size " << octreeLevels.vector().capacity() * sizeof(islands[0]) << " bytes" << '\n';
  std::cout << "Narrowphase class size " << sizeof(CPUNarrowphase) << " bytes" << '\n';
}
