#include "CPUOctreeBroadphaseParallel.h"

#include <iostream>
#include <cstring>
#include <chrono>

//#include <glm/gtx/norm.hpp>

#define PRINT_VAR(name, var) std::cout << name << ": " << var << "\n";
#define PRINT_VEC4(name, var) std::cout << name << " x: " << var.x << " y: " << var.y << " z: " << var.z << " w: " << var.w << "\n";

#define OUTSIDE 0
#define INSIDE 1
#define INTERSECT 2

#define ONLY_TRIGGER_ID 0xFFFFFFFF-1

#include <HelperFunctions.h>

CPUOctreeProxyParallel::CPUOctreeProxyParallel(const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter) :
                  BroadphaseProxy(objIndex, type, collisionGroup, collisionFilter) {
  nodeIndex = UINT32_MAX;
  containerIndex = UINT32_MAX-1;
  //objType = type;

  dummy2 = UINT32_MAX;
}

CPUOctreeProxyParallel::~CPUOctreeProxyParallel() {}

uint32_t CPUOctreeProxyParallel::getProxyIndex() const {
  return proxyIndex;
}

uint32_t CPUOctreeProxyParallel::getNodeIndex() const {
  return nodeIndex;
}

uint32_t CPUOctreeProxyParallel::getContainerIndex() const {
  return containerIndex;
}

void CPUOctreeProxyParallel::setNodeIndex(const uint32_t &index) {
  nodeIndex = index;
}

void CPUOctreeProxyParallel::setContainerIndex(const uint32_t &index) {
  containerIndex = index;
}

void CPUOctreeProxyParallel::updateAABB() {

}

// FastAABB CPUOctreeBroadphaseParallel::CPUParallelOctreeNode::getAABB() const {
//   return box;
// }

void CPUOctreeBroadphaseParallel::CPUParallelOctreeNode::add(CPUOctreeProxyParallel* proxy) {
  //basePtr->mutexes[index]
  std::unique_lock<std::mutex> lock(mutex);

  if (!freeIndices.empty()) {
    const uint32_t index = freeIndices.back();
    freeIndices.pop_back();

    proxy->setContainerIndex(index);
    proxy->setNodeIndex(nodeIndex);
    proxies[index] = proxy->getProxyIndex();
  } else {
    proxy->setContainerIndex(proxies.size());
    proxy->setNodeIndex(nodeIndex);
    proxies.push_back(proxy->getProxyIndex());
  }
}

void CPUOctreeBroadphaseParallel::CPUParallelOctreeNode::remove(CPUOctreeProxyParallel* proxy) {
  std::unique_lock<std::mutex> lock(mutex);

  const uint32_t index = proxy->getContainerIndex();
  if (proxies.size() <= index) std::cout << "proxies.size() " << proxies.size() << " index " << index << "\n";
  ASSERT(proxies.size() > index);
  proxies[index] = UINT32_MAX;
  freeIndices.push_back(proxy->getContainerIndex());

  proxy->setContainerIndex(UINT32_MAX);
  proxy->setNodeIndex(UINT32_MAX);
}

size_t CPUOctreeBroadphaseParallel::CPUParallelOctreeNode::size() const {
  return proxies.size() - freeIndices.size();
}

CPUOctreeBroadphaseParallel::CPUOctreeBroadphaseParallel(dt::thread_pool* pool, const OctreeCreateInfo &octreeInfo) {
  this->pool = pool;

  size_t nodesCount = 0;
  size_t eight = 1;
  for (uint32_t i = 0; i < octreeInfo.depth; ++i) {
    nodesCount += eight;
    eight *= 8;
  }

  this->nodesCount = nodesCount;

  if (octreeInfo.depth > 10) throw std::runtime_error("Depth > 10 is not supported");
  // при 10 получается буфер будет весить больше 10 Гб - это тоже плохо лол
  this->depth = octreeInfo.depth;

  // нужно бы как нибудь выделить аабб нодов в отдельный буфер
  // proxies.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  // proxies.reserve(nodesCount + 10000);
  // nodes.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, nodesCount);
  // indices1.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 10000);
  // indices2.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 10000);

  // changes.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 10000);
  // proxyIndices.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 10000);
  // toPairsCalculate.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 10000);

  proxies.reserve(1000);

  std::vector<CPUParallelOctreeNode> list(nodesCount);
  nodes.swap(list);
  nodeBoxes.resize(nodesCount);
  //nodes.resize(nodesCount);
  //mutexies.resize(nodesCount);
  indices1.resize(1000);
  indices2.resize(1000);

  //std::vector<std::mutex> list(nodesCount);
  //mutexes.swap(list);

//   for (uint32_t i = 0; i < nodesCount; ++i) {
//     mutexies.emplace();
//   }

  changes.resize(1000);
  proxyIndices.resize(1000);
  toPairsCalculate.resize(1000);

  memset(proxies.data(), 0, proxies.capacity() * sizeof(proxies[0]));
  memset(nodes.data(), 0, nodes.capacity() * sizeof(nodes[0]));
  memset(indices1.data(), 0, indices1.capacity() * sizeof(indices1[0]));
  memset(indices2.data(), 0, indices2.capacity() * sizeof(indices2[0]));
  memset(changes.data(), 0, changes.capacity() * sizeof(changes[0]));
  memset(proxyIndices.data(), 0, proxyIndices.capacity() * sizeof(proxyIndices[0]));
  memset(toPairsCalculate.data(), 0, toPairsCalculate.capacity() * sizeof(toPairsCalculate[0]));

  nodes[0].nodeIndex = 0;
  nodes[0].childIndex = 1;
  nodeBoxes[0] = {octreeInfo.center, octreeInfo.extent};
  //nodes[0].box = {octreeInfo.center, octreeInfo.extent};

  std::vector<size_t> nodeIdx;
  nodeIdx.reserve(nodesCount);
  nodeIdx.push_back(0);
  size_t size = 1;
  size_t offset = 0;
  size_t mul = 1;
  size_t count = 1;
  size_t lastCount = 0;
  const uint8_t xMask = (1<<0);
  const uint8_t yMask = (1<<2);
  const uint8_t zMask = (1<<1);
  for (uint32_t i = 1; i < octreeInfo.depth; ++i) { // тут должен быть 1 так как мы уже какбы прошли root
    for (uint32_t j = 0; j < size - offset; ++j) {
      const size_t parentIndex = nodeIdx[offset + j];
      nodes[parentIndex].childIndex = count + (parentIndex - lastCount) * 8;

      for (uint8_t k = 0; k < 8; ++k) {
        const size_t index = count + (parentIndex - lastCount) * 8 + k;
        const FastAABB &parentBox = nodeBoxes[parentIndex];

        //nodes.push_back({});
//         nodes[index].basePtr = this;
        nodes[index].nodeIndex = index;
        nodes[index].childIndex = UINT32_MAX;

        simd::vec4 extent = parentBox.extent / 2.0f;
        simd::vec4 center;
        center.x = xMask & k ? parentBox.center.x + extent.x : parentBox.center.x - extent.x;
        center.y = yMask & k ? parentBox.center.y + extent.y : parentBox.center.y - extent.y;
        center.z = zMask & k ? parentBox.center.z + extent.z : parentBox.center.z - extent.z;
        center.w = 1.0f;

        nodeBoxes[index] = FastAABB{center, extent};

//         proxies.emplace_back(UINT32_MAX, PhysicsType(0, 0, 0, 0, 0, 0), 0, 0);
//         proxies.back().setAABB({center, extent});

        nodeIdx.push_back(index);
      }
    }

    offset = size;
    size = nodeIdx.size();
    mul *= 8;
    lastCount = count;
    count += mul;
  }
}

CPUOctreeBroadphaseParallel::~CPUOctreeBroadphaseParallel() {

}

void CPUOctreeBroadphaseParallel::setInputBuffers(const InputBuffers &buffers) {
  indexBuffer = buffers.indexBuffer;
  verticies = buffers.verticies;
  objects = buffers.objects;
  systems = buffers.systems;
  transforms = buffers.transforms;
  rotationDatas = buffers.rotationDatas;

  rays = buffers.rays;
  frustums = buffers.frustums;
  frustumPoses = buffers.frustumPoses;
}

void CPUOctreeBroadphaseParallel::setOutputBuffers(const OutputBuffers &buffers, void* indirectPairBuffer) {
  overlappingPairCache = buffers.overlappingPairCache;
  staticOverlappingPairCache = buffers.staticOverlappingPairCache;
  rayPairCache = buffers.rayPairCache;
  frustumTestsResult = buffers.frustumTestsResult;
}

uint32_t CPUOctreeBroadphaseParallel::createProxy(const FastAABB &box, const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter) {
  // можно здесь залочить мьютекс
  // но я пока не уверен нужно ли мне это
  // создание и удаление прокси параллельно
  // для этого нужно и предыдущий шаг тоже параллельным сделать (то есть создание/удаление вообще всех объектов параллельно)
  // но скорее всего это невозможно/ненужно

  uint32_t index;

  if (freeProxy != UINT32_MAX) {
    index = freeProxy;
    freeProxy = proxies[index].dummy2;
    proxies[index] = CPUOctreeProxyParallel(objIndex, type, collisionGroup, collisionFilter);
    proxies[index].setAABB(box);
    proxies[index].proxyIndex = index;
  } else {
    index = proxies.size();
    proxies.emplace_back(objIndex, type, collisionGroup, collisionFilter);
    proxies.back().setAABB(box);
    proxies[index].proxyIndex = index;
  }

  return index;
}

BroadphaseProxy* CPUOctreeBroadphaseParallel::getProxy(const uint32_t &index) {
  return &proxies[index];
}

void CPUOctreeBroadphaseParallel::destroyProxy(BroadphaseProxy* proxy) {
  CPUOctreeProxyParallel* p = (CPUOctreeProxyParallel*)proxy;

  if (p->getNodeIndex() != UINT32_MAX) {
    nodes[p->getNodeIndex()].remove(p);
  }

  p->dummy2 = freeProxy;
  freeProxy = p->proxyIndex;
//   p->setContainerIndex(UINT32_MAX);
//   p->containerIndex = UINT32_MAX;
}

void CPUOctreeBroadphaseParallel::destroyProxy(const uint32_t &index) {
  // тип если у нас UINT32_MAX то прокси еще не успело правильно обработаться в октодереве
  if (proxies[index].getNodeIndex() != UINT32_MAX) {
    nodes[proxies[index].getNodeIndex()].remove(&proxies[index]);
  }

  proxies[index].dummy2 = freeProxy;
  freeProxy = index;
//   proxies[index].setContainerIndex(UINT32_MAX);
//   proxies[index].objIndex = UINT32_MAX;
}

void CPUOctreeBroadphaseParallel::updateBuffers(const uint32_t &objCount, const uint32_t &dynObjCount, const uint32_t &raysCount, const uint32_t &frustumsCount) {
  //if (proxies.size() < objCount) proxies.reserve(objCount);
  if (changes.size() < objCount) changes.resize(objCount);
  if (indices1.size() < objCount) indices1.resize(objCount);
  if (indices2.size() < objCount) indices2.resize(objCount);
  if (proxyIndices.size() < objCount+1) proxyIndices.resize(objCount+1);
  if (toPairsCalculate.size() < objCount+1) toPairsCalculate.resize(objCount+1);

  uint32_t powerOfTwo = nextPowerOfTwo(dynObjCount*2); // compute the next highest power of 2 of 32-bit v
  uint32_t powerOfTwoStat = nextPowerOfTwo(objCount*2);

  if (overlappingPairCache->size() < powerOfTwo+1) overlappingPairCache->resize(powerOfTwo+1);
  if (staticOverlappingPairCache->size() < powerOfTwoStat+1) staticOverlappingPairCache->resize(powerOfTwoStat+1);
  if (rayPairCache->size() < (objCount/2)+1) rayPairCache->resize((objCount/2)+1);
  if (frustumTestsResult->size() < frustumsCount*objCount+1) frustumTestsResult->resize(frustumsCount*objCount+1);

  this->raysCount = raysCount;
  this->frustumCount = frustumsCount;
}

void CPUOctreeBroadphaseParallel::update() {
  static const auto updateAABB = [&] (const size_t &start, const size_t &count, std::atomic<uint32_t> &counter) {
    for (size_t index = start; index < start+count; ++index) {
      const uint32_t &objIndex = indexBuffer->at(index+1);
      const Object &obj = objects->at(objIndex);
      const uint32_t &proxyIndex = obj.proxyIndex;
      
      // куда подевались индексы 0 и 3?????????
//       if (objIndex == 0) std::cout << "objIndex " << objIndex << "\n";
//       if (objIndex == 3) std::cout << "objIndex " << objIndex << "\n";

      if (obj.objType.getObjType() == BBOX_TYPE) {
        const simd::vec4 &pos = transforms->at(obj.transformIndex).pos;
        const simd::vec4 &ext = verticies->at(obj.vertexOffset);
        const simd::mat4 &orn = systems->at(obj.coordinateSystemIndex);

        const FastAABB aabb = recalcAABB(pos, ext, orn);
        proxies[proxyIndex].setAABB(aabb);
        
//         if (proxyIndex == 0) {
//           std::cout << "updateAABB" << "\n";
//           std::cout << "obj.coordinateSystemIndex " << obj.coordinateSystemIndex << "\n";
//           PRINT_VEC4("proxy center ", proxies[proxyIndex].getAABB().center)
//           PRINT_VEC4("proxy extent ", proxies[proxyIndex].getAABB().extent)
//           PRINT_VEC4("aabb  center ", aabb.center)
//           PRINT_VEC4("aabb  extent ", aabb.extent)
//         }

      } else if (obj.objType.getObjType() == SPHERE_TYPE) {
        const simd::vec4 &sphere = transforms->at(obj.transformIndex).pos;
        float arr[4];
        sphere.store(arr);
        
        const FastAABB box = {simd::vec4(arr[0], arr[1], arr[2], 1.0f), simd::vec4(arr[3], arr[3], arr[3], 0.0f)};
        proxies[proxyIndex].setAABB(box);
      } else {
        const simd::vec4 pos = obj.transformIndex != UINT32_MAX ? transforms->at(obj.transformIndex).pos : simd::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        simd::mat4 orn = systems->at(obj.coordinateSystemIndex);

        if (obj.rotationDataIndex != UINT32_MAX) {
          const RotationData &data = rotationDatas->at(obj.rotationDataIndex);

          const simd::mat4 &anchorMatrix = data.matrix;

          orn = anchorMatrix * orn;
        }

        proxies[proxyIndex].setAABB(recalcAABB(verticies, pos, obj.vertexOffset, obj.vertexCount, orn));
      }
      
//       if (proxyIndex == 0) {
//         std::cout << "updateAABB" << "\n";
//         PRINT_VEC4("proxy center ", proxies[proxyIndex].getAABB().center)
//         PRINT_VEC4("proxy extent ", proxies[proxyIndex].getAABB().extent)
//       }
      
      //throw std::runtime_error("no more");

      const uint32_t id = counter.fetch_add(1);
      proxyIndices[id+1] = proxyIndex;
    }
  };

  static const auto changePlaceInOctree = [&] (const size_t &start, const size_t &count, std::atomic<uint32_t> &counter) {
    for (size_t index = start; index < start+count; ++index) {
      const uint32_t &proxyIndex = proxyIndices[index+1];
      ASSERT(proxies.size() > proxyIndex);
      CPUOctreeProxyParallel* data = &proxies[proxyIndex];
      uint32_t currentIndex = 0;
      
//       if (proxyIndex == 0 || proxyIndex == 3) {
//         std::cout << "changePlaceInOctree" << "\n";
//         PRINT_VEC4("proxy center ", data->getAABB().center)
//         PRINT_VEC4("proxy extent ", data->getAABB().extent)
//       }

      bool end = true;
      while (end) {
        bool isOneChild = false;

        if (nodes[currentIndex].childIndex == UINT32_MAX) break;

        for (uint32_t i = 0; i < 8; ++i) {
          const uint32_t &index = nodes[currentIndex].childIndex + i;
          //const FastAABB &nodeBox = nodes[index].box;
          const FastAABB &nodeBox = nodeBoxes[index];
          //const bool inChild = ObjinNode(data, index); // тут нужно чекать находится ли объект в octree node
          const bool inChild = AABBcontain(data->getAABB(), nodeBox);
          isOneChild = isOneChild || inChild;

          if (isOneChild) {
            currentIndex = index;
            break;
          }
        }

        end = end && isOneChild;
      }

      if (currentIndex != data->getNodeIndex()) {
        if (data->getNodeIndex() != UINT32_MAX) nodes[data->getNodeIndex()].remove(data);
        nodes[currentIndex].add(data);
      }
      
//       std::cout << "Object Index " << data->getObjectIndex() << " data->getNodeIndex() " << data->getNodeIndex() << " node size " << nodes[currentIndex].proxies.size() << "\n";
      
//       if (proxyIndex == 0 || proxyIndex == 3) {
//         std::cout << "changePlaceInOctree" << "\n";
//         std::cout << "data->getNodeIndex() " << data->getNodeIndex() << "\n";
//       }

      if (data->getType().isDynamic()) {
        // я полагаю тут нужно использовать
        // std::memory_order_relaxed
        const uint32_t id = counter.fetch_add(1);
        toPairsCalculate[id+1] = proxyIndex;
      }
    }
  };
  
//   RegionLog rl("CPUOctreeBroadphaseParallel::update()");

  proxyIndices[0] = 0;
  std::atomic<uint32_t> proxyCounter(0);
  const uint32_t &objCount = indexBuffer->at(0);

  //if (objCount > 2) throw std::runtime_error("objCount > 2");
  // objCount+1 ?
  
  {
    const size_t count = glm::ceil(float(objCount) / float(pool->size()+1));
    size_t start = 0;
    for (uint32_t i = 0; i < pool->size()+1; ++i) {
      const size_t jobCount = std::min(count, objCount-start);
      if (jobCount == 0) break;

      pool->submitnr(updateAABB, start, jobCount, std::ref(proxyCounter));

      start += jobCount;
    }

    pool->compute();
    pool->wait();
  }
  
  //throw std::runtime_error("no more");

  proxyIndices[0] = proxyCounter;
  toPairsCalculate[0] = 0;
  
  // proxyIndices[0]+1 ?

  std::atomic<uint32_t> counter(0);
  {
    const size_t count = glm::ceil(float(proxyIndices[0]) / float(pool->size()+1));
    size_t start = 0;
    for (uint32_t i = 0; i < pool->size()+1; ++i) {
      const size_t jobCount = std::min(count, proxyIndices[0]-start);
      if (jobCount == 0) break;

      pool->submitnr(changePlaceInOctree, start, jobCount, std::ref(counter));

      start += jobCount;
    }

    pool->compute();
    pool->wait();
  }
  
  //printStats();
  
  //throw std::runtime_error("no more");

  toPairsCalculate[0] = counter;
}

// тут скорее всего не помешает раскинуть первый уровень октодерева по потокам
void CPUOctreeBroadphaseParallel::calculateOverlappingPairs() {
  static const auto checkAndAdd = [&] (const CPUOctreeProxyParallel* first, const CPUOctreeProxyParallel* second, std::atomic<uint32_t> &pairsCounter, std::atomic<uint32_t> &staticPairsCounter) {
    if ((!second->getType().isTrigger()) && (!second->getType().isBlocking())) return;

                  // x - это тип объекта, y - это те объекты с которыми есть коллизия
    const bool collide = (first->collisionGroup() & second->collisionFilter()) != 0 &&
                         (second->collisionGroup() & first->collisionFilter()) != 0;

    if (!collide) return;

    const bool currentObjTrigget = first->getType().isTrigger() && (!first->getType().isBlocking());
    const bool proxyTrigget = second->getType().isTrigger() && (!second->getType().isBlocking());
    const bool triggerOnly = currentObjTrigget || proxyTrigget;

    const bool dynamicPair = first->getType().isDynamic() && second->getType().isDynamic();

    const FastAABB &firstBox  = first->getAABB();
    const FastAABB &secondBox = second->getAABB();

    if (testAABB(firstBox, secondBox)) {
      if (dynamicPair) {
        const uint32_t id = pairsCounter.fetch_add(1);

        const uint32_t firstIndex  = glm::min(first->getObjectIndex(), second->getObjectIndex());
        const uint32_t secondIndex = glm::max(first->getObjectIndex(), second->getObjectIndex());

//         std::cout << "id " << id << "\n";
//         std::cout << "first index  " << firstIndex << "\n";
//         std::cout << "second index " << secondIndex << "\n";

        overlappingPairCache->at(id+1).firstIndex = firstIndex;
        overlappingPairCache->at(id+1).secondIndex = secondIndex;
        const float dist2 = simd::distance2(firstBox.center, secondBox.center);
        overlappingPairCache->at(id+1).dist = dist2;
        overlappingPairCache->at(id+1).islandIndex = triggerOnly ? ONLY_TRIGGER_ID : nodesCount - glm::min(first->getNodeIndex(), second->getNodeIndex()) - 1;

        // до нарров фазы мы именно эти пары мы должны отсортировать по индексам!!!
        // а после нарров фазы отсортировать по индексам островов
        // но в этом случае мне еще нужно по уровням октодерева распределить
      } else {
        const uint32_t id = staticPairsCounter.fetch_add(1);

        staticOverlappingPairCache->at(id+1).firstIndex = first->getObjectIndex();
        staticOverlappingPairCache->at(id+1).secondIndex = second->getObjectIndex();
        const float dist2 = simd::distance2(firstBox.center, secondBox.center);
        staticOverlappingPairCache->at(id+1).dist = dist2;
        staticOverlappingPairCache->at(id+1).islandIndex = triggerOnly ? ONLY_TRIGGER_ID : first->getObjectIndex();
        // эти пары похоже что нужно отсортировать только раз
        // и тоже нужно распределить по островам
      }
    }
  };

  static const std::function<void(const CPUParallelOctreeNode*, const CPUOctreeProxyParallel*, std::atomic<uint32_t>&, std::atomic<uint32_t>&)> calcPairReq =
  [&] (const CPUParallelOctreeNode* node, const CPUOctreeProxyParallel* current, std::atomic<uint32_t> &pairsCounter, std::atomic<uint32_t> &staticPairsCounter) {
//     std::cout << "calcPairReq childNode index " << node->nodeIndex << " childNode size  " << node->proxies.size() << "\n";
    
    for (uint32_t i = 0; i < node->proxies.size(); ++i) {
      if (node->proxies[i] == UINT32_MAX) continue;

      if (current->getProxyIndex() == node->proxies[i]) continue;

      checkAndAdd(current, &proxies[node->proxies[i]], pairsCounter, staticPairsCounter);
    }

    if (node->childIndex == UINT32_MAX) return;

    for (uint32_t i = 0; i < 8; ++i) {
      // нужно ли здесь добавить вычисление последующих нодов в тред пул
      // мне кажется что возможно так будет быстрее, хотя кто знает

      const CPUParallelOctreeNode* childNode = &nodes[node->childIndex + i];
      const FastAABB nodeBox = nodeBoxes[childNode->nodeIndex];
      
      if (!testAABB(nodeBox, current->getAABB())) continue;

      calcPairReq(childNode, current, pairsCounter, staticPairsCounter);
    }
  };

  static const auto calcPair = [&] (const size_t &start, const size_t &count, std::atomic<uint32_t> &pairsCounter, std::atomic<uint32_t> &staticPairsCounter) {
    for (size_t i = start; i < start+count; ++i) {
      const uint32_t index = toPairsCalculate[i+1];
//       std::cout << "index " << index << "\n";
      const CPUOctreeProxyParallel &current = proxies[index];
      const FastAABB &first = current.getAABB();

      // короче неправильный аабб сюда приходит у объекта, где может быть проблема?
      // по идее он перевычисляется в одном месте
      
//       if (index == 0) {
//         std::cout << "calcPair" << "\n";
//         PRINT_VEC4("center", first.center)
//         PRINT_VEC4("extent", first.extent)
//         throw std::runtime_error("ewpkdewdwewev");
//       }
      
      const FastAABB &nodeBox = nodeBoxes[0];
      if (!testAABB(nodeBox, first)) throw std::runtime_error(std::string("obj ") + std::to_string(proxies[index].getObjectIndex()) + " not in octree");

      const CPUParallelOctreeNode* node = &nodes[0];

      for (uint32_t i = 0; i < node->proxies.size(); ++i) {
        if (node->proxies[i] == UINT32_MAX) continue;

        if (index == node->proxies[i]) continue;

        checkAndAdd(&current, &proxies[node->proxies[i]], pairsCounter, staticPairsCounter);
      }

      if (node->childIndex == UINT32_MAX) return;

      for (uint32_t i = 0; i < 8; ++i) {
        // нужно ли здесь добавить вычисление последующих нодов в тред пул
        // мне кажется что возможно так будет быстрее, хотя кто знает

        const CPUParallelOctreeNode* childNode = &nodes[node->childIndex + i];
        const FastAABB &nodeBox = nodeBoxes[childNode->nodeIndex];
        
//         PRINT_VEC4("first center", first.center)
//         PRINT_VEC4("first extent", first.extent)
//         
//         PRINT_VEC4("nodeBox center", nodeBox.center)
//         PRINT_VEC4("nodeBox extent", nodeBox.extent)
        
        if (!testAABB(nodeBox, first)) continue;

        calcPairReq(childNode, &current, pairsCounter, staticPairsCounter);
      }
    }
  };

  // RegionLog rl("CPUOctreeBroadphaseParallel::calculateOverlappingPairs()");

  std::atomic<uint32_t> pairsCounter(0);
  std::atomic<uint32_t> staticPairsCounter(0);

  const size_t count = glm::ceil(float(toPairsCalculate[0]) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, toPairsCalculate[0]-start);
    if (jobCount == 0) break;

    pool->submitnr(calcPair, start, jobCount, std::ref(pairsCounter), std::ref(staticPairsCounter));

    start += jobCount;
  }

  pool->compute();
  pool->wait();
  
  //throw std::runtime_error("no more");

  overlappingPairCache->at(0).firstIndex = pairsCounter;
  overlappingPairCache->at(0).secondIndex = 0;
  overlappingPairCache->at(0).dist = 0;
  overlappingPairCache->at(0).islandIndex = 0;

  staticOverlappingPairCache->at(0).firstIndex = staticPairsCounter;
  staticOverlappingPairCache->at(0).secondIndex = 0;
  staticOverlappingPairCache->at(0).dist = 0;
  staticOverlappingPairCache->at(0).islandIndex = 0;

  // потом надо будет почекать как можно устроить параллельную сортировку на цпу
  // почекал сортировку, приводить к ближайшей степени двойки не нужно
  overlappingPairCache->at(0).islandIndex = overlappingPairCache->at(0).firstIndex;
}

// тут скорее всего не помешает раскинуть первый уровень октодерева по потокам
void CPUOctreeBroadphaseParallel::calculateRayTests() {
  static const auto checkAndAdd = [&] (const uint32_t &currentIndex, const RayData &current, const CPUOctreeProxyParallel* proxy, std::atomic<uint32_t> &counter) {
    if (!proxy->getType().canBlockRays()) return;

    const FastAABB &box = proxy->getAABB();

//     std::cout << "proxy " << proxy->getProxyIndex() << " obj index " << proxy->getObjectIndex() << "\n";
//     PRINT_VEC4("proxy   pos", proxy->getAABB().center)
//     PRINT_VEC4("current pos", current.pos)
//     PRINT_VEC4("current dir", current.dir)
    if (intersection(box, current)) {
      const uint32_t id = counter.fetch_add(1);
//       std::cout << "id " << id << "\n";

      rayPairCache->at(id+1).firstIndex = currentIndex;
      rayPairCache->at(id+1).secondIndex = proxy->getObjectIndex();
      const float dist2 = simd::distance2(current.pos, box.center);
      rayPairCache->at(id+1).dist = dist2;
      rayPairCache->at(id+1).islandIndex = currentIndex;
    }
  };

  static const std::function<void(const CPUParallelOctreeNode*, const uint32_t&, const RayData&, std::atomic<uint32_t>&)> calcRayPairsReq =
  [&] (const CPUParallelOctreeNode* node, const uint32_t &index, const RayData &current, std::atomic<uint32_t> &counter) {
    for (uint32_t i = 0; i < node->proxies.size(); ++i) {
      if (node->proxies[i] == UINT32_MAX) continue;

      checkAndAdd(index, current, &proxies[node->proxies[i]], counter);
    }

    if (node->childIndex == UINT32_MAX) return;

    for (uint32_t i = 0; i < 8; ++i) {
      const CPUParallelOctreeNode* childNode = &nodes[node->childIndex + i];
      const FastAABB &nodeBox = nodeBoxes[childNode->nodeIndex];
      
      if (!intersection(nodeBox, current)) continue;

      calcRayPairsReq(childNode, index, current, counter);
    }
  };

  static const auto calcRayPairs = [&] (const size_t &start, const size_t &count, std::atomic<uint32_t> &counter) {
    for (size_t index = start; index < start+count; ++index) {
      const RayData &current = rays->at(index);

      const FastAABB &nodeBox = nodeBoxes[0];
      if (!intersection(nodeBox, current)) return;

      const CPUParallelOctreeNode* node = &nodes[0];

      for (uint32_t i = 0; i < node->proxies.size(); ++i) {
        if (node->proxies[i] == UINT32_MAX) continue;

        checkAndAdd(index, current, &proxies[node->proxies[i]], counter);
      }

      if (node->childIndex == UINT32_MAX) return;

      for (uint32_t i = 0; i < 8; ++i) {
        const CPUParallelOctreeNode* childNode = &nodes[node->childIndex + i];
        const FastAABB &nodeBox = nodeBoxes[childNode->nodeIndex];
        
        if (!intersection(nodeBox, current)) continue;

        calcRayPairsReq(childNode, index, current, counter);
      }
    }
  };

  // RegionLog rl("CPUOctreeBroadphaseParallel::calculateRayTests()");

  std::atomic<uint32_t> counter(0);

  const size_t count = glm::ceil(float(raysCount) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, raysCount-start);
    if (jobCount == 0) break;

    pool->submitnr(calcRayPairs, start, jobCount, std::ref(counter));

    start += jobCount;
  }

  pool->compute();
  pool->wait();

  rayPairCache->at(0).firstIndex = counter;
}

void CPUOctreeBroadphaseParallel::calculateFrustumTests() {
  static const std::function<void(const CPUParallelOctreeNode*, const uint32_t &, std::atomic<uint32_t> &)> addObjInNode =
  [&] (const CPUParallelOctreeNode* node, const uint32_t &index, std::atomic<uint32_t> &counter) {
    //const Frustum &frustum = frustums->at(index);
    const simd::vec4 &pos = frustumPoses->at(index);

    for (uint32_t i = 0; i < node->proxies.size(); ++i) {
      if (node->proxies[i] == UINT32_MAX) continue;

      if (!proxies[node->proxies[i]].getType().isVisible()) continue;

      const CPUOctreeProxyParallel* proxy = &proxies[node->proxies[i]];

      //if (testFrustumAABB(frustum, proxy->getAABB()) == OUTSIDE) continue;

      const uint32_t id = counter.fetch_add(1);

      frustumTestsResult->at(id+1).firstIndex = index;
      frustumTestsResult->at(id+1).secondIndex = proxy->getObjectIndex();
      const float dist2 = simd::distance2(pos, proxy->getAABB().center);
      frustumTestsResult->at(id+1).dist = dist2;
      frustumTestsResult->at(id+1).islandIndex = index;
    }

    if (node->childIndex == UINT32_MAX) return;

    for (uint32_t i = 0; i < 8; ++i) {
      const CPUParallelOctreeNode* childNode = &nodes[node->childIndex + i];

      addObjInNode(childNode, index, counter);
    }
  };

  static const std::function<void(const CPUParallelOctreeNode*, const uint32_t &, std::atomic<uint32_t> &)> calcFrustumPairsReq =
  [&] (const CPUParallelOctreeNode* node, const uint32_t &index, std::atomic<uint32_t> &counter) {
    const Frustum &frustum = frustums->at(index);
    const simd::vec4 &pos = frustumPoses->at(index);
    
//     std::cout << "childNode index " << node->nodeIndex << " childNode size  " << node->proxies.size() << "\n";

    for (uint32_t i = 0; i < node->proxies.size(); ++i) {
      if (node->proxies[i] == UINT32_MAX) continue;

      if (!proxies[node->proxies[i]].getType().isVisible()) continue;
      
      ASSERT(proxies.size() > node->proxies[i]);
      const CPUOctreeProxyParallel* proxy = &proxies[node->proxies[i]];
      
//       std::cout << "proxy index " << node->proxies[i] << "\n";
//       PRINT_VEC4("proxy center ", proxy->getAABB().center)
//       PRINT_VEC4("proxy extent ", proxy->getAABB().extent)
      
      const uint32_t res = testFrustumAABB(frustum, proxy->getAABB());
//       std::cout << "res " << res << '\n';
      if (res == OUTSIDE) continue;

      const uint32_t id = counter.fetch_add(1);

      frustumTestsResult->at(id+1).firstIndex = index;
      frustumTestsResult->at(id+1).secondIndex = proxy->getObjectIndex();
      const float dist2 = simd::distance2(pos, proxy->getAABB().center);
      frustumTestsResult->at(id+1).dist = dist2;
      frustumTestsResult->at(id+1).islandIndex = index;
    }

    if (node->childIndex == UINT32_MAX) return;

    for (uint32_t i = 0; i < 8; ++i) {
      const CPUParallelOctreeNode* childNode = &nodes[node->childIndex + i];
      const FastAABB &nodeBox = nodeBoxes[childNode->nodeIndex];

      int res = testFrustumAABB(frustum, nodeBox);

      if (res == INSIDE) addObjInNode(childNode, index, counter);
      else if (res == INTERSECT) calcFrustumPairsReq(childNode, index, counter);
    }
  };

  static const auto calcFrustumPairs = [&] (const size_t &start, const size_t &count, dt::thread_pool* pool, std::atomic<uint32_t> &counter) {
    for (size_t index = start; index < start+count; ++index) {
      const Frustum &frustum = frustums->at(index);
      const simd::vec4 &pos = frustumPoses->at(index);

      const FastAABB &nodeBox = nodeBoxes[0];
      if (testFrustumAABB(frustum, nodeBox) == OUTSIDE) throw std::runtime_error("wtf?");

      const CPUParallelOctreeNode* node = &nodes[0];

      for (uint32_t i = 0; i < node->proxies.size(); ++i) {
        if (node->proxies[i] == UINT32_MAX) continue;

        if (!proxies[node->proxies[i]].getType().isVisible()) continue;

        const CPUOctreeProxyParallel* proxy = &proxies[node->proxies[i]];

        int res = testFrustumAABB(frustum, proxy->getAABB());

//         std::cout << "proxy index " << node->proxies[i] << "\n";
//         PRINT_VEC4("proxy center ", proxy->getAABB().center)
//         PRINT_VEC4("proxy extent ", proxy->getAABB().extent)
//         std::cout << "res " << res << '\n';
        
        if (res == OUTSIDE) continue;

        const uint32_t id = counter.fetch_add(1);

        frustumTestsResult->at(id+1).firstIndex = index;
        frustumTestsResult->at(id+1).secondIndex = proxy->getObjectIndex();
        const float dist2 = simd::distance2(pos, proxy->getAABB().center);
        frustumTestsResult->at(id+1).dist = dist2;
        frustumTestsResult->at(id+1).islandIndex = index;
      }

      if (node->childIndex == UINT32_MAX) return;

      for (uint32_t i = 0; i < 8; ++i) {
        const CPUParallelOctreeNode* childNode = &nodes[node->childIndex + i];
        const FastAABB &nodeBox = nodeBoxes[childNode->nodeIndex];

        int res = testFrustumAABB(frustum, nodeBox);
        
//         PRINT_VEC4("childNode->box center ", childNode->box.center)
//         PRINT_VEC4("childNode->box extent ", childNode->box.extent)
//         std::cout << "res " << res << '\n';

        // тут поди надо добавить эти функции в треад пул
//         if (res == INSIDE) addObjInNode(childNode, index, counter);
//         else if (res == INTERSECT) calcFrustumPairsReq(childNode, index, counter);

        if (res == INSIDE) pool->submitnr(addObjInNode, childNode, index, std::ref(counter));
        else if (res == INTERSECT) pool->submitnr(calcFrustumPairsReq, childNode, index, std::ref(counter));
//         pool->submitnr(addObjInNode, childNode, index, std::ref(counter));
      }
    }
  };

  // RegionLog rl("CPUOctreeBroadphaseParallel::calculateFrustumTests()");

  std::atomic<uint32_t> counter(0);

  const size_t count = glm::ceil(float(frustumCount) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, frustumCount-start);
    if (jobCount == 0) break;

    pool->submitnr(calcFrustumPairs, start, jobCount, pool, std::ref(counter));

    start += jobCount;
  }

  pool->compute();
  pool->wait();
  
  //printStats();
  
  //throw std::runtime_error("no more");

  frustumTestsResult->at(0).firstIndex = counter;
}

void CPUOctreeBroadphaseParallel::postlude() {

}

void CPUOctreeBroadphaseParallel::printStats() {
  const size_t totalMemory = proxies.capacity() * sizeof(proxies[0]) +
                             nodes.capacity() * sizeof(nodes[0]) +
                             changes.capacity() * sizeof(changes[0]) +
                             indices1.capacity() * sizeof(indices1[0]) +
                             indices2.capacity() * sizeof(indices2[0]) +
                             proxyIndices.capacity() * sizeof(proxyIndices[0]) +
                             toPairsCalculate.capacity() * sizeof(toPairsCalculate[0]);// +
//                              objPairs.vector().capacity() * sizeof(proxies[0]) +
//                              rayPairs.vector().capacity() * sizeof(proxies[0]) +
//                              frustumPairs.vector().capacity() * sizeof(proxies[0]);

  const size_t usedMemory = proxies.size() * sizeof(proxies[0]) +
                            nodes.size() * sizeof(nodes[0]) +
                            (changes[0].x+1) * sizeof(proxies[0]) +
                            proxies.size() * sizeof(indices1[0]) +
                            proxies.size() * sizeof(indices2[0]) +
                            (proxyIndices[0]+1) * sizeof(proxyIndices[0]) +
                            (toPairsCalculate[0]+1) * sizeof(toPairsCalculate[0]);// +
//                             (objPairs[0].firstIndex+1) * sizeof(objPairs[0]) +
//                             (rayPairs[0].firstIndex+1) * sizeof(rayPairs[0]) +
//                             frustumPairs.size() * sizeof(frustumPairs[0]);

  std::cout << '\n';
  std::cout << "CPU octree broadphase data" << '\n';
  std::cout << "Node count    " << nodesCount << " Depth " << depth << '\n';
  std::cout << "Proxies count " << proxies.size() << '\n';
  std::cout << "Total memory usage     " << totalMemory << " bytes" << '\n';
  std::cout << "Total memory with data " << usedMemory << " bytes" << '\n';
  std::cout << "Free memory            " << (totalMemory-usedMemory) << " bytes" << '\n';
  std::cout << "Broadphase class size  " << sizeof(CPUOctreeBroadphaseParallel) << " bytes" << '\n';

  for (uint32_t i = nodesCount; i < proxies.size(); ++i) {
    // std::cout << "Proxy " << i << ": \n";
    // std::cout << "node index     : " << proxies[i].nodeIndex << "\n";
    // std::cout << "container index: " << proxies[i].containerIndex << ": \n";
    // std::cout << "obj index: " << proxies[i].objIndex << ": \n";
//     PRINT_VEC4("proxy center", proxies[i].getAABB().center)
//     PRINT_VEC4("proxy extent", proxies[i].getAABB().extent)
  }

  std::cout << "Proxy count to update: " << proxyIndices[0] << "\n";

  PRINT_VAR("Calculating pair index count", toPairsCalculate[0])

  std::cout << '\n';

  PRINT_VAR("Pairs count", overlappingPairCache->at(0).firstIndex)
  PRINT_VAR("Max node count", overlappingPairCache->at(0).secondIndex)
  PRINT_VAR("Global obj count", glm::floatBitsToUint(overlappingPairCache->at(0).dist))
  PRINT_VAR("Test", overlappingPairCache->at(0).islandIndex)

  // for (uint32_t i = 1; i < objPairs[0].firstIndex+1; ++i) {
  //   if (objPairs[i].islandIndex == UINT32_MAX) continue;

  //   std::cout << "Pair " << (i-1) << "\n";
  //   std::cout << "First index  " << objPairs[i].firstIndex << '\n';
  //   std::cout << "Second index " << objPairs[i].secondIndex << '\n';
  //   std::cout << "Distance     " << objPairs[i].dist << '\n';
  //   std::cout << "Island index " << objPairs[i].islandIndex << '\n';
  //   std::cout << '\n';
  // }

  std::cout << '\n';
  std::cout << "Nodes" << '\n';

  uint32_t objCount = 0;
  uint32_t maxOffset = 0;
  for (uint32_t i = 0; i < nodesCount; ++i) {
    // std::cout << "Node index  " << i << '\n';
    // std::cout << "Node count  " << nodes[i].count << '\n';
    // std::cout << "Node offset " << nodes[i].offset << '\n';
    // std::cout << '\n';

    objCount += nodes[i].proxies.size();
  }

  std::cout << "Recalc objCount " << objCount << '\n';
  std::cout << "Max offset " << maxOffset << '\n';
  //std::cout << "Previous offset " << nodes[abnormalOffsetIndex-1].offset << '\n';

  std::cout << '\n';

  // for (uint32_t i = 1; i < toPairsCalculate[0]+1; i++) {
  //   if (toPairsCalculate[i] > 5681) std::cout << "To pairs calc index " << toPairsCalculate[i] << '\n';
  // }

  std::cout << '\n';

  PRINT_VAR("Valuable pairs", overlappingPairCache->at(0).secondIndex)
  PRINT_VAR("Only trigger pairs", glm::floatBitsToUint(overlappingPairCache->at(0).dist))
  PRINT_VAR("Next power of two", overlappingPairCache->at(0).islandIndex)

  std::cout << '\n';

  uint32_t maxObjInNode = 0;
  uint32_t objsInNodes = 0;
  uint32_t index = 0;
  for (uint32_t i = 0; i < nodesCount; ++i) {
    objsInNodes += nodes[i].size();
    if (nodes[i].size() > maxObjInNode) {
      maxObjInNode = nodes[i].size();
      index = i;
    }
  }

  PRINT_VAR("Avg obj in node", float(objsInNodes) / float(nodesCount))
  PRINT_VAR("Max obj in node", maxObjInNode)
  PRINT_VAR("Node idx with max", index)
  const FastAABB &nodeBox = nodeBoxes[index];
  PRINT_VEC4("Max node center", nodeBox.center)
  PRINT_VEC4("Max node extent", nodeBox.extent)

  std::cout << '\n';

  // for (uint32_t i = nodes[index].offset; i < nodes[index].offset+nodes[index].count; ++i) {
  //   const uint32_t proxyIndex = indices2[index];
  //   std::cout << "Proxy index " << proxyIndex << '\n';
  //   PRINT_VEC4("proxy center", proxies[proxyIndex].getAABB().center)
  //   PRINT_VEC4("proxy extent", proxies[proxyIndex].getAABB().extent)
  // }

  // for (uint32_t i = 0; i < glm::min(uint32_t(100), nodes[index].count); ++i) {
  //   std::cout << "Obj index " << i << '\n';
  //   // PRINT_VEC4("Center", proxies[indices2[nodes[index].offset+i]].getAABB().center)
  //   // PRINT_VEC4("Extent", proxies[indices2[nodes[index].offset+i]].getAABB().extent)
  //   std::cout << "Proxy index " << indices2[nodes[index].offset+i] << '\n';
  // }

  std::cout << "Changes count " << changes[0].x << '\n';
  std::cout << "Proxy indices count " << proxyIndices[0] << '\n';
  // for (uint32_t i = 1; i < proxyIndices[0]+1; ++i) {
  //   std::cout << "proxy index " << proxyIndices[i] << '\n';
  // }
  // for (uint32_t i = 1; i < changes[0].x+1; ++i) {
  //   std::cout << "proxy index " << changes[i].z << '\n';
  // }

  // uint32_t count = 0;
  // for (uint32_t i = 1; i < changes[0].x+1; ++i) {
  //   if (changes[i].w != 4681) ++count;
  //   //std::cout << "Change proxy index " << changes[i].w << '\n';
  // }
  // std::cout << "Normal changes count " << count << '\n';

  // if (objPairs[0].firstIndex > 0) {
  //   for (uint32_t i = 0; i < 1000; ++i) {
  //     //if (indices2[i] == 0) std::cout << "Idx with max id " << i << '\n';
  //     std::cout << "index " << " has proxy index " << indices2[i] << '\n';
  //   }
  // }

  // std::cout << "NodeIdx" << '\n';
  // for (uint32_t i = 0; i < 6; ++i) {
  //   std::cout << indices2[i] << " ";
  // }
  // std::cout << '\n';
}

// simd::vec4 getVertex(const simd::vec4 &pos, const simd::vec4 &ext, const simd::mat4 &orn, const uint32_t &index) {
//   simd::vec4 p = pos;
//   p = (index & 1) > 0 ? p + orn[0]*ext.x : p - orn[0]*ext.x;
//   p = (index & 2) > 0 ? p + orn[1]*ext.y : p - orn[1]*ext.y;
//   p = (index & 4) > 0 ? p + orn[2]*ext.z : p - orn[2]*ext.z;
//   p.w = 1.0f;
//
//   return p;
// }

// FastAABB recalcAABB(const simd::vec4 &pos, const simd::vec4 &ext, const simd::mat4 &orn) {
//   simd::vec4 mx = getVertex(pos, ext, orn, 0), mn = getVertex(pos, ext, orn, 0);
//
//   for (uint32_t i = 1; i < 8; ++i) {
//     mx = glm::max(mx, getVertex(pos, ext, orn, i));
//     mn = glm::min(mn, getVertex(pos, ext, orn, i));
//   }
//
//   const simd::vec4 boxPos =         (mn + mx) / 2.0f;
//   const simd::vec4 boxExt = glm::abs(mn - mx) / 2.0f;
//
//   return {boxPos, boxExt};
// }
//
// FastAABB recalcAABB(const ArrayInterface<simd::vec4>* verticies, const simd::vec4 &pos, const uint32_t &vertexOffset, const uint32_t &vertexSize, const simd::mat4 &orn) {
//   const simd::vec4 dir = simd::vec4(pos.x, pos.y, pos.z, 0.0f);
//   simd::vec4 mx = orn * (dir + verticies->at(vertexOffset)), mn = orn * (dir + verticies->at(vertexOffset));
//
//   for (uint32_t i = 1; i < vertexSize; ++i) {
//     mx = glm::max(mx, orn * (dir + verticies->at(vertexOffset + i)));
//     mn = glm::min(mn, orn * (dir + verticies->at(vertexOffset + i)));
//   }
//
//   const simd::vec4 boxPos =         (mn + mx) / 2.0f;
//   const simd::vec4 boxExt = glm::abs(mn - mx) / 2.0f;
//
//   return {boxPos, boxExt};
// }
//
// // bool testAABB(const FastAABB &first, const FastAABB &second) {
// //   const simd::vec4 center = glm::abs(first.center - second.center);
// //   const simd::vec4 extent =          first.extent + second.extent;
//
// //   return extent.x > center.x && extent.y > center.y && extent.z > center.z;
// // }
//
// bool intersection(const FastAABB &box, const RayData &ray) {
//   const simd::vec4 boxMin = box.center - box.extent;
//   const simd::vec4 boxMax = box.center + box.extent;
//
//   float t1 = (boxMin[0] - ray.dir[0]) / ray.dir[0];
//   float t2 = (boxMax[0] - ray.dir[0]) / ray.dir[0];
//
//   float tmin = glm::min(t1, t2);
//   float tmax = glm::max(t1, t2);
//
//   for (int i = 1; i < 3; ++i) {
//     t1 = (boxMin[i] - ray.dir[i])/ray.dir[i];
//     t2 = (boxMax[i] - ray.dir[i])/ray.dir[i];
//
//     tmin = glm::max(tmin, glm::min(t1, t2));
//     tmax = glm::min(tmax, glm::max(t1, t2));
//   }
//
//   return tmax > glm::max(tmin, 0.0f);
// }
//
// uint32_t testFrustumAABB(const Frustum &frustum, const FastAABB &box) {
//   uint32_t result = INSIDE; // Assume that the aabb will be inside the frustum
//   for(uint32_t i = 0; i < 6; ++i) {
//     const simd::vec4 frustumPlane = frustum.planes[i];
//
//     const float d = glm::dot(box.center,          simd::vec4(frustumPlane.x, frustumPlane.y, frustumPlane.z, 0.0f));
//
//     const float r = glm::dot(box.extent, glm::abs(simd::vec4(frustumPlane.x, frustumPlane.y, frustumPlane.z, 0.0f)));
//
//     const float d_p_r = d + r;
//     const float d_m_r = d - r;
//
//     if (d_p_r < -frustumPlane.w) {
//       result = OUTSIDE;
//       break;
//     } else if (d_m_r < -frustumPlane.w) result = INTERSECT;
//   }
//
//   return result;
// }
