#include "CPUOctreeBroadphase.h"

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

#include "HelperFunctions.h"

// FastAABB recalcAABB(const simd::vec4 &pos, const simd::vec4 &ext, const glm::mat4 &orn);
// FastAABB recalcAABB(const ArrayInterface<simd::vec4>* verticies, const simd::vec4 &pos, const uint32_t &vertexOffset, const uint32_t &vertexSize, const glm::mat4 &orn);
// //bool AABBcontain(const FastAABB &first, const FastAABB &second);
// 
// //bool testAABB(const FastAABB &first, const FastAABB &second);
// 
// bool intersection(const FastAABB &box, const RayData &ray);
// 
// uint32_t testFrustumAABB(const FrustumStruct &frustum, const FastAABB &box);

CPUOctreeProxy::CPUOctreeProxy(const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter) : 
                  BroadphaseProxy(objIndex, type, collisionGroup, collisionFilter) {
  nodeIndex = UINT32_MAX;
  containerIndex = UINT32_MAX-1;
  //objType = type;

  dummy2 = UINT32_MAX;
}
CPUOctreeProxy::~CPUOctreeProxy() {}

uint32_t CPUOctreeProxy::getNodeIndex() const {
  return nodeIndex;
}

uint32_t CPUOctreeProxy::getContainerIndex() const {
  return containerIndex;
}

void CPUOctreeProxy::setNodeIndex(const uint32_t &index) {
  nodeIndex = index;
}

void CPUOctreeProxy::setContainerIndex(const uint32_t &index) {
  containerIndex = index;
}

void CPUOctreeProxy::updateAABB() {

}

CPUOctreeBroadphase::CPUOctreeBroadphase(const OctreeCreateInfo &octreeInfo) {
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

  proxies.reserve(nodesCount + 10000);
  nodes.reserve(nodesCount);
  indices1.resize(10000);
  indices2.resize(10000);

  changes.resize(10000);
  proxyIndices.resize(10000);
  toPairsCalculate.resize(10000);

  memset(proxies.data(), 0, proxies.capacity() * sizeof(proxies[0]));
  memset(nodes.data(), 0, nodes.capacity() * sizeof(nodes[0]));
  memset(indices1.data(), 0, indices1.capacity() * sizeof(indices1[0]));
  memset(indices2.data(), 0, indices2.capacity() * sizeof(indices2[0]));
  memset(changes.data(), 0, changes.capacity() * sizeof(changes[0]));
  memset(proxyIndices.data(), 0, proxyIndices.capacity() * sizeof(proxyIndices[0]));
  memset(toPairsCalculate.data(), 0, toPairsCalculate.capacity() * sizeof(toPairsCalculate[0]));

  nodes[0].offset = 0;
  nodes[0].count = 0;
  nodes[0].childIndex = 1;
  nodes[0].proxyIndex = proxies.size();
  proxies.emplace_back(UINT32_MAX, PhysicsType(0, 0, 0, 0, 0, 0), 0, 0);
  proxies.back().setAABB({octreeInfo.center, octreeInfo.extent});

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
        const size_t parentProxyIndex = nodes[parentIndex].proxyIndex;

        nodes[index].childIndex = UINT32_MAX;
        nodes[index].proxyIndex = proxies.size();
        nodes[index].offset = 0;
        nodes[index].count = 0;

        simd::vec4 extent = proxies[parentProxyIndex].getAABB().extent / 2.0f;
        simd::vec4 center;
        center.x = xMask & k ? proxies[parentProxyIndex].getAABB().center.x + extent.x : proxies[parentProxyIndex].getAABB().center.x - extent.x;
        center.y = yMask & k ? proxies[parentProxyIndex].getAABB().center.y + extent.y : proxies[parentProxyIndex].getAABB().center.y - extent.y;
        center.z = zMask & k ? proxies[parentProxyIndex].getAABB().center.z + extent.z : proxies[parentProxyIndex].getAABB().center.z - extent.z;
        center.w = 1.0f;

        //nodes[index].box = AABB(center, extent);

        proxies.emplace_back(UINT32_MAX, PhysicsType(0, 0, 0, 0, 0, 0), 0, 0);
        proxies.back().setAABB({center, extent});

        nodeIdx.push_back(index);
      }
    }

    offset = size;
    size = nodeIdx.size();
    mul *= 8;
    lastCount = count;
    count += mul;
  }

  // for (uint32_t i = 0; i < nodesCount; ++i) {
  //   std::cout << "node index " << i << '\n';
  //   std::cout << "node child " << nodes[i].childIndex << '\n';
  // }

  //objPairs.construct(device, 4096);
  
//   const uint32_t pairSize = 4096+1;
//   objPairs.resize(pairSize);
//   staticObjPairs.resize(pairSize);
//   //objPairs.resize(3000);
//   //rayPairs.construct(device, 4096);
//   rayPairs.resize(pairSize);
//   //rayPairs.resize(3000);
//   //frustumPairs.construct(device, 4096);
//   frustumPairs.resize(pairSize);
//   //frustumPairs.resize(3000);
// 
//   memset(objPairs.vector().data(), 0, objPairs.vector().capacity() * sizeof(objPairs[0]));
//   memset(rayPairs.vector().data(), 0, rayPairs.vector().capacity() * sizeof(rayPairs[0]));
//   memset(frustumPairs.vector().data(), 0, frustumPairs.vector().capacity() * sizeof(frustumPairs[0]));
}

CPUOctreeBroadphase::~CPUOctreeBroadphase() {

}

// void CPUOctreeBroadphase::setCollisionTestBuffer(ArrayInterface<uint32_t>* indexBuffer) {
//   this->indexBuffer = indexBuffer;
// }
// 
// void CPUOctreeBroadphase::setRayTestBuffer(ArrayInterface<RayData>* rayBuffer) {
//   rays = rayBuffer;
// }
// 
// void CPUOctreeBroadphase::setUpdateBuffers(const UpdateBuffers &buffers) {
//   verticies = buffers.verticies;
//   objects = buffers.objects;
//   systems = buffers.systems;
//   transforms = buffers.transforms;
//   rotationDatas = buffers.rotationDatas;
// 
//   rays = buffers.rays;
//   frustums = buffers.frustums;
//   frustumPoses = buffers.frustumPoses;
// }

void CPUOctreeBroadphase::setInputBuffers(const InputBuffers &buffers) {
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

void CPUOctreeBroadphase::setOutputBuffers(const OutputBuffers &buffers, void* indirectPairBuffer) {
  overlappingPairCache = buffers.overlappingPairCache;
  staticOverlappingPairCache = buffers.staticOverlappingPairCache;
  rayPairCache = buffers.rayPairCache;
  frustumTestsResult = buffers.frustumTestsResult;
}

uint32_t CPUOctreeBroadphase::createProxy(const FastAABB &box, const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter) {
  uint32_t index;

  if (freeProxy != UINT32_MAX) {
    index = freeProxy;
    freeProxy = proxies[index].dummy2;
    proxies[index] = CPUOctreeProxy(objIndex, type, collisionGroup, collisionFilter);
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

BroadphaseProxy* CPUOctreeBroadphase::getProxy(const uint32_t &index) {
  return &proxies[index];
}

void CPUOctreeBroadphase::destroyProxy(BroadphaseProxy* proxy) {
  CPUOctreeProxy* p = (CPUOctreeProxy*)proxy;
  p->dummy2 = freeProxy;
  freeProxy = p->proxyIndex;
  p->proxyIndex = UINT32_MAX;
  p->containerIndex = UINT32_MAX;
}

void CPUOctreeBroadphase::destroyProxy(const uint32_t &index) {
  proxies[index].dummy2 = freeProxy;
  freeProxy = index;
  proxies[index].setContainerIndex(UINT32_MAX);
  proxies[index].objIndex = UINT32_MAX;
}

void CPUOctreeBroadphase::updateBuffers(const uint32_t &objCount, const uint32_t &dynObjCount, const uint32_t &raysCount, const uint32_t &frustumsCount) {
  //if (proxies.size() < objCount) proxies.reserve(objCount);
  if (changes.size() < dynObjCount) changes.resize(dynObjCount);
  if (indices1.size() < objCount) indices1.resize(objCount);
  if (indices2.size() < objCount) indices2.resize(objCount);
  if (proxyIndices.size() < dynObjCount+1) proxyIndices.resize(dynObjCount+1);
  if (toPairsCalculate.size() < dynObjCount+1) toPairsCalculate.resize(dynObjCount+1);

  uint32_t v = dynObjCount*2; // compute the next highest power of 2 of 32-bit v
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  
  if (overlappingPairCache->size() < v+1) overlappingPairCache->resize(v+1);
  if (staticOverlappingPairCache->size() < v+1) staticOverlappingPairCache->resize(v+1);
  if (rayPairCache->size() < raysCount*3) rayPairCache->resize(raysCount*3);
  if (frustumTestsResult->size() < frustumsCount*objCount) frustumTestsResult->resize(frustumsCount*objCount);

  this->raysCount = raysCount;
  this->frustumCount = frustumsCount;
}

void CPUOctreeBroadphase::update() {
  auto start = std::chrono::steady_clock::now();

  proxyIndices[0] = 0;

  const uint32_t &objCount = indexBuffer->at(0);
  for (uint32_t i = 1; i < objCount+1; ++i) {
    const uint32_t index = i;

    const uint32_t &objIndex = indexBuffer->at(index);
    const Object &obj = objects->at(objIndex);
    const uint32_t &proxyIndex = obj.proxyIndex;

    if (obj.objType.getObjType() == BBOX_TYPE) {
      const simd::vec4 &pos = transforms->at(obj.transformIndex).pos;
      const simd::vec4 &ext = verticies->at(obj.vertexOffset);
      const simd::mat4 &orn = systems->at(obj.coordinateSystemIndex);

      proxies[proxyIndex].setAABB(recalcAABB(pos, ext, orn));
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

    //const uint32_t a = obj.worldInfo.z & DYNAMIC;
    //if (a == DYNAMIC) {
      // const uint32_t id = atomicAdd(outCount, 1);
      // outProxies[id] = proxyIndex;
      proxyIndices[proxyIndices[0]+1] = proxyIndex;
      ++proxyIndices[0];
    //}
  }

  toPairsCalculate[0] = 0;
  changes[0].x = 0;
  
//   static bool exit = false;
    
//   if (exit) {
//     std::cout << "proxies count " << proxyIndices[0] << '\n';
//     for (uint32_t i = 1; i < proxyIndices[0]+1; ++i) {
//       std::cout << "proxies index " << proxyIndices[i] << "\n";
//     }
//     assert(proxyIndices[0] == 1);
//   }
//   
//   exit = true;

  for (uint32_t i = 1; i < proxyIndices[0]+1; ++i) {
    const uint32_t index = i;

    const uint32_t &proxyIndex = proxyIndices[index];
    const CPUOctreeProxy &data = proxies[proxyIndex];

    // и тип здесь нужно заново раскидывать объекты по октодереву
    //uint32_t currentIndex = data.getContainerIndex() == UINT32_MAX ? UINT32_MAX : 0;
    uint32_t currentIndex = 0;

    //if (currentIndex != UINT32_MAX) {
      bool end = true;
      while (end) {
        bool isOneChild = false;

        if (nodes[currentIndex].childIndex == UINT32_MAX) break;

        for (uint32_t i = 0; i < 8; ++i) {
          const uint32_t &index = nodes[currentIndex].childIndex + i;
          const FastAABB &nodeBox = proxies[nodes[index].proxyIndex].getAABB();
          //const bool inChild = ObjinNode(data, index); // тут нужно чекать находится ли объект в octree node
          const bool inChild = AABBcontain(data.getAABB(), nodeBox);
          isOneChild = isOneChild || inChild;

          if (isOneChild) {
            currentIndex = index;
            break;
          }
        }

        //if (!isOneChild) end = false;
        end = end && isOneChild;
      }
    //}

    if (currentIndex != data.getNodeIndex()) {
      // const uint32_t index = atomicAdd(count.x, 1);
      // changes[index].x = data.octreeData.x;   // то где мы были раньше
      // changes[index].y = data.octreeData.y;   // индекс (пригодился)
      // changes[index].z = currentIndex;     // то куда мы попадаем
      // changes[index].w = proxyIndex;       // индекс прокси
      
      const uint32_t id = changes[0].x;
      ++changes[0].x;

      changes[id+1].x = data.getNodeIndex(); // то где мы были раньше
      changes[id+1].y = data.getContainerIndex(); // индекс (пригодился)
      changes[id+1].z = currentIndex; // то куда мы попадаем
      changes[id+1].w = proxyIndex; // индекс прокси
      
//       std::cout << "changes old node " << changes[id+1].x << "\n";
//       std::cout << "changes new node " << changes[id+1].z << "\n";
//       std::cout << "changes proxy    " << changes[id+1].w << "\n";
//       std::cout << "proxy node index " << proxies[changes[id+1].w].getNodeIndex() << "\n";
    }

    // const uint32_t a = data.proxyData.z & DYNAMIC;
    // if (a == DYNAMIC) {
    //   const uint32_t id = atomicAdd(outCount, 1);
    //   outProxies[id] = proxyIndex;//data.proxyData.z;
    //
    //   atomicAdd(command.x, 1);
    // }

    if (data.getType().isDynamic()) {
      // const uint32_t id = atomicAdd(outCount, 1);
      // outProxies[id] = proxyIndex;//data.proxyData.z;

      // atomicAdd(command.x, 1);

      toPairsCalculate[toPairsCalculate[0]+1] = proxyIndex;
      ++toPairsCalculate[0];
    }
  }

  memcpy(indices1.data(), indices2.data(), indices2.size() * sizeof(indices2[0]));

//   for (uint32_t i = 0; i < nodesCount; ++i) {
//     uint32_t newOffset = 0;
//     for (uint32_t j = 0; j < i; ++j) {
//       const uint32_t index = j;
// 
//       newOffset += nodes[index].count;
//     }
// 
//     nodes[i].offset = newOffset;
//   }

  for (uint32_t i = 0; i < nodesCount; ++i) {
    const uint32_t nodeIndex = i;
    
    assert(nodes[nodeIndex].offset < 10000);
    assert(nodes[nodeIndex].count < 10000);
    
    uint32_t newOffset = nodes[nodeIndex].offset;
    uint32_t addSize = 0;
    uint32_t subSize = 0;
    uint32_t currentIndex = 0;

    for (uint32_t j = 1; j < changes[0].x+1; ++j) {
      newOffset -= uint32_t(changes[j].x < nodeIndex);
      newOffset += uint32_t(changes[j].z < nodeIndex);
      addSize   += uint32_t(changes[j].z == nodeIndex);
      subSize   += uint32_t(changes[j].x == nodeIndex);
    }

    const uint32_t newSize = nodes[nodeIndex].count + addSize - subSize;
    const uint32_t countThatStay = nodes[nodeIndex].count - subSize;
    
//     if (nodeIndex == 0) {
//       std::cout << "\n";
//       std::cout << "node 0" << "\n";
//       std::cout << "offset     " << nodes[nodeIndex].offset << "\n";
//       std::cout << "count      " << nodes[nodeIndex].count << "\n";
//       
//       std::cout << "newOffset  " << newOffset << "\n";
//       std::cout << "addSize    " << addSize << "\n";
//       std::cout << "subSize    " << subSize << "\n";
//     }
//     
//     if (nodeIndex == 136) {
//       static bool exit = false;
//       
//       std::cout << "\n";
//       std::cout << "node 136" << "\n";
//       std::cout << "offset     " << nodes[nodeIndex].offset << "\n";
//       std::cout << "count      " << nodes[nodeIndex].count << "\n";
//       
//       std::cout << "newOffset  " << newOffset << "\n";
//       std::cout << "addSize    " << addSize << "\n";
//       std::cout << "subSize    " << subSize << "\n";
//       
// //       for (uint32_t j = 1; j < changes[0].x+1; ++j) {
// //         std::cout << "changes old node " << changes[j].x << "\n";
// //         std::cout << "changes new node " << changes[j].z << "\n";
// //         std::cout << "changes proxy    " << changes[j].w << "\n";
// //         std::cout << "proxy node index " << proxies[changes[j].w].getNodeIndex() << "\n";
// //       }
//       
//       if (exit) {
//         assert(changes[0].x < 2);
//         assert(nodes[nodeIndex].count > 0);
//       }
//       
//       exit = true;
//     }

    if (nodes[nodeIndex].count == 0 && newSize == 0) continue;

    // я удалял старый объект не из того нода!!!
    for (uint32_t j = 1; j < changes[0].x+1; ++j) {
      if (nodeIndex == changes[j].x && changes[j].y < UINT32_MAX-1) indices1[nodes[nodeIndex].offset + changes[j].y] = UINT32_MAX;
      
      if (nodeIndex != changes[j].z) continue;
      
      const uint32_t id = currentIndex;
      ++currentIndex;
      
      indices2[newOffset+id] = changes[j].w;

      proxies[changes[j].w].setNodeIndex(nodeIndex);
      proxies[changes[j].w].setContainerIndex(id);
    }

    if (countThatStay != 0) {
      for (uint32_t j = 0; j < nodes[nodeIndex].count; ++j) {
        const uint32_t index = indices1[nodes[nodeIndex].offset + j];
        
        if (index == UINT32_MAX) continue;
        
        const uint32_t id = currentIndex;
        ++currentIndex;
        
        indices2[newOffset+id] = index;

        proxies[index].setNodeIndex(nodeIndex);
        proxies[index].setContainerIndex(id);
      }
    }
    
    assert(newSize < 10000);
    assert(newOffset < 10000);

    nodes[nodeIndex].count = newSize;
    nodes[nodeIndex].offset = newOffset;
  }
  
//   for (uint32_t i = 0; i < 5111; ++i) {
//     assert(indices2[i] < 5111 + nodesCount);
//     assert(indices2[i] >= nodesCount);
//   }

  auto end = std::chrono::steady_clock::now() - start;
  auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
//   std::cout << "Update octree time : " << mcs << " mcs" << '\n';
}

void CPUOctreeBroadphase::calculateOverlappingPairs() {
  auto start = std::chrono::steady_clock::now();

  // std::cout << "Obj count " << toPairsCalculate[0] << '\n';
  // if (toPairsCalculate[0] == 0) throw std::runtime_error("bad obj count");
  // for (uint32_t i = 1; i < toPairsCalculate[0]+1; ++i) {
  //   std::cout << "proxy index " << toPairsCalculate[i] << '\n';
  // }

  overlappingPairCache->at(0).firstIndex = 0;
  overlappingPairCache->at(0).secondIndex = 0;
  overlappingPairCache->at(0).dist = 0;
  overlappingPairCache->at(0).islandIndex = 0;
  
  staticOverlappingPairCache->at(0).firstIndex = 0;
  staticOverlappingPairCache->at(0).secondIndex = 0;
  staticOverlappingPairCache->at(0).dist = 0;
  staticOverlappingPairCache->at(0).islandIndex = 0;

  static uint32_t stack[512];

  // for (uint32_t i = 0; i < nodesCount; ++i) {
  //   std::cout << "node index " << i << '\n';
  //   std::cout << "node child " << nodes[i].childIndex << '\n';
  // }

  for (uint32_t currentIndex = 1; currentIndex < toPairsCalculate[0]+1; ++currentIndex) {
    const uint32_t currentProxyIndex = toPairsCalculate[currentIndex];

    const CPUOctreeProxy &current = proxies[currentProxyIndex];
    const FastAABB &first = current.getAABB();

    stack[0] = 0;
    uint32_t memOffset = 1;
    uint32_t globalObjCount = nodes[0].count;

    uint32_t lastMemory = 0;

    for (uint32_t i = 1; i < depth; ++i) {
      const uint32_t currentOffset = memOffset;
      const uint32_t checkCount = (memOffset - lastMemory) * 8;

      for (uint32_t j = 0; j < checkCount; ++j) {
        const uint32_t index = j;

        const uint32_t parentIndex = stack[index / 8 + lastMemory];
        const uint32_t childIndex = nodes[parentIndex].childIndex + index % 8;

        // std::cout << '\n';
        // std::cout << "parentIndex " << parentIndex << " depth " << i << '\n';
        // std::cout << "nodes[parentIndex].childIndex " << nodes[parentIndex].childIndex << '\n';
        // std::cout << "childIndex + index % 8        " << childIndex << '\n';
        const CPUOctreeNode &node = nodes[childIndex];
        const FastAABB &nodeBox = proxies[node.proxyIndex].getAABB();
        if (testAABB(nodeBox, first)) {
          const uint32_t id = memOffset;
          ++memOffset;
          
          stack[id] = childIndex;

          const uint32_t nodeObjCount = node.count;
          globalObjCount += nodeObjCount;
        }
      }

      lastMemory = currentOffset;
    }

    uint32_t stackIndex = 0;
    uint32_t objCount = nodes[stack[stackIndex]].count;
    for (uint32_t i = 0; i < globalObjCount; ++i) {
      const uint32_t threadIndex = i;

      uint32_t nodeIndex = stack[stackIndex];
      CPUOctreeNode node = nodes[nodeIndex];
      //uint32_t objCount = node.data.x;
      while (objCount <= threadIndex) {
        ++stackIndex;
        //if (stackIndex >= memoryOffset) break; // по идее это не должно произойти
        nodeIndex = stack[stackIndex];
        node = nodes[nodeIndex];
        objCount += node.count;
      }

      // на это обратить пристальное внимание
      // (так же обратить внимание на эту -1)
      // (threadIndex всегда <= objCount, а это значит 0 мы здесь не получим, а выйти за пределы - запросто)
      // нет, скорее всего -1 здесь не нужна
      const uint32_t index = node.offset + (objCount - threadIndex - 1);
      // std::cout << "Index " << index << '\n';
      if (index > 6000) throw std::runtime_error("Index > 6000");
      const uint32_t proxyIndex = indices2[index];

      if (currentProxyIndex == proxyIndex) continue;

      const CPUOctreeProxy &p = proxies[proxyIndex];
      if (proxyIndex < nodesCount) throw std::runtime_error("proxyIndex < nodesCount");

      if ((!p.getType().isTrigger()) && (!p.getType().isBlocking())) continue;

      //const AABB second = boxes[objects[objectIndex].additionalData.w];
      const FastAABB &second = p.getAABB();

                    // x - это тип объекта, y - это те объекты с которыми есть коллизия
      const bool collide = (current.collisionGroup() & p.collisionFilter()) != 0 &&
                           (p.collisionGroup() & current.collisionFilter()) != 0;

      const bool currentObjTrigget = current.getType().isTrigger() && (!current.getType().isBlocking());
      const bool proxyTrigget = p.getType().isTrigger() && (!p.getType().isBlocking());
      const bool triggerOnly = currentObjTrigget || proxyTrigget;
      
//       if () {
//         std::cout << "\n";
//         std::cout << "first  center x: " << first.center.x << " y: " << first.center.y << " z: " << first.center.z << "\n";
//         std::cout << "first  extent x: " << first.extent.x << " y: " << first.extent.y << " z: " << first.extent.z << "\n";
//         std::cout << "second center x: " << second.center.x << " y: " << second.center.y << " z: " << second.center.z << "\n";
//         std::cout << "second extent x: " << second.extent.x << " y: " << second.extent.y << " z: " << second.extent.z << "\n";
//       }
      
      const bool dynamicPair = current.getType().isDynamic() && p.getType().isDynamic();

      if (collide && testAABB(first, second)) {
        //if (p.getObjectIndex() == 3) throw std::runtime_error("3");
        
        // const uint32_t id = atomicAdd(count.x, 1);

        // // тут наверное еще нужно расположить минимальный индекс первым
        // pairs[id].data.x = current.proxyData.w;
        // pairs[id].data.y = p.proxyData.w;
        // const float dist2 = distance2(first.center, second.center);
        // pairs[id].data.z = floatBitsToUint(dist2);
        // pairs[id].data.w = triggerOnly ? ONLY_TRIGGER_ID : 0;
        // //pairs[id].data.w = triggerOnly ? ONLY_TRIGGER_ID : nodeCount-min(current.octreeData.x, p.octreeData.x);
        
        if (dynamicPair) {
          const uint32_t id = overlappingPairCache->at(0).firstIndex;
          ++overlappingPairCache->at(0).firstIndex;
          
          const uint32_t firstIndex  = glm::min(current.getObjectIndex(), p.getObjectIndex());
          const uint32_t secondIndex = glm::max(current.getObjectIndex(), p.getObjectIndex());
//           objPairs[id+1].firstIndex = current.getObjectIndex();
//           objPairs[id+1].secondIndex = p.getObjectIndex();
          overlappingPairCache->at(id+1).firstIndex = firstIndex;
          overlappingPairCache->at(id+1).secondIndex = secondIndex;
          const float dist2 = glm::distance2(first.center, second.center);
          overlappingPairCache->at(id+1).dist = dist2;
          //objPairs[id+1].islandIndex = triggerOnly ? ONLY_TRIGGER_ID : 0;
          overlappingPairCache->at(id+1).islandIndex = triggerOnly ? ONLY_TRIGGER_ID : nodesCount - glm::min(current.getNodeIndex(), p.getNodeIndex()) - 1;
          
          // до нарров фазы мы именно эти пары мы должны отсортировать по индексам!!!
          // а после нарров фазы отсортировать по индексам островов
          // но в этом случае мне еще нужно по уровням октодерева распределить 
        } else {          
          const uint32_t id = staticOverlappingPairCache->at(0).firstIndex;
          ++staticOverlappingPairCache->at(0).firstIndex;

          staticOverlappingPairCache->at(id+1).firstIndex = current.getObjectIndex();
          staticOverlappingPairCache->at(id+1).secondIndex = p.getObjectIndex();
          const float dist2 = glm::distance2(first.center, second.center);
          staticOverlappingPairCache->at(id+1).dist = dist2;
          staticOverlappingPairCache->at(id+1).islandIndex = triggerOnly ? ONLY_TRIGGER_ID : current.getObjectIndex();
          // эти пары похоже что нужно отсортировать только раз
          // и тоже нужно распределить по островам
        }

//         std::cout << '\n';
//         std::cout << "Pair " << id << '\n';
//         std::cout << "first index  " << objPairs[id+1].firstIndex << '\n';
//         std::cout << "second index " << objPairs[id+1].secondIndex << '\n';
//         std::cout << "dist         " << objPairs[id+1].dist << '\n';
//         
//         std::cout << "node index  " << p.getNodeIndex() << "\n";
//         std::cout << "node offset " << nodes[p.getNodeIndex()].offset << '\n';
//         std::cout << "node count  " << nodes[p.getNodeIndex()].count << '\n';
        
        //if (p.getObjectIndex() == 5010) throw std::runtime_error("5010");
        
        //if (p.getObjectIndex() == 5106) throw std::runtime_error("5106");
      }
    }
  }

  // потом надо будет почекать как можно устроить параллельную сортировку на цпу
  overlappingPairCache->at(0).islandIndex = overlappingPairCache->at(0).firstIndex;
//   static bool noObj = true;
//   if (objPairs[0].firstIndex != 0) {
//     noObj = false;
//   }
//   
//   if (!noObj) {
//     std::cout << "Pair count " << objPairs[0].firstIndex << '\n';
//     noObj = true;
//     throw std::runtime_error("found pairs");
//   }

  auto end = std::chrono::steady_clock::now() - start;
  auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
//   std::cout << "Pairs calc time : " << mcs << " mcs" << '\n';
}

void CPUOctreeBroadphase::calculateRayTests() {
  auto start = std::chrono::steady_clock::now();

  static uint32_t stack[512];

  for (uint32_t currentIndex = 0; currentIndex < raysCount; ++currentIndex) {
    //const uint32_t currentProxyIndex = toPairsCalculate[currentIndex];

    const RayData &current = rays->at(currentIndex);

    stack[0] = 0;
    uint32_t memOffset = 1;
    uint32_t globalObjCount = nodes[0].count;

    uint32_t lastMemory = 0;

    for (uint32_t i = 1; i < depth; ++i) {
      const uint32_t currentOffset = memOffset;
      const uint32_t checkCount = (memOffset - lastMemory) * 8;

      for (uint32_t j = 0; j < checkCount; ++j) {
        const uint32_t index = j;

        const uint32_t parentIndex = stack[index + lastMemory];
        const uint32_t childIndex = nodes[parentIndex].childIndex + index % 8;

        const CPUOctreeNode &node = nodes[childIndex];
        const FastAABB &nodeBox = proxies[node.proxyIndex].getAABB();
        if (intersection(nodeBox, current)) {
          const uint32_t id = memOffset;
          stack[id] = childIndex;
          ++memOffset;

          const uint32_t nodeObjCount = node.count;
          globalObjCount += nodeObjCount;
        }
      }

      lastMemory = currentOffset;
    }

    uint32_t stackIndex = 0;
    uint32_t objCount = nodes[stack[stackIndex]].count;
    for (uint32_t i = 0; i < globalObjCount; ++i) {
      const uint32_t threadIndex = i;

      uint32_t nodeIndex = stack[stackIndex];
      CPUOctreeNode node = nodes[nodeIndex];
      //uint32_t objCount = node.data.x;
      while (objCount < threadIndex) {
        ++stackIndex;
        //if (stackIndex >= memoryOffset) break; // по идее это не должно произойти
        nodeIndex = stack[stackIndex];
        node = nodes[nodeIndex];
        objCount += node.count;
      }

      // на это обратить пристальное внимание
      // (так же обратить внимание на эту -1)
      // (threadIndex всегда <= objCount, а это значит 0 мы здесь не получим, а выйти за пределы - запросто)
      // нет, скорее всего -1 здесь не нужна
      const uint32_t index = node.offset + (objCount - threadIndex - 1);
      //atomicMax(count.y, index);
      const uint32_t proxyIndex = indices2[index];

      const CPUOctreeProxy &p = proxies[proxyIndex];

      const FastAABB &second = p.getAABB();

      if (!p.getType().canBlockRays()) continue;

      if (intersection(second, current)) {
        // const uint32_t id = atomicAdd(count.x, 1);

        // // тут наверное еще нужно расположить минимальный индекс первым
        // pairs[id].data.x = current.proxyData.w;
        // pairs[id].data.y = p.proxyData.w;
        // const float dist2 = distance2(first.center, second.center);
        // pairs[id].data.z = floatBitsToUint(dist2);
        // pairs[id].data.w = triggerOnly ? ONLY_TRIGGER_ID : 0;
        // //pairs[id].data.w = triggerOnly ? ONLY_TRIGGER_ID : nodeCount-min(current.octreeData.x, p.octreeData.x);

        const uint32_t id = rayPairCache->at(0).firstIndex;
        ++rayPairCache->at(0).firstIndex;

        rayPairCache->at(id+1).firstIndex = currentIndex;
        rayPairCache->at(id+1).secondIndex = p.getObjectIndex();
        const float dist2 = glm::distance2(current.pos, second.center);
        rayPairCache->at(id+1).dist = dist2;
        rayPairCache->at(id+1).islandIndex = currentIndex;
      }
    }
  }

  auto end = std::chrono::steady_clock::now() - start;
  auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  std::cout << "Rays calc time : " << mcs << " mcs" << '\n';
}

void CPUOctreeBroadphase::calculateFrustumTests() {
  auto start = std::chrono::steady_clock::now();

  for (uint32_t i = 0; i < frustumCount; ++i) {
    uint32_t objCount = 0;
    const uint32_t frustumIndex = i;

    const FrustumStruct &frustum = frustums->at(frustumIndex);
    const simd::vec4 &pos = frustumPoses->at(frustumIndex);

    for (uint32_t j = 0; j < frustumIterativeNodeCount; ++j) {
      objCount += nodes[j].count;
    }

    uint32_t nodeIndex = 0;
    uint32_t objCountLocal = nodes[nodeIndex].count;
    for (uint32_t j = 0; j < objCount; ++j) {
      const uint32_t threadIndex = j;

      CPUOctreeNode node = nodes[nodeIndex];
      while (objCountLocal < threadIndex) {
        ++nodeIndex;
        node = nodes[nodeIndex];
        objCountLocal += node.count;
      }

      const uint32_t index = node.offset + (objCountLocal - threadIndex - 1);
      const uint32_t proxyIndex = indices2[index];

      const CPUOctreeProxy &proxy = proxies[proxyIndex];
      if (!proxy.getType().isVisible()) continue;

      const uint32_t res = testFrustumAABB(frustum, proxy.getAABB());

      if (res > OUTSIDE) {
        const uint32_t id = frustumTestsResult->at(0).firstIndex;
        ++frustumTestsResult->at(0).firstIndex;

        frustumTestsResult->at(id+1).firstIndex = frustumIndex;
        frustumTestsResult->at(id+1).secondIndex = proxy.getObjectIndex();
        const float dist2 = glm::distance2(pos, proxy.getAABB().center);
        frustumTestsResult->at(id+1).dist = dist2;
        frustumTestsResult->at(id+1).islandIndex = frustumIndex;
      }
    }
  }

  static std::vector<uint32_t> stack;
  stack.resize(64+1);
  for (uint32_t frustumIndex = 0; frustumIndex < frustumCount; ++frustumIndex) {
    for (uint32_t k = 0; k < frustumDispatchNodeCount; ++k) {
      const uint32_t nodeIndex = frustumIterativeNodeCount + k;

      const FrustumStruct &frustum = frustums->at(frustumIndex);
      const simd::vec4 &pos = frustumPoses->at(frustumIndex);

      uint32_t memOffset = 1;
      stack[0] = nodeIndex;

      uint32_t globalObjCount = nodes[nodeIndex].count;

      uint32_t lastMemory = 0;
      const uint32_t currentMaxDepth = 2; // нужно сделать глобальным

      // проверка на полное вхождение нода и ранний выход
      const CPUOctreeNode &node = nodes[nodeIndex];
      const FastAABB &nodeBox = proxies[node.proxyIndex].getAABB();

      const uint32_t testRes = testFrustumAABB(frustum, nodeBox);
      if (testRes == OUTSIDE) continue;

      if (testRes == INSIDE) {
        // тут мы можем параллельно обойти всех детей
        addEveryObj(nodeIndex, currentMaxDepth, frustumIndex, pos);

        continue;
      }

      for (uint32_t i = 0; i < currentMaxDepth; ++i) {
        const uint32_t currentOffset = memOffset;
        const uint32_t checkCount = (memOffset - lastMemory) * 8;

        for (uint32_t j = 0; j < checkCount; ++j) {
          const uint32_t index = j;
          const uint32_t parentIndex = stack[index + lastMemory];
          const uint32_t childIndex = nodes[parentIndex].childIndex + index % 8;

          const CPUOctreeNode &node = nodes[childIndex];
          const FastAABB nodeBox = proxies[node.proxyIndex].getAABB();

          const uint32_t testRes = testFrustumAABB(frustum, nodeBox);

          if (testRes == INSIDE) {
            // то мы обходим все объекты нода и его потомка и добавляем их
            // просто итеративно (НЕ многопоточно я имею ввиду)
            addEveryObjIterative(childIndex, currentMaxDepth-i, frustumIndex, pos);
          }

          if (testRes == INTERSECT) {
            const uint32_t id = memOffset;
            ++memOffset;
            stack[id] = childIndex;

            const uint32_t nodeObjCount = node.count;
            globalObjCount += nodeObjCount;
          }
        }

        lastMemory = currentOffset;
      }

      uint32_t stackIndex = 0;
      uint32_t objCount = nodes[stack[stackIndex]].count;
      for (uint32_t i = 0; i < globalObjCount; ++i) {
        const uint32_t threadIndex = i;

        uint32_t nodeIndex = stack[stackIndex];
        CPUOctreeNode node = nodes[nodeIndex];
        //uint objCount = node.data.x;
        while (objCount < threadIndex) {
          ++nodeIndex;
          //if (stackIndex >= memoryOffset) break; // по идее это не должно произойти
          nodeIndex = stack[stackIndex];
          node = nodes[nodeIndex];
          objCount += node.count;
        }

        // на это обратить пристальное внимание
        // (так же обратить внимание на эту -1)
        // (threadIndex всегда <= objCount, а это значит 0 мы здесь не получим, а выйти за пределы - запросто)
        const uint32_t index = node.offset + (objCount - threadIndex - 1);
        const uint32_t proxyIndex = indices2[index];

        const CPUOctreeProxy &proxy = proxies[proxyIndex];
        if (!proxy.getType().isVisible()) continue;

        const uint32_t res = testFrustumAABB(frustum, proxy.getAABB());

        if (res > OUTSIDE) {
          const uint32_t id = frustumTestsResult->at(0).firstIndex;
          ++frustumTestsResult->at(0).firstIndex;

          frustumTestsResult->at(id+1).firstIndex = frustumIndex;
          frustumTestsResult->at(id+1).secondIndex = proxy.getObjectIndex();
          const float dist = simd::distance2(pos, proxy.getAABB().center);
          frustumTestsResult->at(id+1).dist = dist;
          frustumTestsResult->at(id+1).islandIndex = frustumIndex;
        }
      }
    }
  }

  auto end = std::chrono::steady_clock::now() - start;
  auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  std::cout << "Frustum calc time : " << mcs << " mcs" << '\n';
}

void CPUOctreeBroadphase::postlude() {

}

// ArrayInterface<BroadphasePair>* CPUOctreeBroadphase::getOverlappingPairCache() {
//   return &objPairs;
// }
// 
// const ArrayInterface<BroadphasePair>* CPUOctreeBroadphase::getOverlappingPairCache() const {
//   return &objPairs;
// }
// 
// ArrayInterface<BroadphasePair>* CPUOctreeBroadphase::getStaticOverlappingPairCache() {
//   return &staticObjPairs;
// }
// 
// const ArrayInterface<BroadphasePair>* CPUOctreeBroadphase::getStaticOverlappingPairCache() const {
//   return &staticObjPairs;
// }
// 
// ArrayInterface<BroadphasePair>* CPUOctreeBroadphase::getRayPairCache() {
//   return &rayPairs;
// }
// 
// const ArrayInterface<BroadphasePair>* CPUOctreeBroadphase::getRayPairCache() const {
//   return &rayPairs;
// }
// 
// ArrayInterface<BroadphasePair>* CPUOctreeBroadphase::getFrustumTestsResult() {
//   return &frustumPairs;
// }
// 
// const ArrayInterface<BroadphasePair>* CPUOctreeBroadphase::getFrustumTestsResult() const {
//   return &frustumPairs;
// }

void CPUOctreeBroadphase::printStats() {
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
  std::cout << "Broadphase class size  " << sizeof(CPUOctreeBroadphase) << " bytes" << '\n';
  
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

    objCount += nodes[i].count;
    maxOffset = glm::max(maxOffset, nodes[i].offset);
  }

  uint32_t abnormalOffsetIndex = 0;
  for (uint32_t i = 0; i < nodesCount; ++i) {
    if (nodes[i].offset >= 1000) {
      abnormalOffsetIndex = i;
      break;
    }
  }

  std::cout << "Recalc objCount " << objCount << '\n';
  std::cout << "Max offset " << maxOffset << '\n';
  std::cout << "Abnormal offset index " << abnormalOffsetIndex << '\n';
  std::cout << "Abnormal offset " << nodes[abnormalOffsetIndex].offset << '\n';
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
    objsInNodes += nodes[i].count;
    if (nodes[i].count > maxObjInNode) {
      maxObjInNode = nodes[i].count;
      index = i;
    }
  }

  PRINT_VAR("Avg obj in node", float(objsInNodes) / float(nodesCount))
  PRINT_VAR("Max obj in node", maxObjInNode)
  PRINT_VAR("Node idx with max", index)
  PRINT_VAR("Max node offset", nodes[index].offset)
  PRINT_VEC4("Max node center", proxies[nodes[index].proxyIndex].getAABB().center)
  PRINT_VEC4("Max node extent", proxies[nodes[index].proxyIndex].getAABB().extent)

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

// simd::vec4 getVertex(const simd::vec4 &pos, const simd::vec4 &ext, const glm::mat4 &orn, const uint32_t &index) {
//   simd::vec4 p = pos;
//   p = (index & 1) > 0 ? p + orn[0]*ext.x : p - orn[0]*ext.x;
//   p = (index & 2) > 0 ? p + orn[1]*ext.y : p - orn[1]*ext.y;
//   p = (index & 4) > 0 ? p + orn[2]*ext.z : p - orn[2]*ext.z;
//   p.w = 1.0f;
// 
//   return p;
// }

// FastAABB recalcAABB(const simd::vec4 &pos, const simd::vec4 &ext, const glm::mat4 &orn) {
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
// FastAABB recalcAABB(const ArrayInterface<simd::vec4>* verticies, const simd::vec4 &pos, const uint32_t &vertexOffset, const uint32_t &vertexSize, const glm::mat4 &orn) {
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
// uint32_t testFrustumAABB(const FrustumStruct &frustum, const FastAABB &box) {
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

void CPUOctreeBroadphase::addEveryObj(const uint32_t &mainNodeIndex, const uint32_t &depth, const uint32_t &frustumIndex, const simd::vec4 &frustumPos) {
  uint32_t nodeCount = 1;
  for (uint32_t i = 0; i < depth; ++i) {
    for (uint32_t j = 0; j < nodeCount; ++j) {
      const uint32_t index = j;

      uint32_t childIndex = index % 8;
      CPUOctreeNode &data = nodes[mainNodeIndex];
      for (uint32_t k = 0; k < i; ++k) {
        childIndex = (index >> k*3) % 8;
        data = nodes[data.childIndex + childIndex];
      }

      for (uint32_t k = 0; k < data.count; ++k) {
        const uint32_t proxyIndex = indices2[data.offset + k];
        const CPUOctreeProxy &proxy = proxies[proxyIndex];

        if (!proxy.getType().isVisible()) continue;

        const uint32_t id = frustumTestsResult->at(0).firstIndex;
        ++frustumTestsResult->at(0).firstIndex;

        frustumTestsResult->at(id+1).firstIndex = frustumIndex;
        frustumTestsResult->at(id+1).secondIndex = proxy.getObjectIndex();
        const float dist = simd::distance2(frustumPos, proxy.getAABB().center);
        frustumTestsResult->at(id+1).dist = dist;
        frustumTestsResult->at(id+1).islandIndex = frustumIndex;
      }
    }

    nodeCount *= 8;
  }
}

void CPUOctreeBroadphase::addEveryObjIterative(const uint32_t &mainNodeIndex, const uint32_t &depth, const uint32_t &frustumIndex, const simd::vec4 &frustumPos) {
  uint32_t nodeCount = 1;

  for (uint32_t i = 0; i < depth; ++i) {
    for (uint32_t j = 0; j < nodeCount; ++j) {
      uint32_t childIndex = j % 8;
      CPUOctreeNode &data = nodes[mainNodeIndex]; //nodes[node.data.w + childIndex];
      for (uint32_t k = 0; k < i; ++k) {
        childIndex = (j >> k*3) % 8;
        data = nodes[data.childIndex + childIndex];
      }

      for (uint32_t k = 0; k < data.count; ++k) {
        const uint32_t proxyIndex = indices2[data.offset + k];
        const CPUOctreeProxy &proxy = proxies[proxyIndex];

        if (!proxy.getType().isVisible()) continue;

        const uint32_t id = frustumTestsResult->at(0).firstIndex;
        ++frustumTestsResult->at(0).firstIndex;

        frustumTestsResult->at(id+1).firstIndex = frustumIndex;
        frustumTestsResult->at(id+1).secondIndex = proxy.getObjectIndex();
        const float dist = simd::distance2(frustumPos, proxy.getAABB().center);
        frustumTestsResult->at(id+1).dist = dist;
        frustumTestsResult->at(id+1).islandIndex = frustumIndex;
      }
    }

    nodeCount *= 8;
  }
}

