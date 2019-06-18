#include "GPUOctreeBroadphase.h"
#include "Globals.h"
#include <chrono>

GPUOctreeProxy::GPUOctreeProxy(const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter) :
                  BroadphaseProxy(objIndex, type, collisionGroup, collisionFilter) {
  nodeIndex = UINT32_MAX;
  containerIndex = UINT32_MAX-1;
  //objType = type;

  dummy2 = UINT32_MAX;
}

GPUOctreeProxy::~GPUOctreeProxy() {}

uint32_t GPUOctreeProxy::getNodeIndex() const {
  return nodeIndex;
}

uint32_t GPUOctreeProxy::getContainerIndex() const {
  return containerIndex;
}

void GPUOctreeProxy::setNodeIndex(const uint32_t &index) {
  nodeIndex = index;
}

void GPUOctreeProxy::setContainerIndex(const uint32_t &index) {
  containerIndex = index;
}

void GPUOctreeProxy::updateAABB() {

}

GPUOctreeBroadphase::GPUOctreeBroadphase(const GPUOctreeBroadphaseCreateInfo &info) {
  construct(info);
}

GPUOctreeBroadphase::~GPUOctreeBroadphase() {
  device->destroy(calcProxy);
  device->destroy(calcChanges);
  device->destroy(updateOctree);

  device->destroyLayout("pair_calc_layout");
  device->destroyLayout("changes_calc_layout");
  device->destroyLayout("update_octree_layout");
}

// void GPUOctreeBroadphase::setCollisionTestBuffer(ArrayInterface<uint32_t>* indexBuffer) {
//   indexBuffer->descriptorPtr(&indexBufferDesc);
// }
// 
// void GPUOctreeBroadphase::setRayTestBuffer(ArrayInterface<RayData>* rayBuffer) {
//   rayBuffer->descriptorPtr(&rayBufferDesc);
// }
// 
// void GPUOctreeBroadphase::setUpdateBuffers(const UpdateBuffers &buffers) {
//   buffers.objects->descriptorPtr(&objectsDataDesc);
//   buffers.systems->descriptorPtr(&matricesDesc);
//   buffers.transforms->descriptorPtr(&transformsDesc);
//   buffers.rotationDatas->descriptorPtr(&rotationDatasDesc);
// 
//   buffers.rays->descriptorPtr(&rayDesc);
//   buffers.frustums->descriptorPtr(&frustumDesc);
//   buffers.frustumPoses->descriptorPtr(&frustumPosesDesc);
// 
//   // Нужно еще предусмотреть случай если данные объектов будут в разных дескрипторах
// }

void GPUOctreeBroadphase::setInputBuffers(const InputBuffers &buffers) {
  buffers.indexBuffer->gpu_buffer(&indexBufferDesc);
  buffers.objects->gpu_buffer(&objectsDataDesc);
  buffers.systems->gpu_buffer(&matricesDesc);
  buffers.transforms->gpu_buffer(&transformsDesc);
  buffers.rotationDatas->gpu_buffer(&rotationDatasDesc);

  buffers.rays->gpu_buffer(&rayDesc);
  buffers.frustums->gpu_buffer(&frustumDesc);
  buffers.frustumPoses->gpu_buffer(&frustumPosesDesc);
}

void GPUOctreeBroadphase::setOutputBuffers(const OutputBuffers &buffers, void* indirectPairBuffer) {
  (void)indirectPairBuffer;
  
  overlappingPairCache = buffers.overlappingPairCache;
  staticOverlappingPairCache = buffers.staticOverlappingPairCache;
  rayPairCache = buffers.rayPairCache;
  frustumTestsResult = buffers.frustumTestsResult;
  
  yavf::Buffer* buffer;
  buffers.overlappingPairCache->gpu_buffer(&buffer);
  overlappingPairCacheDesc = buffer->descriptorSet();
  buffers.staticOverlappingPairCache->gpu_buffer(&buffer);
  staticOverlappingPairCacheDesc = buffer->descriptorSet();
  buffers.rayPairCache->gpu_buffer(&buffer);
  rayPairCacheDesc = buffer->descriptorSet();
  buffers.frustumTestsResult->gpu_buffer(&buffer);
  frustumTestsResultDesc = buffer->descriptorSet();
}

uint32_t GPUOctreeBroadphase::createProxy(const FastAABB &box, const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter) {
  uint32_t index;

  if (freeProxy != UINT32_MAX) {
    index = freeProxy;
    freeProxy = proxies[index].dummy2;
    proxies[index] = GPUOctreeProxy(objIndex, type, collisionGroup, collisionFilter);
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

BroadphaseProxy* GPUOctreeBroadphase::getProxy(const uint32_t &index) {
  return &proxies[index];
}

void GPUOctreeBroadphase::destroyProxy(BroadphaseProxy* proxy) {
  GPUOctreeProxy* p = (GPUOctreeProxy*)proxy;

  const uint32_t id = changes[0].y;
  ++changes[0].y;

  changes[id+1].x = p->nodeIndex;
  changes[id+1].y = p->containerIndex;
  changes[id+1].z = UINT32_MAX;
  changes[id+1].w = p->objIndex;

  p->dummy2 = freeProxy;
  freeProxy = p->proxyIndex;
  p->proxyIndex = UINT32_MAX;
  p->containerIndex = UINT32_MAX;
}

void GPUOctreeBroadphase::destroyProxy(const uint32_t &index) {
  const uint32_t id = changes[0].y;
  ++changes[0].y;

  changes[id+1].x = proxies[index].nodeIndex;
  changes[id+1].y = proxies[index].containerIndex;
  changes[id+1].z = UINT32_MAX;
  changes[id+1].w = proxies[index].objIndex;

  proxies[index].dummy2 = freeProxy;
  freeProxy = index;
  proxies[index].containerIndex = UINT32_MAX;
  proxies[index].objIndex = UINT32_MAX;
}

void GPUOctreeBroadphase::updateBuffers(const uint32_t &objCount, const uint32_t &dynObjCount, const uint32_t &raysCount, const uint32_t &frustumsCount) {
  if (proxies.size() < objCount) proxies.reserve(objCount);
  if (changes.size() < objCount) changes.resize(objCount);
  if (indices1.size() < objCount) indices1.resize(objCount);
  if (indices2.size() < objCount) indices2.resize(objCount);
  if (proxyIndices.size() < objCount+1) proxyIndices.resize(objCount+1);
  if (toPairsCalculate.size() < dynObjCount+1) toPairsCalculate.resize(dynObjCount+1);

  uint32_t v = dynObjCount*2; // compute the next highest power of 2 of 32-bit v
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
//   if (objPairs.size() < v) objPairs.resize(v);
//   if (rayPairs.size() < raysCount*3) rayPairs.resize(raysCount*3);
//   if (frustumPairs.size() < frustumsCount*(objCount/2)) frustumPairs.resize(frustumsCount*(objCount/2));
  
  if (overlappingPairCache->size() < v) overlappingPairCache->resize(v);
  if (staticOverlappingPairCache->size() < v) staticOverlappingPairCache->resize(v);
  if (rayPairCache->size() < raysCount*3) rayPairCache->resize(raysCount*3);
  if (frustumTestsResult->size() < frustumsCount*(objCount/2)) frustumTestsResult->resize(frustumsCount*(objCount/2));

  this->raysCount = raysCount;
  this->frustumCount = frustumsCount;

  // и не здесь
  //changes[0].x = 0;
}

void GPUOctreeBroadphase::update() {
  // можно передавать ТОЛЬКО дескрипторы (это в основном касается только ГПУ физики)
//   yavf::Descriptor objectDesc = VK_NULL_HANDLE;
//   objects->descriptorPtr(&objectDesc);
//   yavf::Descriptor transformDesc = VK_NULL_HANDLE;
//   transforms->descriptorPtr(&transformDesc);
//   yavf::Descriptor indexDesc = VK_NULL_HANDLE;
//   indexBuffer->descriptorPtr(&indexDesc);

  task->begin();

  task->setPipeline(calcProxy);
  //task->setDescriptor({objectDesc, transformDesc, indexDesc, proxies.descriptorSet(), proxyIndexies.descriptorSet()}, 0);
  task->setDescriptor({objectsDataDesc->handle(), 
                       matricesDesc->handle(), 
                       transformsDesc->handle(), 
                       rotationDatasDesc->handle(), 
                       indexBufferDesc->handle(), 
                       proxies.descriptorSet()->handle(), 
                       proxyIndices.descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);

  // {
  //   const std::vector<VkBufferMemoryBarrier> barriers{
  //     {
  //       VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
  //       nullptr,
  //       VK_ACCESS_MEMORY_WRITE_BIT,
  //       VK_ACCESS_MEMORY_READ_BIT,
  //       VK_QUEUE_FAMILY_IGNORED,
  //       VK_QUEUE_FAMILY_IGNORED,
  //       proxies.handle(),
  //       0,
  //       proxies.buffer_size()
  //     },
  //     {
  //       VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
  //       nullptr,
  //       VK_ACCESS_MEMORY_WRITE_BIT,
  //       VK_ACCESS_MEMORY_READ_BIT,
  //       VK_QUEUE_FAMILY_IGNORED,
  //       VK_QUEUE_FAMILY_IGNORED,
  //       proxyIndexies.handle(),
  //       0,
  //       proxyIndexies.buffer_size()
  //     }
  //   };

  //   task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  //                    {},
  //                    barriers,
  //                    {});
  // }

  task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->end();
  
  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Proxy recalc time: " << ns << " mcs" << "\n";

  task->begin();

  // здесь еще будет обновление октодерева
  task->setPipeline(calcChanges);
  task->setDescriptor({nodes.descriptorSet()->handle(),
                       changes.descriptorSet()->handle(),
                       proxies.descriptorSet()->handle(),
                       proxyIndices.descriptorSet()->handle(),
                       toPairsCalculate.descriptorSet()->handle(),
                       indirectBuffer->descriptorSet()->handle()}, 0);
  //task->setDescriptor(toPairsCalculate.descriptorSet(), 5);
  task->dispatch(1, 1, 1);

  task->copy(indices2.handle(), indices1.handle(), 0, 0, indices2.buffer_size());

  task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->end();
  
  start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  end = std::chrono::steady_clock::now() - start;
  ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Calc changes time: " << ns << " mcs" << "\n";

  // task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->begin();

  task->setPipeline(preUpdateOctree);
  task->setDescriptor(nodes.descriptorSet(), 0);
  task->dispatch(nodesCount, 1, 1);

  task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->setPipeline(updateOctree);
  task->setDescriptor({nodes.descriptorSet()->handle(),
                       indices1.descriptorSet()->handle(),
                       changes.descriptorSet()->handle(),
                       proxies.descriptorSet()->handle(),
                       indices2.descriptorSet()->handle()}, 0);
  task->dispatch(nodesCount, 1, 1);

  task->end();
  
  start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  end = std::chrono::steady_clock::now() - start;
  ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Update octree indices time: " << ns << " mcs" << "\n";

  // task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  // если я хочу каждый раз не пересобирать таск, то мне необходимо позаботиться о том, чтобы toggleIndiciesBuffer не было вовсе

  // task эндим не здесь, хотя в будущем можно будет подумать об параллельном вычислении некоторых стадий
}

void GPUOctreeBroadphase::calculateOverlappingPairs() {
  // VkDispatchIndirectCommand* c = (VkDispatchIndirectCommand*)indirectBuffer->ptr();
  // std::cout << "Indirect command x:" << c->x << '\n';
  // std::cout << "Indirect command y:" << c->y << '\n';
  // std::cout << "Indirect command z:" << c->z << '\n';

  task->begin();

  //task->update()

  task->setPipeline(calcPairs);
  task->setDescriptor({proxies.descriptorSet()->handle(), 
                       nodes.descriptorSet()->handle(),
                       indices2.descriptorSet()->handle(),
                       toPairsCalculate.descriptorSet()->handle(),
                       overlappingPairCacheDesc->handle(), //objPairs.vector().descriptorSet(),
                       indirectPairsCount->descriptorSet()->handle()}, 0);
  task->dispatchIndirect(indirectBuffer);

  // task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->end();
  
  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Calc pairs time: " << ns << " mcs" << "\n";

  // task->begin();

  // task->setPipeline(pairsToPowerOfTwo);
  // task->setDescriptor({objPairs.vector().descriptorSet(),
  //                      indirectPairsCount->descriptorSet()}, 0);
  // task->dispatch(1, 1, 1);

  // {
  //   const std::vector<VkBufferMemoryBarrier> barriers{
  //     {
  //       VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
  //       nullptr,
  //       VK_ACCESS_MEMORY_WRITE_BIT,
  //       VK_ACCESS_MEMORY_READ_BIT,
  //       VK_QUEUE_FAMILY_IGNORED,
  //       VK_QUEUE_FAMILY_IGNORED,
  //       objPairs.vector().handle(),
  //       0,
  //       objPairs.vector().buffer_size()
  //     },
  //     {
  //       VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
  //       nullptr,
  //       VK_ACCESS_MEMORY_WRITE_BIT,
  //       VK_ACCESS_MEMORY_READ_BIT,
  //       VK_QUEUE_FAMILY_IGNORED,
  //       VK_QUEUE_FAMILY_IGNORED,
  //       indirectPairsCount->get(),
  //       0,
  //       indirectPairsCount->param().size
  //     }
  //   };

  //   task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  //                    {},
  //                    barriers,
  //                    {});
  // }

  // task->end();
  
  // start = std::chrono::steady_clock::now();
  // task->start();
  // task->wait();
  // end = std::chrono::steady_clock::now() - start;
  // ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  // std::cout << "Post pairs calc time: " << ns << " mcs" << "\n";

  // task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
}

void GPUOctreeBroadphase::calculateRayTests() {
  task->begin();

  task->setPipeline(calcRayPairs);
  task->setDescriptor({proxies.descriptorSet()->handle(), 
                       nodes.descriptorSet()->handle(), 
                       indices2.descriptorSet()->handle(), 
                       rayDesc->handle(), 
                       rayPairCacheDesc->handle()}, 0);
  task->dispatch(raysCount, 1, 1);

  task->end();
  
  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Calc ray pairs time: " << ns << " mcs" << "\n";
}

void GPUOctreeBroadphase::calculateFrustumTests() {
  task->begin();

  task->setPipeline(iterativeFrustum);
  task->setDescriptor({proxies.descriptorSet()->handle(), nodes.descriptorSet()->handle(), indices2.descriptorSet()->handle(), frustumDesc->handle(), frustumPosesDesc->handle(), frustumTestsResultDesc->handle()}, 0);
  task->dispatch(frustumCount, 1, 1);

  task->setPipeline(octreeFrustum);
  task->setDescriptor({proxies.descriptorSet()->handle(), 
                       nodes.descriptorSet()->handle(), 
                       indices2.descriptorSet()->handle(), 
                       frustumDesc->handle(), 
                       frustumPosesDesc->handle(), 
                       frustumTestsResultDesc->handle()}, 0);
  task->dispatch(frustumDispatchNodeCount, frustumCount, 1);

  task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->setPipeline(pairsToPowerOfTwo);
  task->setDescriptor(frustumTestsResultDesc/*frustumPairs.vector().descriptorSet()*/, 0);
  task->dispatch(1, 1, 1);

  task->end();
  
  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Calc frustum pairs time: " << ns << " mcs" << "\n";
}

void GPUOctreeBroadphase::postlude() {
  changes[0].x = 0;
}

// ArrayInterface<BroadphasePair>* GPUOctreeBroadphase::getOverlappingPairCache() {
//   return &objPairs;
// }
// 
// const ArrayInterface<BroadphasePair>* GPUOctreeBroadphase::getOverlappingPairCache() const {
//   return &objPairs;
// }
// 
// ArrayInterface<BroadphasePair>* GPUOctreeBroadphase::getRayPairCache()  {
//   return &rayPairs;
// }
// 
// const ArrayInterface<BroadphasePair>* GPUOctreeBroadphase::getRayPairCache() const {
//   return &rayPairs;
// }
// 
// ArrayInterface<BroadphasePair>* GPUOctreeBroadphase::getFrustumTestsResult() {
//   return &frustumPairs;
// }
// 
// const ArrayInterface<BroadphasePair>* GPUOctreeBroadphase::getFrustumTestsResult() const {
//   return &frustumPairs;
// }
// 
// void* GPUOctreeBroadphase::getIndirectPairBuffer() {
//   return indirectPairsCount;
// }

void GPUOctreeBroadphase::printStats() {
  const size_t totalMemory = proxies.buffer_size() + 
                             nodes.buffer_size() + 
                             changes.buffer_size() + 
                             indices1.buffer_size() + 
                             indices2.buffer_size() + 
                             proxyIndices.buffer_size() + 
                             toPairsCalculate.buffer_size();
                             //objPairs.vector().buffer_size() + 
                             //rayPairs.vector().buffer_size() + 
                             //frustumPairs.vector().buffer_size();

  const size_t usedMemory = proxies.size()*sizeof(proxies[0]) + 
                            nodes.size()*sizeof(nodes[0]) + 
                            changes[0].x*sizeof(proxies[0]) + 
                            proxies.size()*sizeof(indices1[0]) + 
                            proxies.size()*sizeof(indices2[0]) + 
                            proxyIndices[0]*sizeof(proxyIndices[0]) + 
                            toPairsCalculate[0]*sizeof(toPairsCalculate[0]);
                            //objPairs[0].firstIndex*sizeof(objPairs[0]) + 
                            //rayPairs[0].firstIndex*sizeof(rayPairs[0]) + 
                            //frustumPairs.size()*sizeof(frustumPairs[0]);

  std::cout << '\n';
  std::cout << "GPU octree broadphase data" << '\n';
  std::cout << "Node count    " << nodesCount << " Depth " << depth << '\n';
  std::cout << "Proxies count " << proxies.size() << '\n';
  std::cout << "Total memory usage     " << totalMemory << " bytes" << '\n';
  std::cout << "Total memory with data " << usedMemory << " bytes" << '\n';
  std::cout << "Free memory            " << (totalMemory-usedMemory) << " bytes" << '\n';
  std::cout << "Broadphase class size  " << sizeof(GPUOctreeBroadphase) << " bytes" << '\n';

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

  VkDispatchIndirectCommand* c = (VkDispatchIndirectCommand*)indirectBuffer->ptr();
  PRINT_VAR("Indirect test x", c->x)

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

void GPUOctreeBroadphase::construct(const GPUOctreeBroadphaseCreateInfo &info) {
  this->device = info.device;
  this->task = info.task;

  size_t nodesCount = 0;
  size_t eight = 1;
  for (uint32_t i = 0; i < info.octreeInfo.depth; ++i) {
    nodesCount += eight;
    eight *= 8;
  }

  this->nodesCount = nodesCount;

  if (info.octreeInfo.depth > 10) throw std::runtime_error("Depth > 10 is not supported");
  // при 10 получается буфер будет весить больше 10 Гб - это тоже плохо лол
  this->depth = info.octreeInfo.depth;

  // нужно бы как нибудь выделить аабб нодов в отдельный буфер
  proxies.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  proxies.reserve(nodesCount + 10000);
  nodes.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, nodesCount);
  indices1.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 10000);
  indices2.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 10000);

  changes.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 10000);
  proxyIndices.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 10000);
  toPairsCalculate.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 10000);

  memset(proxies.data(), 0, proxies.buffer_size());
  memset(nodes.data(), 0, nodes.buffer_size());
  memset(indices1.data(), 0, indices1.buffer_size());
  memset(indices2.data(), 0, indices2.buffer_size());
  memset(changes.data(), 0, changes.buffer_size());
  memset(proxyIndices.data(), 0, proxyIndices.buffer_size());
  memset(toPairsCalculate.data(), 0, toPairsCalculate.buffer_size());

  nodes[0].offset = 0;
  nodes[0].count = 0;
  nodes[0].childIndex = 1;
  nodes[0].proxyIndex = proxies.size();
  proxies.emplace_back(UINT32_MAX, PhysicsType(0, 0, 0, 0, 0, 0), 0, 0);
  proxies.back().setAABB({info.octreeInfo.center, info.octreeInfo.extent});

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
  for (uint32_t i = 1; i < info.octreeInfo.depth; ++i) { // тут должен быть 1 так как мы уже какбы прошли root
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

        glm::vec4 extent = proxies[parentProxyIndex].getAABB().extent / 2.0f;
        glm::vec4 center;
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

//   objPairs.construct(device, 4096);
//   //objPairs.resize(3000);
//   rayPairs.construct(device, 4096);
//   //rayPairs.resize(3000);
//   frustumPairs.construct(device, 4096);
//   //frustumPairs.resize(3000);
// 
//   memset(objPairs.vector().data(), 0, objPairs.vector().buffer_size());
//   memset(rayPairs.vector().data(), 0, rayPairs.vector().buffer_size());
//   memset(frustumPairs.vector().data(), 0, frustumPairs.vector().buffer_size());

  // Мне нужно создать дескриптор пул!!!
  yavf::DescriptorPool pool = device->descriptorPool("physics_descriptor_pool");
  {
    yavf::DescriptorPoolMaker dpm(device);

    if (pool == VK_NULL_HANDLE) {
      pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 40).poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5).create("physics_descriptor_pool");
    }
  }

  yavf::DescriptorSetLayout octree_data_layout = device->setLayout("physics_object_layout");
  yavf::DescriptorSetLayout physics_compute_layout = device->setLayout("physics_compute_layout");
  {
    yavf::DescriptorLayoutMaker dlm(device);

    if (octree_data_layout == VK_NULL_HANDLE) {
      octree_data_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                               binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                               binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                               binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                               binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("octree_data_layout");
    }

    if (physics_compute_layout == VK_NULL_HANDLE) {
      physics_compute_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("physics_compute_layout");
    }
  }

  yavf::PipelineLayout pair_calc_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout changes_calc_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout pre_update_octree_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout update_octree_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout calc_pairs_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout to_power_of_to_layout = VK_NULL_HANDLE;

  yavf::PipelineLayout ray_pairs_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout iterative_frustum_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout frustum_pairs_layout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);

    pair_calc_layout = plm.addDescriptorLayout(octree_data_layout).
                           addDescriptorLayout(physics_compute_layout).
                           addDescriptorLayout(physics_compute_layout).
                           addDescriptorLayout(physics_compute_layout).
                           addDescriptorLayout(physics_compute_layout).
                           addDescriptorLayout(physics_compute_layout).
                           addDescriptorLayout(physics_compute_layout).create("pair_calc_layout");

    changes_calc_layout = plm.addDescriptorLayout(physics_compute_layout)
                             .addDescriptorLayout(physics_compute_layout)
                             .addDescriptorLayout(physics_compute_layout)
                             .addDescriptorLayout(physics_compute_layout)
                             .addDescriptorLayout(physics_compute_layout)
                             .addDescriptorLayout(physics_compute_layout).create("changes_calc_layout");

    pre_update_octree_layout = plm.addDescriptorLayout(physics_compute_layout).create("pre_update_octree_layout");

    update_octree_layout = plm.addDescriptorLayout(physics_compute_layout)
                              .addDescriptorLayout(physics_compute_layout)
                              .addDescriptorLayout(physics_compute_layout)
                              .addDescriptorLayout(physics_compute_layout)
                              .addDescriptorLayout(physics_compute_layout).create("update_octree_layout");

    calc_pairs_layout = plm.addDescriptorLayout(physics_compute_layout)
                           .addDescriptorLayout(physics_compute_layout)
                           .addDescriptorLayout(physics_compute_layout)
                           .addDescriptorLayout(physics_compute_layout)
                           .addDescriptorLayout(physics_compute_layout)
                           .addDescriptorLayout(physics_compute_layout).create("calc_pairs_layout");

    to_power_of_to_layout = plm.addDescriptorLayout(physics_compute_layout).create("to_power_of_to_layout");

    ray_pairs_layout = plm.addDescriptorLayout(physics_compute_layout).
                           addDescriptorLayout(physics_compute_layout).
                           addDescriptorLayout(physics_compute_layout).
                           addDescriptorLayout(physics_compute_layout).
                           addDescriptorLayout(physics_compute_layout).create("ray_pairs_layout");

    iterative_frustum_layout = plm.addDescriptorLayout(physics_compute_layout).
                                   addDescriptorLayout(physics_compute_layout).
                                   addDescriptorLayout(physics_compute_layout).
                                   addDescriptorLayout(physics_compute_layout).
                                   addDescriptorLayout(physics_compute_layout).
                                   addDescriptorLayout(physics_compute_layout).create("iterative_frustum_layout");
  }

  {
    yavf::ComputePipelineMaker pm(device);

    uint32_t workGroupSizeCalcProxy = info.internal == nullptr ? 256 : glm::min(uint32_t(512), glm::max(uint32_t(1), info.internal->workGroupSizeCalcProxy));
    uint32_t workGroupSizeCalcChanges = info.internal == nullptr ? 256 : glm::min(uint32_t(512), glm::max(uint32_t(1), info.internal->workGroupSizeCalcProxy));
    uint32_t workGroupSizePreUpdate = info.internal == nullptr ? 64 : glm::min(uint32_t(128), glm::max(uint32_t(1), info.internal->workGroupSizeCalcProxy));
    uint32_t workGroupSizeUpdate = info.internal == nullptr ? 64 : glm::min(uint32_t(128), glm::max(uint32_t(1), info.internal->workGroupSizeCalcProxy));
    uint32_t workGroupSizeCalcPairs = info.internal == nullptr ? 64 : glm::min(uint32_t(128), glm::max(uint32_t(1), info.internal->workGroupSizeCalcProxy));
    
    yavf::raii::ShaderModule calcProxyShader(device,         (Global::getGameDir() + "shaders/recalculateAABB.spv").c_str());
    yavf::raii::ShaderModule calcChangesShader(device,       (Global::getGameDir() + "shaders/updateOctree.spv").c_str());
    yavf::raii::ShaderModule preUpdateOctreeShader(device,   (Global::getGameDir() + "shaders/preUpdateNodeIdx.spv").c_str());
    yavf::raii::ShaderModule updateOctreeShader(device,      (Global::getGameDir() + "shaders/updateNodeIdx2.spv").c_str());
    yavf::raii::ShaderModule calcPairsShader(device,         (Global::getGameDir() + "shaders/octree.spv").c_str());
    yavf::raii::ShaderModule calcRayPairsShader(device,      (Global::getGameDir() + "shaders/octreeRay.spv").c_str());
    yavf::raii::ShaderModule iterativeFrustumShader(device,  (Global::getGameDir() + "shaders/iterativeFrustum.spv").c_str());
    yavf::raii::ShaderModule octreeFrustumShader(device,     (Global::getGameDir() + "shaders/octreeFrustum2.spv").c_str());
    yavf::raii::ShaderModule pairsToPowerOfTwoShader(device, (Global::getGameDir() + "shaders/frustumPowerOfTwo.spv").c_str());

    calcProxy = pm.addSpecializationEntry(0, 0, sizeof(uint32_t)).addData(sizeof(uint32_t), &workGroupSizeCalcProxy).
                   shader(calcProxyShader).create("calcProxy", pair_calc_layout);
                   
    calcChanges = pm.addSpecializationEntry(0, 0, sizeof(uint32_t)).addData(sizeof(uint32_t), &workGroupSizeCalcChanges).
                     shader(calcChangesShader).create("calcChanges", changes_calc_layout);
                     
    preUpdateOctree = pm.addSpecializationEntry(0, 0, sizeof(uint32_t)).addData(sizeof(uint32_t), &workGroupSizePreUpdate).
                         shader(preUpdateOctreeShader).create("preUpdateOctree", pre_update_octree_layout);
                         
    updateOctree = pm.addSpecializationEntry(0, 0, sizeof(uint32_t)).addData(sizeof(uint32_t), &workGroupSizeUpdate).
                      shader(updateOctreeShader).create("updateOctree", update_octree_layout);

    uint32_t sharedSize = info.internal == nullptr ? 512 : glm::min(uint32_t(1024), glm::max(uint32_t(1), info.internal->workGroupSizeCalcProxy));
    uint32_t arr[4] = {static_cast<uint32_t>(depth), static_cast<uint32_t>(nodesCount), workGroupSizeCalcPairs, sharedSize};
    calcPairs = pm.addSpecializationEntry(0, 0*sizeof(uint32_t), sizeof(uint32_t)).
                   addSpecializationEntry(1, 1*sizeof(uint32_t), sizeof(uint32_t)).
                   addSpecializationEntry(2, 2*sizeof(uint32_t), sizeof(uint32_t)).
                   addSpecializationEntry(3, 3*sizeof(uint32_t), sizeof(uint32_t)).
                   addData(4*sizeof(uint32_t), arr).
                   shader(calcPairsShader).create("calcPairs", calc_pairs_layout);
    
    calcRayPairs = pm.shader(calcRayPairsShader).create("calcRayPairs", ray_pairs_layout);
    iterativeFrustum = pm.shader(iterativeFrustumShader).create("iterativeFrustum", iterative_frustum_layout);
    octreeFrustum = pm.shader(octreeFrustumShader).create("octreeFrustum", iterative_frustum_layout);
    pairsToPowerOfTwo = pm.shader(pairsToPowerOfTwoShader).create("pairsToPowerOfTwo", to_power_of_to_layout);
  }

  {
    yavf::DescriptorMaker dm(device);

    {
      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t i = d->add({proxies.handle(), 0, proxies.buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      proxies.setDescriptor(d, i);
    }

    {
      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t i = d->add({nodes.handle(), 0, nodes.buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      nodes.setDescriptor(d, i);
    }

    {
      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t i = d->add({changes.handle(), 0, changes.buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      changes.setDescriptor(d, i);
    }

    {
      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t i = d->add({indices1.handle(), 0, indices1.buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      indices1.setDescriptor(d, i);
    }

    {
      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t i = d->add({indices2.handle(), 0, indices2.buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      indices2.setDescriptor(d, i);
    }

    {
      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t i = d->add({proxyIndices.handle(), 0, proxyIndices.buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      proxyIndices.setDescriptor(d, i);
    }

    {
      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t i = d->add({toPairsCalculate.handle(), 0, toPairsCalculate.buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      toPairsCalculate.setDescriptor(d, i);
    }

    {
      indirectBuffer = device->create(yavf::BufferCreateInfo::buffer(sizeof(VkDispatchIndirectCommand), 
                                                                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT), 
                                      VMA_MEMORY_USAGE_CPU_ONLY);

      //memset(indirectBuffer->ptr(), 0, indirectBuffer->param().size);
      VkDispatchIndirectCommand* c = reinterpret_cast<VkDispatchIndirectCommand*>(indirectBuffer->ptr());
      c->x = 0;
      //c->y = nodesCount;
      c->y = 1;
      c->z = 1;

      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t i = d->add({indirectBuffer, 0, indirectBuffer->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      indirectBuffer->setDescriptor(d, i);
    }

    //indirectPairsCount.construct(device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    {
      indirectPairsCount = device->create(yavf::BufferCreateInfo::buffer(sizeof(VkDispatchIndirectCommand), 
                                                                         VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), 
                                          VMA_MEMORY_USAGE_CPU_ONLY);

      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t i = d->add({indirectPairsCount, 0, indirectPairsCount->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      indirectPairsCount->setDescriptor(d, i);
    }
  }
}

GPUOctreeBroadphaseDirectPipeline::GPUOctreeBroadphaseDirectPipeline(const GPUOctreeBroadphaseCreateInfo &info) : 
  GPUOctreeBroadphase(info) {}

GPUOctreeBroadphaseDirectPipeline::~GPUOctreeBroadphaseDirectPipeline() {
  device->destroy(calcProxy);
  device->destroy(calcChanges);
  device->destroy(updateOctree);

  device->destroyLayout("pair_calc_layout");
  device->destroyLayout("changes_calc_layout");
  device->destroyLayout("update_octree_layout");
}

void GPUOctreeBroadphaseDirectPipeline::update() {
  task->setPipeline(calcProxy);
  task->setDescriptor({objectsDataDesc->handle(), 
                       transformsDesc->handle(), 
                       indexBufferDesc->handle(), 
                       proxies.descriptorSet()->handle(), 
                      proxyIndices.descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);

  task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  // здесь еще будет обновление октодерева
  task->setPipeline(calcChanges);
  task->setDescriptor({nodes.descriptorSet()->handle(),
                       changes.descriptorSet()->handle(),
                       proxies.descriptorSet()->handle(),
                       proxyIndices.descriptorSet()->handle(),
                       toPairsCalculate.descriptorSet()->handle(),
                       indirectBuffer->descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);

  task->copy(indices2.handle(), indices1.handle(), 0, 0, indices2.buffer_size());

  task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->setPipeline(preUpdateOctree);
  task->setDescriptor(nodes.descriptorSet(), 0);
  task->dispatch(nodesCount, 1, 1);

  task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->setPipeline(updateOctree);
  task->setDescriptor({nodes.descriptorSet()->handle(),
                       indices1.descriptorSet()->handle(),
                       changes.descriptorSet()->handle(),
                       proxies.descriptorSet()->handle(),
                       indices2.descriptorSet()->handle()}, 0);
  task->dispatch(nodesCount, 1, 1);
}

void GPUOctreeBroadphaseDirectPipeline::calculateOverlappingPairs() {
  task->setPipeline(calcPairs);
  task->setDescriptor({proxies.descriptorSet()->handle(), 
                       nodes.descriptorSet()->handle(),
                       indices2.descriptorSet()->handle(),
                       toPairsCalculate.descriptorSet()->handle(),
                       overlappingPairCacheDesc->handle(),//objPairs.vector().descriptorSet(),
                       indirectPairsCount->descriptorSet()->handle()}, 0);
  task->dispatchIndirect(indirectBuffer);

//   task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
//                    VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
}

void GPUOctreeBroadphaseDirectPipeline::calculateRayTests() {
  task->setPipeline(calcRayPairs);
  task->setDescriptor({proxies.descriptorSet()->handle(), 
                       nodes.descriptorSet()->handle(), 
                       indices2.descriptorSet()->handle(), 
                       rayDesc->handle(), 
                       rayPairCacheDesc->handle()}, 0);
  task->dispatch(raysCount, 1, 1);
}

void GPUOctreeBroadphaseDirectPipeline::calculateFrustumTests() {
  task->setPipeline(iterativeFrustum);
  task->setDescriptor({proxies.descriptorSet()->handle(), 
                       nodes.descriptorSet()->handle(), 
                       indices2.descriptorSet()->handle(), 
                       frustumDesc->handle(), 
                       frustumPosesDesc->handle(), 
                       frustumTestsResultDesc->handle()}, 0);
  task->dispatch(frustumCount, 1, 1);

  task->setPipeline(octreeFrustum);
  task->setDescriptor({proxies.descriptorSet()->handle(), 
                       nodes.descriptorSet()->handle(), 
                       indices2.descriptorSet()->handle(), 
                       frustumDesc->handle(), 
                       frustumPosesDesc->handle(), 
                       frustumTestsResultDesc->handle()}, 0);
  task->dispatch(frustumDispatchNodeCount, frustumCount, 1);
}
