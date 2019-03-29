#include "GPUSolver.h"
#include "Globals.h"
#include <chrono>

GPUSolver::GPUSolver(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUSolverData* data) {
  construct(device, task, data);
}

GPUSolver::~GPUSolver() {}

// void GPUSolver::setBuffers(const SolverBuffers &buffers, void* indirectIslandCount, void* indirectPairCount) {
//   buffers.objects->descriptorPtr(&objData);
//   //buffers.datas->descriptorPtr(&physDatas);
//   buffers.systems->descriptorPtr(&matrices);
//   buffers.transforms->descriptorPtr(&transforms);
//   //buffers.staticPhysDatas->descriptorPtr(&staticPhysicDatas);
//   buffers.rotationDatas->descriptorPtr(&rotationDatas);
//   buffers.pairs->descriptorPtr(&pairs);
//   buffers.islands->descriptorPtr(&islands);
//   buffers.indicies->descriptorPtr(&indicies);
//   //buffers.velocities->descriptorPtr(&velocities);
//   buffers.gravity->descriptorPtr(&gravity);
// 
//   buffers.rays->descriptorPtr(&rays);
//   buffers.rayPairs->descriptorPtr(&rayPairs);
// 
//   this->indirectIslandCount = (yavf::Buffer*)indirectIslandCount;
//   this->indirectPairsCount = (yavf::Buffer*)indirectPairCount;
// }

void GPUSolver::setInputBuffers(const InputBuffers &buffers, void* indirectIslandCount, void* indirectPairCount) {
  yavf::Buffer* buffer;
  buffers.objects->descriptorPtr(&buffer);
  objData = buffer->descriptorSet();
  //buffers.datas->descriptorPtr(&physDatas);
  buffers.systems->descriptorPtr(&buffer);
  matrices = buffer->descriptorSet();
  buffers.transforms->descriptorPtr(&buffer);
  transforms = buffer->descriptorSet();
  //buffers.staticPhysDatas->descriptorPtr(&staticPhysicDatas);
  buffers.rotationDatas->descriptorPtr(&buffer);
  rotationDatas = buffer->descriptorSet();
  buffers.pairs->descriptorPtr(&buffer);
  pairs = buffer->descriptorSet();
  buffers.islands->descriptorPtr(&buffer);
  islands = buffer->descriptorSet();
  buffers.indicies->descriptorPtr(&buffer);
  indicies = buffer->descriptorSet();
  //buffers.velocities->descriptorPtr(&velocities);
  buffers.gravity->descriptorPtr(&buffer);
  gravity = buffer->descriptorSet();

  buffers.rays->descriptorPtr(&buffer);
  rays = buffer->descriptorSet();
  buffers.rayPairs->descriptorPtr(&buffer);
  rayPairs = buffer->descriptorSet();

  this->indirectIslandCount = (yavf::Buffer*)indirectIslandCount;
  this->indirectPairsCount = (yavf::Buffer*)indirectPairCount;
}

void GPUSolver::setOutputBuffers(const OutputBuffers &buffers) {
  overlappingData = buffers.overlappingData;
  dataIndices = buffers.dataIndices;
  raysData = buffers.raysData;
  raysIndices = buffers.raysIndices;
  triggerIndices = buffers.triggerIndices;
  
  yavf::Buffer* buffer;
  overlappingData->descriptorPtr(&buffer);
  overlappingDataDesc = buffer->descriptorSet();
  dataIndices->descriptorPtr(&buffer);
  dataIndicesDesc = buffer->descriptorSet();
  raysData->descriptorPtr(&buffer);
  raysDataDesc = buffer->descriptorSet();
  raysIndices->descriptorPtr(&buffer);
  raysIndicesDesc = buffer->descriptorSet();
  triggerIndices->descriptorPtr(&buffer);
  triggerIndicesDesc = buffer->descriptorSet();
}

void GPUSolver::updateBuffers(const uint32_t &pairsCount, const uint32_t &rayPairs) {
  // достаточно ли полтора? мне кажется что нет...
  // тут скорее должно быть количество всех объектов + количество пар (или все объекты * 1.5f)
  if (overlappingData->size() < pairsCount * 1.5f) overlappingData->resize(pairsCount * 1.5f);
  if (raysData->size() < rayPairs) raysData->resize(rayPairs);
  //if (overlapTmpBuffer->param().size < pairsCount) overlapTmpBuffer->recreate(pairsCount, 0);
  if (triggerIndices->size() < overlappingData->size()) triggerIndices->resize(overlappingData->size());
}

void GPUSolver::calculateData() {
  task->begin();

  task->setPipeline(searchPairs);
  task->setDescriptor({pairs->handle(), 
                       overlappingDataDesc->handle()/*overlappingData.vector().descriptor()*/, 
                       indirectOverlappingCalc.buffer()->descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);

  task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  // task->end();

  // auto start = std::chrono::steady_clock::now();
  // task->start();
  // task->wait();
  // auto end = std::chrono::steady_clock::now() - start;
  // auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  // std::cout << "Pair finding time: " << ns << " mcs" << "\n";

  // task->begin();

  task->setPipeline(calcOverlappingData);
  task->setDescriptor({objData->handle(), 
                       matrices->handle(), 
                       transforms->handle(), 
                       rotationDatas->handle(), 
                       overlappingDataDesc->handle(), 
                       indirectOverlappingCalc.buffer()->descriptorSet()->handle()}, 0);
  task->dispatchIndirect(indirectOverlappingCalc.buffer());

  task->end();

  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Overlapping data calculaton time: " << ns << " mcs" << "\n";
}

void GPUSolver::calculateRayData() {
  task->begin();

  task->setPipeline(calcRayData);
  task->setDescriptor({objData->handle(), 
                       matrices->handle(), 
                       transforms->handle(), 
                       rotationDatas->handle(), 
                       rays->handle(), 
                       rayPairs->handle(), 
                       raysDataDesc->handle(), 
                       rayIndices.buffer()->descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);

  task->end();

  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Ray data calculaton time: " << ns << " mcs" << "\n";
}

void GPUSolver::solve() {
  task->begin();

  // task->setPipeline(solvePhysics);
  // task->setDescriptor({objData, physDatas, transforms, pairs, islands, indicies, velocities, gravity}, 0);
  // task->dispatch(1, 1, 1);

  const uint32_t iterationCount = 2;
  uint32_t arr[2];
  for (uint32_t i = 0; i < iterationCount; ++i) {
    arr[0] = i;
    arr[1] = iterationCount;
    task->setPipeline(posRecalc);
    task->setConsts(0, 2*sizeof(uint32_t), arr, VK_SHADER_STAGE_COMPUTE_BIT);
    task->setDescriptor({objData->handle(), transforms->handle(), indicies->handle(), gravity->handle()}, 0);
    task->dispatch(1, 1, 1);

    task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                     VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

    task->setPipeline(overlapForSolver);
    task->setDescriptor({objData->handle(), 
                         matrices->handle(), 
                         transforms->handle(), 
                         rotationDatas->handle(), 
                         pairs->handle(), 
                         overlapTmpBuffer->descriptorSet()->handle(), 
                         gravity->handle()}, 0);
    //task->dispatchIndirect(indirectPairsCount);
    task->dispatchIndirect(indirectIslandCount, sizeof(VkDispatchIndirectCommand));

    task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                     VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

    task->setPipeline(newSolver);
    task->setDescriptor({objData->handle(), 
                         matrices->handle(), 
                         transforms->handle(), 
                         islands->handle(), 
                         overlapTmpBuffer->descriptorSet()->handle(), 
                         gravity->handle()}, 0);
    task->dispatch(1, 1, 1);

    task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                     VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
  }

  // task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->end();

  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Solver time: " << ns << " mcs" << "\n";
}

// ArrayInterface<OverlappingData>* GPUSolver::getOverlappingData() {
//   return &overlappingData;
// }
// 
// const ArrayInterface<OverlappingData>* GPUSolver::getOverlappingData() const {
//   return &overlappingData;
// }
// 
// ArrayInterface<DataIndices>* GPUSolver::getDataIndices() {
//   return &indirectOverlappingCalc;
// }
// 
// const ArrayInterface<DataIndices>* GPUSolver::getDataIndices() const {
//   return &indirectOverlappingCalc;
// }
// 
// ArrayInterface<OverlappingData>* GPUSolver::getRayIntersectData() {
//   return &rayData;
// }
// 
// const ArrayInterface<OverlappingData>* GPUSolver::getRayIntersectData() const {
//   return &rayData;
// }
// 
// ArrayInterface<DataIndices>* GPUSolver::getRayIndices() {
//   return &rayIndixies;
// }
// 
// const ArrayInterface<DataIndices>* GPUSolver::getRayIndices() const {
//   return &rayIndixies;
// }

void GPUSolver::printStats() {
  std::cout << '\n';
  std::cout << "GPU solver data" << '\n';
  std::cout << "Solver class size " << sizeof(GPUSolver) << " bytes" << '\n';
}

void GPUSolver::construct(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUSolverData* data) {
  this->device = device;
  this->task = task;

  indirectOverlappingCalc.construct(device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    rayIndices.construct(device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

//   overlappingData.construct(device, 500);
//   rayData.construct(device, 500);

  yavf::DescriptorPool pool = device->descriptorPool("physics_descriptor_pool");
  {
    yavf::DescriptorPoolMaker dpm(device);

    if (pool == VK_NULL_HANDLE) {
      pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 40).poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5).create("physics_descriptor_pool");
    }
  }

  yavf::DescriptorSetLayout octree_data_layout = device->setLayout("physics_object_layout");
  yavf::DescriptorSetLayout physics_compute_layout = device->setLayout("physics_compute_layout");
  yavf::DescriptorSetLayout layoutUniform = device->setLayout("physics_uniform_layout");
  //yavf::DescriptorSetLayout phys_data_layout = device->setLayout("phys_data_layout");
  {
    yavf::DescriptorLayoutMaker dlm(device);

    if (octree_data_layout == VK_NULL_HANDLE) {
      octree_data_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                               binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                               binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                               binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                               binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("physics_object_layout");
    }

    if (physics_compute_layout == VK_NULL_HANDLE) {
      physics_compute_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("physics_compute_layout");
    }

    if (layoutUniform == VK_NULL_HANDLE) {
      layoutUniform = dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("physics_uniform_layout");
    }

    // if (phys_data_layout == VK_NULL_HANDLE) {
    //   phys_data_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
    //                          binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("phys_data_layout");
    // }
  }

  yavf::PipelineLayout solver_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout pos_recalc_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout overlap_data_for_solver_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout new_solver_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout ray_intersect_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout search_pairs_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout calc_overlapping_layout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);

    //.addPushConstRange(0, 2*sizeof(uint32_t), VK_SHADER_STAGE_COMPUTE_BIT)
    // solver_layout = plm.addDescriptorLayout(octree_data_layout)
    //                    //.addDescriptorLayout(phys_data_layout)
    //                    .addDescriptorLayout(physics_compute_layout)
    //                    .addDescriptorLayout(physics_compute_layout)
    //                    .addDescriptorLayout(physics_compute_layout)
    //                    .addDescriptorLayout(physics_compute_layout)
    //                    .addDescriptorLayout(layoutUniform)
    //                    .create("solver_layout");

    pos_recalc_layout = plm.addDescriptorLayout(octree_data_layout)
                           //.addDescriptorLayout(phys_data_layout)
                           .addDescriptorLayout(physics_compute_layout)
                           .addDescriptorLayout(physics_compute_layout)
                           .addDescriptorLayout(layoutUniform)
                           .addPushConstRange(0, 2*sizeof(uint32_t), VK_SHADER_STAGE_COMPUTE_BIT).create("pos_recalc_layout");

    overlap_data_for_solver_layout = plm.addDescriptorLayout(octree_data_layout).
                                         //addDescriptorLayout(phys_data_layout).
                                         addDescriptorLayout(physics_compute_layout).
                                         addDescriptorLayout(physics_compute_layout).
                                         addDescriptorLayout(physics_compute_layout).
                                         addDescriptorLayout(physics_compute_layout).
                                         addDescriptorLayout(physics_compute_layout).
                                         addDescriptorLayout(layoutUniform).create("overlap_data_for_solver_layout");

    new_solver_layout = plm.addDescriptorLayout(octree_data_layout).
                            //addDescriptorLayout(phys_data_layout).
                            //addDescriptorLayout(physics_compute_layout).
                            addDescriptorLayout(physics_compute_layout).
                            addDescriptorLayout(physics_compute_layout).
                            addDescriptorLayout(physics_compute_layout).
                            addDescriptorLayout(physics_compute_layout).
                            addDescriptorLayout(layoutUniform).create("new_solver_layout");

    ray_intersect_layout = plm.addDescriptorLayout(octree_data_layout).
                               addDescriptorLayout(physics_compute_layout).
                               addDescriptorLayout(physics_compute_layout).
                               addDescriptorLayout(physics_compute_layout).
                               addDescriptorLayout(physics_compute_layout).
                               addDescriptorLayout(physics_compute_layout).
                               addDescriptorLayout(physics_compute_layout).
                               addDescriptorLayout(physics_compute_layout).create("ray_intersect_layout");

    search_pairs_layout = plm.addDescriptorLayout(physics_compute_layout).
                              addDescriptorLayout(physics_compute_layout).
                              addDescriptorLayout(physics_compute_layout).create("search_pairs_layout");

    calc_overlapping_layout = plm.addDescriptorLayout(octree_data_layout).
                                  addDescriptorLayout(physics_compute_layout).
                                  addDescriptorLayout(physics_compute_layout).
                                  addDescriptorLayout(physics_compute_layout).
                                  addDescriptorLayout(physics_compute_layout).
                                  addDescriptorLayout(physics_compute_layout).create("calc_overlapping_layout");
  }

  {
    yavf::ComputePipelineMaker pm(device);

    uint32_t workGroupSize = data == nullptr ? 64 : glm::min(uint32_t(128), glm::max(uint32_t(1), data->workGroupSizeSolverData));
    
    // я не помню какие шейдеры я тут использовал =(, может быть ошибка
    yavf::raii::ShaderModule posRecalcShader(device, (Global::getGameDir() + "shaders/posRecalc.spv").c_str());
    yavf::raii::ShaderModule overlapForSolverShader(device, (Global::getGameDir() + "shaders/calcOverlappingDataToSolver.spv").c_str());
    yavf::raii::ShaderModule newSolverShader(device, (Global::getGameDir() + "shaders/newSolver.spv").c_str());
    yavf::raii::ShaderModule calcRayDataShader(device, (Global::getGameDir() + "shaders/calcRayIntersect.spv").c_str());
    yavf::raii::ShaderModule searchPairsShader(device, (Global::getGameDir() + "shaders/searchAndAddPair.spv").c_str());
    yavf::raii::ShaderModule calcOverlappingDataShader(device, (Global::getGameDir() + "shaders/calcOverlappingData.spv").c_str());
    // solvePhysics = pm.addSpecializationEntry(0, 0*sizeof(uint32_t), sizeof(uint32_t)).
    //                   addData(sizeof(uint32_t), &workGroupSize).
    //                   shader(VK_SHADER_STAGE_COMPUTE_BIT, Global::getGameDir() + "shaders/solver.spv").create("solvePhysics", solver_layout);

    posRecalc = pm.shader(posRecalcShader).create("posRecalc", pos_recalc_layout);
    overlapForSolver = pm.shader(overlapForSolverShader).create("overlapForSolver", overlap_data_for_solver_layout);
    newSolver = pm.shader(newSolverShader).create("newSolver", new_solver_layout);

    calcRayData = pm.shader(calcRayDataShader).create("calcRayData", ray_intersect_layout);
    searchPairs = pm.shader(searchPairsShader).create("searchPairs", search_pairs_layout);
    calcOverlappingData = pm.shader(calcOverlappingDataShader).create("calcOverlappingData1", calc_overlapping_layout);
  }

  {
    yavf::DescriptorMaker dm(device);

    {
      indirectOverlappingCalc.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
      memset(indirectOverlappingCalc.buffer()->ptr(), 0, sizeof( DataIndices ));

      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t index = d->add({indirectOverlappingCalc.buffer(), 0, indirectOverlappingCalc.buffer()->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      indirectOverlappingCalc.buffer()->setDescriptor(d, index);
    }

    {
      rayIndices.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
      memset(rayIndices.buffer()->ptr(), 0, sizeof( DataIndices ));

      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t index = d->add({rayIndices.buffer(), 0, rayIndices.buffer()->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      rayIndices.buffer()->setDescriptor(d, index);
    }

//     {
//       yavf::Descriptor d = dm.layout(physics_compute_layout).create(pool)[0];
// 
//       yavf::DescriptorUpdate du{
//         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//         0,
//         0,
//         d
//       };
// 
//       overlappingData.vector().setDescriptorData(du);
//     }
// 
//     {
//       yavf::Descriptor d = dm.layout(physics_compute_layout).create(pool)[0];
// 
//       yavf::DescriptorUpdate du{
//         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//         0,
//         0,
//         d
//       };
// 
//       rayData.vector().setDescriptorData(du);
//     }

    {
      overlapTmpBuffer = device->create(yavf::BufferCreateInfo::buffer(3000 * sizeof(OverlappingDataForSolver), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);

      yavf::DescriptorSet* d = dm.layout(physics_compute_layout).create(pool)[0];

      const size_t index = d->add({overlapTmpBuffer, 0, overlapTmpBuffer->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      overlapTmpBuffer->setDescriptor(d, index);
    }
  }
}

GPUSolverDirectPipeline::GPUSolverDirectPipeline(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUSolverData* data) : 
  GPUSolver(device, task, data) {}

GPUSolverDirectPipeline::~GPUSolverDirectPipeline() {}

void GPUSolverDirectPipeline::calculateData() {
  task->setPipeline(searchPairs);
  task->setDescriptor({pairs->handle(), overlappingDataDesc->handle(), indirectOverlappingCalc.buffer()->descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);

  task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->setPipeline(calcOverlappingData);
  task->setDescriptor({objData->handle(), transforms->handle(), overlappingDataDesc->handle(), indirectOverlappingCalc.buffer()->descriptorSet()->handle()}, 0);
  task->dispatchIndirect(indirectOverlappingCalc.buffer());
}

void GPUSolverDirectPipeline::calculateRayData() {
  task->setPipeline(calcRayData);
  task->setDescriptor({objData->handle(), transforms->handle(), rays->handle(), pairs->handle(), raysDataDesc->handle(), rayIndices.buffer()->descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);
}

void GPUSolverDirectPipeline::solve() {
  task->setPipeline(solvePhysics);
  task->setDescriptor({objData->handle(), transforms->handle(), pairs->handle(), islands->handle(), indicies->handle(), gravity->handle()}, 0);
  task->dispatch(1, 1, 1);
}
