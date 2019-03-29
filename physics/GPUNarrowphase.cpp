#include "GPUNarrowphase.h"
#include "Globals.h"
#include <chrono>

GPUNarrowphase::GPUNarrowphase(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUNarrowphaseData* data) {
  construct(device, task, data);
}

GPUNarrowphase::~GPUNarrowphase() {
  device->destroy(islandCalc);
  device->destroy(batchCalc);
  device->destroy(sameCalc);
  device->destroy(sortingPairs);
  device->destroy(calcIslandData);

  device->destroyLayout("find_island_layout");
  device->destroyLayout("process_sorting_layout");
  device->destroyLayout("island_calc_layout");

  device->destroy(indirectIslandCount);
}

// void GPUNarrowphase::setPairBuffer(ArrayInterface<BroadphasePair>* buffer, void* indirectPairCount) {
//   buffer->descriptorPtr(&pairsDescriptor);
//   //pairArray = buffer;
// 
//   this->indirectPairCount = (yavf::Buffer*)indirectPairCount;
// }

void GPUNarrowphase::setInputBuffers(const InputBuffers &inputs, void* indirectPairCount) {
  yavf::Buffer* buffer1;
  inputs.dynPairs->descriptorPtr(&buffer1);
  pairsDescriptor = buffer1->descriptorSet();
  inputs.statPairs->descriptorPtr(&buffer1);
  staticPairsDescriptor = buffer1->descriptorSet();
  
  this->indirectPairCount = (yavf::Buffer*)indirectPairCount;
}

void GPUNarrowphase::setOutputBuffers(const OutputBuffers &outputs, void* indirectIslandCount) {
  throw std::runtime_error("gpu narrowphase dont working yet");
}

void GPUNarrowphase::updateBuffers(const uint32_t &lastPairCount, const uint32_t &lastStaticPairCount) {
  if (islands.vector().size() < 12 + lastPairCount*1.5f) islands.vector().resize(12 + lastPairCount*1.5f);
  
  throw std::runtime_error("gpu narrowphase dont working yet");
  
  //if (islands.vector().size() < 12 + lastPairCount*1.5f) islands.vector().resize(12 + lastPairCount*1.5f);
}

void GPUNarrowphase::calculateIslands() {
  task->begin();

  task->setPipeline(islandCalc);
  task->setDescriptor(pairsDescriptor, 0);
  task->dispatch(1, 1, 1);

  // uint32_t arr[2];
  // for (uint32_t i = 0; i < 50; ++i) {
  //   arr[0] = i;
  //   arr[1] = 50;
  //   task->setConsts(0, 2*sizeof(uint32_t), arr, VK_SHADER_STAGE_COMPUTE_BIT);
  //   task->dispatchIndirect(indirectPairCount);

  //   task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
  //                    VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
  //   // тут это так не работает, нужен индирект буфер
  //   //task->dispatch(glm::max(pairArray->at(0).firstIndex, uint32_t(1)), 1, 1);
  // }

  // {
  //   GPUArray<BroadphasePair>* b = (GPUArray<BroadphasePair>*)pairArray;
  //   const std::vector<VkBufferMemoryBarrier> barriers{
  //     {
  //       VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
  //       nullptr,
  //       VK_ACCESS_MEMORY_WRITE_BIT,
  //       VK_ACCESS_MEMORY_READ_BIT,
  //       VK_QUEUE_FAMILY_IGNORED,
  //       VK_QUEUE_FAMILY_IGNORED,
  //       b->vector().handle(),
  //       0,
  //       b->vector().buffer_size()
  //     }
  //   };

  //   task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  //                    {},
  //                    barriers,
  //                    {});
  // }

  // task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->end();
  
  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Islands calc time: " << ns << " mcs" << "\n";
}

void GPUNarrowphase::calculateBatches() {
  task->begin();

  task->setPipeline(batchCalc);
  task->setDescriptor(pairsDescriptor, 0);
  //task->dispatch(1, 1, 1);
  task->dispatchIndirect(indirectPairCount);

  // task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->end();
  
  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Batches calc time: " << ns << " mcs" << "\n";
}

void GPUNarrowphase::checkIdenticalPairs() {
  task->begin();

  task->setPipeline(sameCalc);
  task->setDescriptor(pairsDescriptor, 0);
  //task->dispatch(1, 1, 1);
  task->dispatchIndirect(indirectPairCount);

  // task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->end();
  
  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Checking same pairs time: " << ns << " mcs" << "\n";
}

// void GPUNarrowphase::sortPairs() {
//   task->begin();

//   task->setPipeline(sortingPairs);
//   task->setDescriptor(pairsDescriptor, 0);
//   task->dispatch(1, 1, 1);

//   task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
//                    VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

//   task->end();
  
//   auto start = std::chrono::steady_clock::now();
//   task->start();
//   task->wait();
//   auto end = std::chrono::steady_clock::now() - start;
//   auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
//   std::cout << "Sorting pairs time: " << ns << " mcs" << "\n";
// }

void GPUNarrowphase::postCalculation() {
  task->begin();

  task->setPipeline(calcIslandData);
  task->setDescriptor({pairsDescriptor->handle(), islands.vector().descriptorSet()->handle(), indirectIslandCount->descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);

  // task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->end();
  
  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Post calc time: " << ns << " mcs" << "\n";
}

// ArrayInterface<IslandData>* GPUNarrowphase::getIslandDataBuffer() {
//   return &islands;
// }
// 
// const ArrayInterface<IslandData>* GPUNarrowphase::getIslandDataBuffer() const {
//   return &islands;
// }

void* GPUNarrowphase::getIndirectIslandCount() {
  return indirectIslandCount;
}

void GPUNarrowphase::printStats() {
  std::cout << '\n';
  std::cout << "Gpu narrowphase data" << '\n';
  std::cout << "Islands count " << islands[0].islandIndex << '\n';
  std::cout << "Total memory usage     " << islands.vector().buffer_size() << " bytes" << '\n';
  std::cout << "Narrowphase class size " << sizeof(GPUNarrowphase) << " bytes" << '\n';

  for (uint32_t i = 1; i < islands[0].islandIndex+1; ++i) {
    // PRINT_VAR("Islands index ", islands[i].islandIndex)
    // PRINT_VAR("Islands offset", islands[i].offset)
    // PRINT_VAR("Islands size  ", islands[i].size)
    // std::cout << "\n";
  }

  //PRINT_VAR("Islands test ", islands[0].offset)
}

void GPUNarrowphase::construct(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUNarrowphaseData* data) {
  this->device = device;
  this->task = task;

  islands.construct(device, 3000);
  memset(islands.vector().data(), 0, islands.vector().buffer_size());

  yavf::DescriptorPool pool = device->descriptorPool("physics_descriptor_pool");
  {
    yavf::DescriptorPoolMaker dpm(device);

    if (pool == VK_NULL_HANDLE) {
      pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 40).poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5).create("physics_descriptor_pool");
    }
  }

  yavf::DescriptorSetLayout physics_compute_layout = device->setLayout("physics_compute_layout");
  {
    yavf::DescriptorLayoutMaker dlm(device);

    if (physics_compute_layout == VK_NULL_HANDLE) {
      physics_compute_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("physics_compute_layout");
    }
  }

  yavf::PipelineLayout process_sorting_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout find_island_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout island_calc_layout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);

    //addPushConstRange(0, 2*sizeof(uint32_t), VK_SHADER_STAGE_COMPUTE_BIT).
    find_island_layout = plm.addDescriptorLayout(physics_compute_layout).create("find_island_layout");

    process_sorting_layout = plm.addDescriptorLayout(physics_compute_layout).create("process_sorting_layout");

    island_calc_layout = plm.addDescriptorLayout(physics_compute_layout).
                             addDescriptorLayout(physics_compute_layout).
                             addDescriptorLayout(physics_compute_layout).create("island_calc_layout");
  }

  {
    yavf::ComputePipelineMaker pm(device);
    
    yavf::raii::ShaderModule islandCalcShader(device, (Global::getGameDir() + "shaders/islands.spv").c_str());
    yavf::raii::ShaderModule batchCalcShader(device, (Global::getGameDir() + "shaders/batching2.spv").c_str());
    yavf::raii::ShaderModule sameCalcShader(device, (Global::getGameDir() + "shaders/checkSamePairs2.spv").c_str());
    yavf::raii::ShaderModule calcIslandDataShader(device, (Global::getGameDir() + "shaders/computeIslandsSize.spv").c_str());

    uint32_t workGroupSizeIslandIdCalc = data == nullptr ? 256 : glm::max(uint32_t(512), glm::min(uint32_t(1), data->workGroupSizeIslandCalc));
    uint32_t iterationCount = data == nullptr ? 50 : glm::max(uint32_t(100), glm::min(uint32_t(10), data->islandIterationCount));
    uint32_t arr[2] = {iterationCount, workGroupSizeIslandIdCalc};
    islandCalc = pm.addSpecializationEntry(0, 0*sizeof(uint32_t), sizeof(uint32_t)).
                    addSpecializationEntry(1, 1*sizeof(uint32_t), sizeof(uint32_t)).
                    addData(2*sizeof(uint32_t), arr).
                    shader(islandCalcShader).create("islandCalc", find_island_layout);

    uint32_t workGroupSizeBatching = data == nullptr ? 16 : glm::max(uint32_t(32), glm::min(uint32_t(1), data->workGroupSizeBatching));
    batchCalc = pm.addSpecializationEntry(0, 0*sizeof(uint32_t), sizeof(uint32_t)).
                   addData(1*sizeof(uint32_t), &workGroupSizeBatching).
                   shader(batchCalcShader).create("batchCalc", process_sorting_layout);

    uint32_t workGroupSizeSamePairsChecking = data == nullptr ? 16 : glm::max(uint32_t(32), glm::min(uint32_t(1), data->workGroupSizeSamePairsChecking));
    sameCalc = pm.addSpecializationEntry(0, 0*sizeof(uint32_t), sizeof(uint32_t)).
                  addData(sizeof(uint32_t), &workGroupSizeSamePairsChecking).
                  shader(sameCalcShader).create("sameCalc", process_sorting_layout);

    // пока наверное не буду менять
    // тут всегда должна быть степень двойки
    //uint32_t workGroupSizeSorting = data == nullptr ? 256 : glm::max(uint32_t(512), glm::min(uint32_t(1), data->workGroupSizeSorting));
    //sortingPairs = pm.shader(VK_SHADER_STAGE_COMPUTE_BIT, Global::getGameDir() + "shaders/sorting.spv").create("sortingPairs", process_sorting_layout);

    uint32_t workGroupSizeCalcIslandData = data == nullptr ? 256 : glm::max(uint32_t(512), glm::min(uint32_t(1), data->workGroupSizeCalcIslandData));
    calcIslandData = pm.addSpecializationEntry(0, 0*sizeof(uint32_t), sizeof(uint32_t)).
                        addData(sizeof(uint32_t), &workGroupSizeCalcIslandData).
                        shader(calcIslandDataShader).create("calcIslandData", island_calc_layout);

    // мне нужно еще почекать есть ли среди пар ступеньки
    // и нужно получить индексы пола и еще можно кое какие други данные
    // но для этого мне нужны данные объекта
    // думаю что это я буду делать в солвере
  }

  {
    yavf::DescriptorMaker dm(device);

    {
      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t index = d->add({islands.vector().handle(), 0, islands.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      islands.vector().setDescriptor(d, index);
    }

    {
      indirectIslandCount = device->create(yavf::BufferCreateInfo::buffer(2*sizeof(VkDispatchIndirectCommand), 
                                                                          VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), 
                                           VMA_MEMORY_USAGE_CPU_ONLY);

      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t index = d->add({indirectIslandCount, 0, indirectIslandCount->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      indirectIslandCount->setDescriptor(d, index);
    }
  }
}

GPUNarrowphaseDirectPipeline::GPUNarrowphaseDirectPipeline(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUNarrowphaseData* data) : 
  GPUNarrowphase(device, task, data) {}

GPUNarrowphaseDirectPipeline::~GPUNarrowphaseDirectPipeline() {
  device->destroy(islandCalc);
  device->destroy(batchCalc);
  device->destroy(sameCalc);
  device->destroy(sortingPairs);
  device->destroy(calcIslandData);

  device->destroyLayout("find_island_layout");
  device->destroyLayout("process_sorting_layout");
  device->destroyLayout("island_calc_layout");

  device->destroy(indirectIslandCount);
}

void GPUNarrowphaseDirectPipeline::calculateIslands() {

}

void GPUNarrowphaseDirectPipeline::calculateBatches() {

}

void GPUNarrowphaseDirectPipeline::checkIdenticalPairs() {

}

void GPUNarrowphaseDirectPipeline::postCalculation() {

}
