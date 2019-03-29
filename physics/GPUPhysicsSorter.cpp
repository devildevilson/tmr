#include "GPUPhysicsSorter.h"
#include "Globals.h"

#include <chrono>

GPUPhysicsSorter::GPUPhysicsSorter(yavf::Device* device, yavf::ComputeTask* task, const GPUPhysicsSorterCreateInfo &info) {
  construct(device, task, info);
}

GPUPhysicsSorter::~GPUPhysicsSorter() {
  for (uint32_t i = 0; i < pairAlgos.size(); ++i) {
    device->destroy(pairAlgos[i]);
  }

  for (uint32_t i = 0; i < overlappingAlgos.size(); ++i) {
    device->destroy(overlappingAlgos[i]);
  }
}

void GPUPhysicsSorter::sort(ArrayInterface<BroadphasePair>* pairs, const uint32_t &algorithmIndex) {
  yavf::Buffer* buffer;
  pairs->descriptorPtr(&buffer);

  task->begin();

  task->setPipeline(pairAlgos[algorithmIndex]);
  task->setDescriptor(buffer->descriptorSet(), 0);
  task->dispatch(1, 1, 1);

  task->end();

  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Sorting pairs time: " << ns << " mcs" << "\n";
}

void GPUPhysicsSorter::sort(ArrayInterface<OverlappingData>* overlappingData, ArrayInterface<DataIndices>* dataIndixes, const uint32_t &algorithmIndex) {
  yavf::Buffer* buffer1;
  overlappingData->descriptorPtr(&buffer1);
  yavf::Buffer* buffer2;
  dataIndixes->descriptorPtr(&buffer2);

  task->begin();

  task->setPipeline(overlappingAlgos[algorithmIndex]);
  task->setDescriptor({buffer1->descriptorSet()->handle(), buffer2->descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);

  task->end();

  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Sorting overlapping data time: " << ns << " mcs" << "\n";
}

void GPUPhysicsSorter::barrier() {
  // task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
}

void GPUPhysicsSorter::printStats() {

}

void GPUPhysicsSorter::construct(yavf::Device* device, yavf::ComputeTask* task, const GPUPhysicsSorterCreateInfo &info) {
  this->device = device;
  this->task = task;

  yavf::DescriptorSetLayout physics_compute_layout = device->setLayout("physics_compute_layout");
  {
    yavf::DescriptorLayoutMaker dlm(device);

    if (physics_compute_layout == VK_NULL_HANDLE) {
      physics_compute_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("physics_compute_layout");
    }
  }

  yavf::PipelineLayout sort_pairs_layout = VK_NULL_HANDLE;
  yavf::PipelineLayout sort_overlapping_layout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);

    sort_pairs_layout = plm.addDescriptorLayout(physics_compute_layout).create("sort_pairs_layout");
    sort_overlapping_layout = plm.addDescriptorLayout(physics_compute_layout).
                                  addDescriptorLayout(physics_compute_layout).create("sort_overlapping_layout");
  }

  pairAlgos.resize(info.pairAlgo.size());
  overlappingAlgos.resize(info.overlappingAlgo.size());
  {
    yavf::ComputePipelineMaker cpm(device);

    for (uint32_t i = 0; i < pairAlgos.size(); ++i) {
      yavf::raii::ShaderModule compute(device, (Global::getGameDir() + info.pairAlgo[i]).c_str());
      pairAlgos[i] = cpm.shader(compute).create("pairAlgo"+std::to_string(i), sort_pairs_layout);
    }

    for (uint32_t i = 0; i < overlappingAlgos.size(); ++i) {
      yavf::raii::ShaderModule compute(device, (Global::getGameDir() + info.overlappingAlgo[i]).c_str());
      overlappingAlgos[i] = cpm.shader(compute).create("overlappingAlgo"+std::to_string(i), sort_overlapping_layout);
    }
  }
}

GPUPhysicsSorterDirectPipeline::GPUPhysicsSorterDirectPipeline(yavf::Device* device, yavf::ComputeTask* task, const GPUPhysicsSorterCreateInfo &info) : 
  GPUPhysicsSorter(device, task, info) {}

GPUPhysicsSorterDirectPipeline::~GPUPhysicsSorterDirectPipeline() {
  for (uint32_t i = 0; i < pairAlgos.size(); ++i) {
    device->destroy(pairAlgos[i]);
  }

  for (uint32_t i = 0; i < overlappingAlgos.size(); ++i) {
    device->destroy(overlappingAlgos[i]);
  }
}

void GPUPhysicsSorterDirectPipeline::sort(ArrayInterface<BroadphasePair>* pairs, const uint32_t &algorithmIndex) {
  yavf::Buffer* buffer;
  pairs->descriptorPtr(&buffer);

  task->setPipeline(pairAlgos[algorithmIndex]);
  task->setDescriptor(buffer->descriptorSet(), 0);
  task->dispatch(1, 1, 1);
}

void GPUPhysicsSorterDirectPipeline::sort(ArrayInterface<OverlappingData>* overlappingData, ArrayInterface<DataIndices>* dataIndixes, const uint32_t &algorithmIndex) {
  yavf::Buffer* buffer1;
  overlappingData->descriptorPtr(&buffer1);
  yavf::Buffer* buffer2;
  dataIndixes->descriptorPtr(&buffer2);

  task->setPipeline(overlappingAlgos[algorithmIndex]);
  task->setDescriptor({buffer1->descriptorSet()->handle(), buffer2->descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);
}

void GPUPhysicsSorterDirectPipeline::barrier() {
  // task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
  //                  VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

  task->setBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
}
