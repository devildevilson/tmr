#include "CPUNarrowphaseParallel.h"

#include <stdexcept>

#include <chrono>
#include <iostream>
#include <cstring>

#define ONLY_TRIGGER_ID 0xFFFFFFFF-1

#define FREE_MUTEX 0
#define MUTEX_LOCK_WRITE 1
#define MUTEX_LOCK_READ 2

struct IslandAdditionalData {
  glm::uvec4 octreeDepth;
  glm::uvec4 octreeLevels[10];
  glm::uvec4 islandCount;
};

UniquePairContainer::UniquePairContainer(const size_t &objCount) {
  const size_t size = std::ceil(float(objCount) / float(UINT32_WIDTH));
  container = new uint32_t[size];
  memset(container, 0, size*sizeof(uint32_t));
  pthread_rwlock_init(&rw_lock, NULL);
}

UniquePairContainer::~UniquePairContainer() {
  delete [] container;
  pthread_rwlock_destroy(&rw_lock);
}

bool UniquePairContainer::write(const uint32_t &first, const uint32_t &second) {
  bool res;
  
  const uint32_t firstContainerIndex = first / float(UINT32_WIDTH);
  const uint32_t firstBitMask = 1 << (first % UINT32_WIDTH);
  
  const uint32_t secondContainerIndex = second / float(UINT32_WIDTH);
  const uint32_t secondBitMask = 1 << (second % UINT32_WIDTH);
  
//   std::unique_lock<std::mutex> lock(mutex);
//   writeVar.wait(lock);
  
  // бред какой-то, почему похожей операции нет в плюсах? есть я не правильно понял, нужно получше посмотреть
//   uint32_t value = FREE_MUTEX;
//   while (!atomicMutex.compare_exchange_weak(value, uint32_t(MUTEX_LOCK_WRITE))); // == MUTEX_LOCK_WRITE
  // вот только оно возвращает булево значение, вместо предыдущего, и следовательно лучше использовать нормальный мьютекс для тех же целей
  
  // для этих целей можно использовать pthread, его скорее всего будет достаточно
  // оставить в таком виде? или лучше убрать rw_lock внутрь? (придетсся выделять память для него каждый кадр, а потом чистить)
  // впрочем память мы выделяем для контейнера, потом нужно наверное переделать так чтобы не перевыделять память каждый кадр (потому что не нужно)
  pthread_rwlock_wrlock(&rw_lock);
  
  const bool hasFirst  = (container[firstContainerIndex]  & firstBitMask)  == firstBitMask;
  const bool hasSecond = (container[secondContainerIndex] & secondBitMask) == secondBitMask;
  
  if (!hasFirst && !hasSecond) {
    res = true;
    container[firstContainerIndex] |= firstBitMask;
    container[secondContainerIndex] |= secondBitMask;
  } else {
    res = false;
  }
  
//   readVar.notify_all();
//   atomicMutex.exchange(FREE_MUTEX);
  pthread_rwlock_unlock(&rw_lock);
  
  return res;
}

bool UniquePairContainer::read(const uint32_t &first, const uint32_t &second) {
  bool res;
  
  const uint32_t firstContainerIndex = first / float(UINT32_WIDTH);
  const uint32_t firstBitMask = 1 << (first % UINT32_WIDTH);
  
  const uint32_t secondContainerIndex = second / float(UINT32_WIDTH);
  const uint32_t secondBitMask = 1 << (second % UINT32_WIDTH);
  
  // с использованием условных переменных пахнет деадлоком
//   {
//     std::unique_lock<std::mutex> lock(mutex);
//     readVar.wait(lock);
//   }
  
  pthread_rwlock_rdlock(&rw_lock);
  
  const bool hasFirst  = (container[firstContainerIndex]  & firstBitMask)  == firstBitMask;
  const bool hasSecond = (container[secondContainerIndex] & secondBitMask) == secondBitMask;
  
  res = hasFirst && hasSecond;
  
  pthread_rwlock_unlock(&rw_lock);
  
//   {
//     std::unique_lock<std::mutex> lock(mutex);
//     writeVar.notify_one();
//   }
  return res;
}

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
  static const auto batching = [&] (const size_t &start, const size_t &count, const size_t &batchId, UniquePairContainer* cont, std::atomic<size_t> &pairsCount) {
    for (size_t i = start; i < start+count; ++i) {
      const size_t index = i+1;
      
      const auto &pair = pairs->at(index);
      if (pair.islandIndex != UINT32_MAX) continue;
      
      const bool hasPair = cont->read(pair.firstIndex, pair.secondIndex);
      if (!hasPair) {
        if (cont->write(pair.firstIndex, pair.secondIndex)) {
          pairs->at(index).islandIndex = batchId;
          pairsCount.fetch_add(-1);
        }
      }
    }
  };
  
  if (pairs->at(0).secondIndex < 2) return;
  
  // для нормальной работы, нужно чтобы pair.islandIndex были все проставлены UINT32_MAX
  // сейчас мы вручную это сделаем, нужно будет потом убрать
  for (size_t i = 0; i < pairs->at(0).secondIndex; ++i) {
    const size_t index = i+1;
    pairs->at(index).islandIndex = UINT32_MAX;
  }
  
  size_t batchId = 0;
  const size_t pairSize = pairs->at(0).secondIndex;
  std::atomic<size_t> pairsCount(pairSize);
  const size_t count = glm::ceil(float(pairSize) / float(pool->size()+1));
  UniquePairContainer pairCont(pairSize);
  
  while (pairsCount > 0) {
    size_t start = 0;
    for (uint32_t i = 0; i < pool->size()+1; ++i) {
      const size_t jobCount = std::min(count, pairSize-start);
      if (jobCount == 0) break;

      pool->submitnr(batching, start, jobCount, batchId, &pairCont, std::ref(pairsCount));

      start += jobCount;
    }
    
    pool->compute();
    pool->wait();
    
    ++batchId;
  }
}

void CPUNarrowphaseParallel::checkIdenticalPairs() {
  // здесь мне нужно посчитать одинаковые пары ТОЛЬКО в динамических парах
  // мне неважно в каком порядке индексы указаны в самой паре
  // а значит я могу прежде отсортировать их по индексам
  // затем пройти итеративно и отметить повторяющиеся пары
  // как пройти их параллельно? параллельно обрабатывать разные первые индексы пары?
  // можно ли это пройти вообще параллельно?

  if (pairs->at(0).firstIndex < 1) return;

  // RegionLog rl("CPUNarrowphaseParallel::checkIdenticalPairs()");

  uint32_t uniqueFirstIndex = UINT32_MAX;
  uint32_t uniqueSecondIndex = UINT32_MAX;
  
  size_t pairsCount = pairs->at(0).firstIndex;
  for (uint32_t i = 0; i < pairsCount; ++i) {
    const uint32_t index = i+1;
    
    if (uniqueFirstIndex == pairs->at(index).firstIndex && uniqueSecondIndex == pairs->at(index).secondIndex) {
      pairs->at(index).islandIndex = UINT32_MAX;
    } else {
      uniqueFirstIndex = pairs->at(index).firstIndex;
      uniqueSecondIndex = pairs->at(index).secondIndex;
    }
    
//     if (uniqueFirstIndex != pairs->at(index).firstIndex) {
//       uniqueFirstIndex = pairs->at(index).firstIndex;
//       uniqueSecondIndex = pairs->at(index).secondIndex;
//     } else {
//       pairs->at(index).islandIndex = uniqueSecondIndex == pairs->at(index).secondIndex ? UINT32_MAX : pairs->at(index).islandIndex;
//       uniqueSecondIndex = uniqueSecondIndex == pairs->at(index).secondIndex ? uniqueSecondIndex : pairs->at(index).secondIndex;
//     }

    if (pairs->at(index).islandIndex == ONLY_TRIGGER_ID) pairs->at(0).dist = glm::uintBitsToFloat(glm::floatBitsToUint(pairs->at(0).dist) + 1);
    if (pairs->at(index).islandIndex <  ONLY_TRIGGER_ID) ++pairs->at(0).secondIndex;
  }
}

void CPUNarrowphaseParallel::postCalculation() {
  static const auto calcIslandCount = [&] (const ArrayInterface<BroadphasePair>* constPairs, const uint32_t &valPairCount, bool dynamic, IslandAdditionalData* data, IslandData* islandsPtr) {
    uint32_t islandCountLocal = uint32_t(valPairCount > 0);
    uint32_t pairIslandIndex = constPairs->at(1).islandIndex;
    for (uint32_t i = 0; i < valPairCount; ++i) {
      const uint32_t index = i+1;
      
      if (constPairs->at(index).islandIndex == pairIslandIndex) {
        const uint32_t islandIndex = islandCountLocal-1;

        ++islandsPtr[islandIndex].size;
        islandsPtr[islandIndex].islandIndex = constPairs->at(index).islandIndex;
        continue;
      }

      ++islandCountLocal;
      pairIslandIndex = constPairs->at(index).islandIndex;

      const uint32_t islandIndex = islandCountLocal-1;
      ++islandsPtr[islandIndex].size;
      islandsPtr[islandIndex].islandIndex = constPairs->at(index).islandIndex;
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

  // RegionLog rl("CPUNarrowphaseParallel::postCalculation()");

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
