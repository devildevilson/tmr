#include "stages.h"
#include "context.h"
#include "yavf.h"
#include "targets.h"
#include "Globals.h"
#include "scene_data.h"
#include "image_data.h"
#include "window.h"
#include "interface_context.h"
#include "input.h"
#include "particles.h"

namespace devils_engine {
  namespace render {
    window_next_frame::window_next_frame(const create_info &info) : w(info.w) {}
    void window_next_frame::begin() {}
    void window_next_frame::proccess(context* ctx) { w->next_frame(); (void)ctx; }
    void window_next_frame::clear() {}
//     update_buffers::update_buffers(const create_info &info) : perspective(true), mat(new matrices), uniform(info.uniform), matrix(info.matrix) {}
//     update_buffers::~update_buffers() { delete mat; }
//     void update_buffers::begin() {}
//     void update_buffers::proccess(context* ctx) { (void)ctx; }
//     void update_buffers::clear() {}
    void task_begin::begin() {}
    void task_begin::proccess(context* ctx) { ctx->interface()->begin(); }
    void task_begin::clear() {}
    void task_end::begin() {}
    void task_end::proccess(context* ctx) { ctx->interface()->end(); }
    void task_end::clear() {}
    gbuffer_begin::gbuffer_begin(const create_info &info) : target(info.target) {}
    void gbuffer_begin::begin() {}
    void gbuffer_begin::proccess(context* ctx) {
//       ASSERT(target->wall_renderpass() == target->renderPass());
      ctx->graphics()->setRenderTarget(target);
      ctx->graphics()->beginRenderPass();
    }
    
    void gbuffer_begin::clear() {}
    
    gbuffer_end::gbuffer_end(const create_info &info) : target(info.target) {}
    void gbuffer_end::begin() {}
    void gbuffer_end::proccess(context* ctx) {
      ctx->graphics()->endRenderPass();
      target->change_pass();
    }
    
    void gbuffer_end::clear() {}
    
    post_begin::post_begin(const create_info &info) : w(info.w) {}
    void post_begin::begin() {}
    void post_begin::proccess(context* ctx) {
      ctx->graphics()->setRenderTarget(w);
      ctx->graphics()->beginRenderPass();
    }
    
    void post_begin::clear() {}
    
    post_end::post_end(const create_info &info) : w(info.w) {}
    void post_end::begin() {}
    void post_end::proccess(context* ctx) {
      ctx->graphics()->endRenderPass();
    }
    
    void post_end::clear() {}
    
    struct MonsterOptimizerCount {
      uint32_t count;
      uint32_t dummy1;
      uint32_t dummy2;
      uint32_t dummy3;
      glm::vec4 playerRotation;
    };
    
    monster_optimizer::monster_optimizer(const create_info &info) : 
      device(info.device), 
      uniform(info.uniform), 
      obj_count(nullptr),
      transforms(info.transforms), 
      matrices(info.matrices), 
      textures(info.textures), 
      
//       inst_datas_array(info.inst_datas_array), 
      inst_datas_array(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), 
      indices(device) 
    {
      yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
      yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      {
        yavf::DescriptorMaker dm(device);
        
        auto desc = dm.layout(storage_layout).create(pool)[0];
        const size_t i = desc->add({indices.vector().handle(), 0, indices.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        indices.vector().setDescriptor(desc, i);
      }
      
      // создать дескриптор
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      
      yavf::PipelineLayout pipeLayout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        
        pipeLayout = plm.addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(storage_layout)
                        .addDescriptorLayout(storage_layout)
                        .addDescriptorLayout(storage_layout)
                        .addDescriptorLayout(storage_layout)
                        .addDescriptorLayout(storage_layout)
                        .create("monster_optimizer_pipeline_layout");
      }
      
      {
        yavf::ComputePipelineMaker cpm(device);
        
        yavf::raii::ShaderModule shader(device, Global::getGameDir() + "shaders/monsterOptimizer.comp.spv");
        
        pipe = cpm.shader(shader).create("monster_optimizer_pipeline", pipeLayout);
      }
      
      obj_count = device->create(yavf::BufferCreateInfo::buffer(sizeof(MonsterOptimizerCount), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      {
        yavf::DescriptorMaker dm(device);
        
        auto desc = dm.layout(uniform_layout).create(pool)[0];
        const size_t i = desc->add({obj_count, 0, obj_count->info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
        obj_count->setDescriptor(desc, i);
      }
      
      {
        yavf::DescriptorMaker dm(device);
        
        auto desc = dm.layout(storage_layout).create(pool)[0];
        const size_t i = desc->add({inst_datas_array.vector().handle(), 0, inst_datas_array.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        inst_datas_array.vector().setDescriptor(desc, i);
      }
    }
    
    monster_optimizer::~monster_optimizer() {
      device->destroyPipeline("monster_optimizer_pipeline");
      device->destroyLayout("monster_optimizer_pipeline_layout");
      device->destroy(obj_count);
    }
      
    void monster_optimizer::add(const object_indices &idx) {
      std::unique_lock<std::mutex> lock(mutex);
      indices.push_back(idx);
    }
    
    void monster_optimizer::begin() {
      auto* data = reinterpret_cast<MonsterOptimizerCount*>(obj_count->ptr());
      data->count = indices.size();
      const glm::vec3 rot = Global::getPlayerRot();
      data->playerRotation = glm::vec4(rot.x, rot.y, rot.z, 0.0f);
      
      if (indices.size() > inst_datas_array.size()) {
        inst_datas_array.resize(indices.size());
      }
    }
    
    #define WORKGROUP_SIZE 256
    
    void monster_optimizer::proccess(context* ctx) {
      if (indices.size() == 0) return;
  
      yavf::ComputeTask* task = ctx->compute();
      
      yavf::Buffer* transforms = reinterpret_cast<yavf::Buffer*>(this->transforms->gpu_buffer());
      yavf::Buffer* matrices = reinterpret_cast<yavf::Buffer*>(this->matrices->gpu_buffer());
      yavf::Buffer* textures = reinterpret_cast<yavf::Buffer*>(this->textures->gpu_buffer());
      yavf::Buffer* instDatas = reinterpret_cast<yavf::Buffer*>(this->inst_datas_array.gpu_buffer());
      
      ASSERT(transforms != nullptr);
      ASSERT(matrices != nullptr);
      ASSERT(textures != nullptr);
      ASSERT(instDatas != nullptr);
      
      task->setPipeline(pipe);
      task->setDescriptor({
        uniform->descriptorSet()->handle(),
        obj_count->descriptorSet()->handle(),

        transforms->descriptorSet()->handle(),
        matrices->descriptorSet()->handle(),
        textures->descriptorSet()->handle(),

        instDatas->descriptorSet()->handle(),

        indices.vector().descriptorSet()->handle()
      }, 0);

      const uint32_t count = std::ceil(float(indices.size()) / float(workgroup_size));
      ASSERT(count > 0);
      task->dispatch(count, 1, 1);

    //  auto* data = reinterpret_cast<MonsterOptimizerCount*>(objCount->ptr());
    //  const size_t count = data->count;
    //  for (size_t i = 0; i < count; ++i) {
    //    const uint transformIndex = indices[i].transform;
    //    const uint matrixIndex = indices[i].matrix;
    //    const uint textureIndex = indices[i].texture;
    //
    //    const simd::vec4 pos = this->transforms->at(transformIndex).pos;
    //    const simd::vec4 vulkanScale = this->transforms->at(transformIndex).scale * simd::vec4(1.0f, -1.0f, 1.0f, 0.0f);
    //
    //    const simd::mat4 matOne = simd::mat4(
    //            1.0f, 0.0f, 0.0f, 0.0f,
    //            0.0f, 1.0f, 0.0f, 0.0f,
    //            0.0f, 0.0f, 1.0f, 0.0f,
    //            0.0f, 0.0f, 0.0f, 1.0f
    //    );
    //
    //    const glm::vec3 rot = Global::getPlayerRot();
    //
    //    simd::mat4 mat = simd::translate(matOne, pos);
    //    mat = simd::rotate(mat, PI_H - rot.y, simd::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    //    mat = simd::scale(mat, vulkanScale);
    //
    //    mat[0].storeu(&instDatasArray->at(i).mat[0].x);
    //    mat[1].storeu(&instDatasArray->at(i).mat[1].x);
    //    mat[2].storeu(&instDatasArray->at(i).mat[2].x);
    //    mat[3].storeu(&instDatasArray->at(i).mat[3].x);
    //
    //    // тут же можно умножить эту матрицу на вид, чтобы не делать это в вершинном шейдере
    //
    //    instDatasArray->at(i).textureData = this->textures->at(textureIndex);
    //  }
    }
    
    void monster_optimizer::clear() {
      indices.vector().clear();
      indices.update();
    }
    
    uint32_t monster_optimizer::instances_count() const {
      return indices.size();
    }
    
    GPUArray<monster_optimizer::instance_data>* monster_optimizer::instance_datas() {
      return &inst_datas_array;
    }
    
    struct GeometryOptimizerCount {
      uint32_t count;
      uint32_t internalVar;
      uint32_t dummy2;
      uint32_t dummy3;
    };
    
    geometry_optimizer::geometry_optimizer(const create_info &info) : 
      device(info.device), 
      uniform(info.uniform), 
      obj_count(nullptr),
      face_count(0),
      indices_count(0),
      transforms(info.transforms), 
      matrices(info.matrices), 
      rotation_datas(info.rotation_datas), 
      textures(info.textures), 
//       indices_array(info.indices_array), 
//       instance_datas(info.instance_datas),
      indices_array(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT),
      instance_datas(device),
      objs(device)
    {
      obj_count = device->create(yavf::BufferCreateInfo::buffer(sizeof(GeometryOptimizerCount), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
  
      yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
      yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout layout = VK_NULL_HANDLE;
      {
        yavf::DescriptorLayoutMaker dlm(device);
        
        layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                    binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                    create("geometry_optimizer_descriptor_layout");
      }
      
      {
        yavf::DescriptorMaker dm(device);
        
        auto desc = dm.layout(layout).create(pool)[0];
        size_t i = desc->add({obj_count, 0, obj_count->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        obj_count->setDescriptor(desc, i);
        i = desc->add({objs.vector().handle(), 0, objs.vector().buffer_size(), 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        objs.vector().setDescriptor(desc, i);
      }
      
      // создать дескриптор
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout instances_layout = device->setLayout("geometry_rendering_data");
      
      {
        yavf::DescriptorLayoutMaker dlm(device);
        
        if (instances_layout == VK_NULL_HANDLE) {
          instances_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT).
                                 binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                                 create("geometry_rendering_data");
        }
      }
      
      {
        yavf::DescriptorMaker dm(device);
        
        auto desc = dm.layout(instances_layout).create(pool)[0];
        size_t i = desc->add({instance_datas.vector().handle(), 0, instance_datas.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        instance_datas.vector().setDescriptor(desc, i);
               i = desc->add({indices_array.vector().handle(), 0, indices_array.vector().buffer_size(), 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        indices_array.vector().setDescriptor(desc, i);
      }
      
      yavf::PipelineLayout pipeLayout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        
        pipeLayout = plm.addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(layout)
                        .addDescriptorLayout(storage_layout)
                        .addDescriptorLayout(storage_layout)
                        .addDescriptorLayout(storage_layout)
                        .addDescriptorLayout(storage_layout)
                        .addDescriptorLayout(instances_layout)
                        .create("geometry_optimizer_pipeline_layout");
      }
      
      {
        yavf::ComputePipelineMaker cpm(device);
        
        yavf::raii::ShaderModule shader(device, Global::getGameDir() + "shaders/geometryOptimizer.comp.spv"); 
        
        pipe = cpm.shader(shader).create("geometry_optimizer_pipeline", pipeLayout);
      }
    }
    
    geometry_optimizer::~geometry_optimizer() {
      device->destroyPipeline("geometry_optimizer_pipeline");
      device->destroyLayout("geometry_optimizer_pipeline_layout");
      device->destroy(obj_count);
    }
    
    void geometry_optimizer::add(const object_indices &idx) {
      std::unique_lock<std::mutex> lock(mutex);
  
      objs.push_back(idx);
      face_count = glm::max(idx.faceIndex+1, face_count);
      indices_count += idx.vertexCount + 1; 
      
      //const auto tex = textures->at(idx.texture);

      ASSERT(idx.vertexCount >= 3);
    }
    
    void geometry_optimizer::begin() {
      GeometryOptimizerCount* data = reinterpret_cast<GeometryOptimizerCount*>(obj_count->ptr());
      data->count = objs.size();
      data->internalVar = 0;
      
      if (indices_count > indices_array.size()) {
        indices_array.resize(indices_count);
      }
    
      if (face_count > instance_datas.size()) {
        instance_datas.resize(face_count);
      }
    }
    
    void geometry_optimizer::proccess(context* ctx) {
      if (objs.size() == 0) return;
  
      yavf::ComputeTask* task = ctx->compute();
      
      yavf::Buffer* transforms = reinterpret_cast<yavf::Buffer*>(this->transforms->gpu_buffer());
      yavf::Buffer* matrices = reinterpret_cast<yavf::Buffer*>(this->matrices->gpu_buffer());
      yavf::Buffer* rotationDatas = reinterpret_cast<yavf::Buffer*>(this->rotation_datas->gpu_buffer());
      yavf::Buffer* textures = reinterpret_cast<yavf::Buffer*>(this->textures->gpu_buffer());
      yavf::Buffer* indices = reinterpret_cast<yavf::Buffer*>(this->indices_array.gpu_buffer());
    //  auto* instances = reinterpret_cast<yavf::Buffer*>(this->instanceDatas->gpu_buffer());
      
      ASSERT(transforms != nullptr);
      ASSERT(matrices != nullptr);
      ASSERT(rotationDatas != nullptr);
      ASSERT(textures != nullptr);
      ASSERT(indices != nullptr);

      // чтобы удобнее и быстрее обновлять дескрипторы, можно сделать descriptor view
      // в нем будет содержаться VkDescriptorUpdateTemplate, чаще всего нужно обновить целый дескриптор
      // также хорошей идеей будет отказаться от обновления дескриптора внутри буфера, и обновлять дескриптор отдельно
      
      // больше 8 дескрипторов драйвер нвидии не поддерживает на линухе =(
      task->setPipeline(pipe);
      task->setDescriptor({
        uniform->descriptorSet()->handle(),
        obj_count->descriptorSet()->handle(),

        transforms->descriptorSet()->handle(),
        matrices->descriptorSet()->handle(),
        rotationDatas->descriptorSet()->handle(),
        textures->descriptorSet()->handle(),

        indices->descriptorSet()->handle()
      }, 0); // instDatas->descriptorSet()->handle()

      const uint32_t count = std::ceil(float(objs.size()) / float(workgroup_size));
      ASSERT(count > 0);
      task->dispatch(count, 1, 1);

//  static bool first = false;
//
//  memset(indicesArray->data(), 0, indicesArray->size()*sizeof(indicesArray->at(0)));
//  memset(instanceDatas->data(), 0, instanceDatas->size()*sizeof(instanceDatas->at(0)));
//
//  auto* data = reinterpret_cast<GeometryOptimizerCount*>(objCount->ptr());
//  const size_t count = data->count;
//  for (size_t i = 0; i < count; ++i) {
//    const uint32_t vertexOffset = objs[i].vertexOffset;
//    const uint32_t vertexCount = objs[i].vertexCount;
//
//    const uint32_t start = data->internalVar;
//    data->internalVar += vertexCount+1;
//
//    for (uint32_t j = 0; j < vertexCount; ++j) {
//      indicesArray->at(start+j) = vertexOffset + j;
//    }
//
//    indicesArray->at(start+vertexCount) = UINT32_MAX;
//
//    const uint32_t textureIndex = objs[i].texture;
//    const uint32_t faceIndex = objs[i].faceIndex;
//
//    instanceDatas->at(faceIndex).textureData = this->textures->at(textureIndex);
//    //instanceDatas->at(faceIndex).textureData.t.imageArrayLayer = instanceDatas->at(faceIndex).textureData.t.imageArrayLayer != 1 ? 5 : instanceDatas->at(faceIndex).textureData.t.imageArrayLayer;
//
////    PRINT_VAR("faceIndex", faceIndex)
//
////    if (!first) {
////      if (instanceDatas->at(faceIndex).textureData.t.imageArrayLayer == 1) {
////        PRINT_VAR("textureIndex", textureIndex)
////        PRINT_VAR("faceIndex   ", faceIndex)
////        PRINT_VAR("data->count ", data->count)
////        PRINT_VAR("i           ", i)
////        first = true;
////
//////        throw std::runtime_error("no more");
////      }
////    }
//  }

//  PRINT_VAR("count", count)

//  throw std::runtime_error("no more");
    }
    
    void geometry_optimizer::clear() {
      objs.vector().clear();
      objs.update();
      
      indices_count = 0;
    }
    
    GPUArray<uint32_t>* geometry_optimizer::indices() {
      return &indices_array;
    }
    
    GPUArray<geometry_optimizer::instance_data>* geometry_optimizer::instances() {
      return &instance_datas;
    }
    
    uint32_t geometry_optimizer::get_indices_count() const {
      return indices_count;
    }
    
    compute_particles::compute_particles(const create_info &info) : device(info.device), uniform(info.uniform), matrices(info.matrices), target(info.target), particles(info.particles) {
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
//       yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout target_layout = target->layout();
      
      yavf::PipelineLayout pipeLayout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        
        pipeLayout = plm.addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(target_layout)
                        .addDescriptorLayout(particles->layout)
                        .create("compute_particles_pipeline_layout");
      }
      
      {
        yavf::ComputePipelineMaker cpm(device);
        
        yavf::raii::ShaderModule shader(device, Global::getGameDir() + "shaders/particles2.comp.spv"); 
        
        pipe = cpm.shader(shader).create("compute_particles_pipeline", pipeLayout);
      }
    }
    
    void compute_particles::begin() {
      auto data = reinterpret_cast<particles_data*>(particles->data_buffer.ptr());
      data->old_count = data->new_max_count;
      const uint32_t count = particles->next_index + data->old_count;
      data->new_count = particles->next_index;
      data->particles_count = count;
      data->new_max_count = 0;
      data->indices_count = 0;
      
      if (count > particles->array.size()) {
        particles->array.resize(count);
//         std::cout << "particles->array size " << count << "\n";
      }
      
      if (count * sizeof(uint32_t) > particles->indices.info().size) {
        particles->indices.recreate(count * sizeof(uint32_t));
      }
    }
    
    void compute_particles::proccess(context* ctx) {
      auto data = reinterpret_cast<particles_data*>(particles->data_buffer.ptr());
      if (data->particles_count == 0) return;
      
      auto task = ctx->compute();
      
      task->setPipeline(pipe);
      task->setDescriptor({
        uniform->descriptorSet()->handle(), 
        matrices->descriptorSet()->handle(), 
        target->current_frame_data()->desc->handle(), 
//         particles->data_buffer.descriptorSet()->handle(), 
//         particles->new_particles.descriptorSet()->handle(), 
        particles->array.vector().descriptorSet()->handle(),
//         particles->indices.descriptorSet()->handle()
      }, 0);
      const uint32_t dispatch = std::ceil(float(data->particles_count) / float(workgroup_size));
      task->dispatch(dispatch, 1, 1);
    }
    
    void compute_particles::clear() {
      particles->reset();
    }
    
    //instance_datas(info.instance_datas)
    monster_gbuffer::monster_gbuffer(const create_info &info) : device(info.device), monster_default(nullptr), uniform(info.uniform), opt(info.opt), target(info.target) {
      // я забыл переделать буферы хранящие данные карты, поэтому вот эти буферы у меня просто перезаписывались
      {
        monster_default = device->create(yavf::BufferCreateInfo::buffer(monster_default_vertices_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
        yavf::Buffer buffer(device, yavf::BufferCreateInfo::buffer(monster_default_vertices_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
        
        memcpy(buffer.ptr(), monster_default_vertices, monster_default_vertices_size);
        //memcpy(monsterDefaultStaging->ptr(), monsterDefaultVertices, 4*sizeof(Vertex));
        
        yavf::TransferTask* task = device->allocateTransferTask();
        
        task->begin();
        task->copy(&buffer, monster_default, 0, 0, monster_default_vertices_size);
        task->end();
        
        task->start();
        task->wait();
        
        device->deallocate(task);
      }
    }
    
    monster_gbuffer::~monster_gbuffer() {
      device->destroy(monster_default);
    }
    
    void monster_gbuffer::begin() {}
    void monster_gbuffer::proccess(context* ctx) {
      // в будущем нужно будет передавать верный сабпасс сюда
      const uint32_t instanceCount = opt->instances_count();
      if (instanceCount == 0) return;
      
      yavf::GraphicTask* task = ctx->graphics();
      
      task->setPipeline(pipe);
      //task->setDescriptor({uniformBuffer->descriptorSet()->handle(), samplers->handle(), images->handle()}, 0);
      task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle()}, 0);
      task->setVertexBuffer(opt->instance_datas()->vector().handle(), 0);
      task->setVertexBuffer(monster_default, 1);
      task->draw(monster_default_vertices_count, instanceCount, 0, 0);
    }
    
    #define MONSTER_PIPELINE_LAYOUT_NAME "deferred_layout"
    #define MONSTER_PIPELINE_NAME "deferred_monster_pipeline"
    
    void monster_gbuffer::clear() {}
    void monster_gbuffer::recreate_pipelines(const game::image_resources_t *resource) {
      this->images_set = resource->set;
  
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      //VkDescriptorSetLayout storage_layout = device->setLayout("storage_layout");
      
      yavf::PipelineLayout deferred_layout = device->layout(MONSTER_PIPELINE_LAYOUT_NAME);
      if (deferred_layout != VK_NULL_HANDLE) {
        device->destroyLayout(MONSTER_PIPELINE_LAYOUT_NAME);
        deferred_layout = VK_NULL_HANDLE;
      }
      
      {
        yavf::PipelineLayoutMaker plm(device);
        
        deferred_layout = plm.addDescriptorLayout(uniform_layout)
                            .addDescriptorLayout(resource->layout)
    //                         .addDescriptorLayout(data->imageSetLayout())
    //                          .addDescriptorLayout(minmax_layout)
                            .create(MONSTER_PIPELINE_LAYOUT_NAME);
      }

      //uint32_t constants[2] = {data->samplerCount(), data->imageCount()};
      uint32_t constants[2] = {resource->images, resource->samplers};
      
      if (pipe.handle() != VK_NULL_HANDLE) {
        device->destroyPipeline(MONSTER_PIPELINE_NAME);
        pipe = yavf::Pipeline(VK_NULL_HANDLE, VK_NULL_HANDLE);
      }
      
      {
        yavf::raii::ShaderModule vertex  (device, Global::getGameDir() + "shaders/deferredObj.vert.spv");
        yavf::raii::ShaderModule fragment(device, Global::getGameDir() + "shaders/deferredObj.frag.spv");
        
        yavf::PipelineMaker pm(device);
        pm.clearBlending();
        
        // почему то получается какая то параша если MonsterGPUOptimizer::InstanceData не выровнен по 16 байт
        // с этим кстати могут быть связаны и эти идиотские лаги при движение вперед всторону и при повороте камеры
        // если не учитывать того что этих лагов нет в вине и винде
        // возможно что нужно переустановить драйверы и vulkan, надо кстати на венде проверить выравнивание по 16 байт
        
        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                  .addSpecializationEntry(0, 0, sizeof(uint32_t))
                  .addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t))
                  .addData(2*sizeof(uint32_t), constants)
                .vertexBinding(0, sizeof(monster_optimizer::instance_data), VK_VERTEX_INPUT_RATE_INSTANCE) // никогда не забывать заполнить эти поля ВЕРНО
                  .vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(monster_optimizer::instance_data, mat) + sizeof(glm::vec4)*0)
                  .vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(monster_optimizer::instance_data, mat) + sizeof(glm::vec4)*1)
                  .vertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(monster_optimizer::instance_data, mat) + sizeof(glm::vec4)*2)
                  .vertexAttribute(3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(monster_optimizer::instance_data, mat) + sizeof(glm::vec4)*3)
                  .vertexAttribute(4, 0, VK_FORMAT_R32_UINT,   offsetof(monster_optimizer::instance_data, textureData) + offsetof(image_data, img) + offsetof(image, container))
//                   .vertexAttribute(5, 0, VK_FORMAT_R32_UINT,   offsetof(monster_optimizer::instance_data, textureData) + offsetof(Texture, image) + offsetof(Image, layer))
//                   .vertexAttribute(6, 0, VK_FORMAT_R32_UINT,   offsetof(monster_optimizer::instance_data, textureData) + offsetof(Texture, samplerIndex))
                  //.vertexAttribute(7,  0, VK_FORMAT_R32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, mirroredU))
                  //.vertexAttribute(8,  0, VK_FORMAT_R32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, mirroredV))
                  .vertexAttribute(5, 0, VK_FORMAT_R32_SFLOAT, offsetof(monster_optimizer::instance_data, textureData) + offsetof(image_data, movementU))
                  .vertexAttribute(6, 0, VK_FORMAT_R32_SFLOAT, offsetof(monster_optimizer::instance_data, textureData) + offsetof(image_data, movementV))
                .vertexBinding(1, sizeof(struct vertex))
                  .vertexAttribute(7,  1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(struct vertex, pos))
                  .vertexAttribute(8, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(struct vertex, color))
                  .vertexAttribute(9, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(struct vertex, tex_coord))
                .depthTest(VK_TRUE)
                .depthWrite(VK_TRUE)
                .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
                .viewport()
                .scissor()
                .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                .colorBlendBegin(VK_FALSE)
                  .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                .colorBlendBegin(VK_FALSE)
                  .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
    //           .rasterizationSamples(VK_SAMPLE_COUNT_8_BIT)
                //.create(MONSTER_PIPELINE_NAME, deferred_layout, target->next_renderpass());
                  .create(MONSTER_PIPELINE_NAME, deferred_layout, target->next_renderpass());
      }
    }
    
    #define GEOMETRY_PIPELINE_LAYOUT_NAME "deferred_layout2"
    #define GEOMETRY_PIPELINE_NAME "deferred_geometry_pipeline"
    //instance_datas(info.instance_datas),
    geometry_gbuffer::geometry_gbuffer(const create_info &info) : device(info.device), uniform(info.uniform), world_map_vertex(info.world_map_vertex), opt(info.opt), target(info.target) {}
    geometry_gbuffer::~geometry_gbuffer() {
      device->destroy(pipe.layout());
      device->destroy(pipe);
    }
    
    void geometry_gbuffer::begin() {}
    void geometry_gbuffer::proccess(context* ctx) {
      const uint32_t indexCount = opt->get_indices_count();
      if (indexCount == 0) return;
      
      yavf::GraphicTask* task = ctx->graphics();
      
      task->setPipeline(pipe);
      //task->setDescriptor({uniformBuffer->descriptorSet()->handle(), samplers->handle(), images->handle(), instances.vector().descriptorSet()->handle()}, 0);
      task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), opt->instances()->vector().descriptorSet()->handle()}, 0);
      task->setVertexBuffer(world_map_vertex, 0);
      task->setIndexBuffer(opt->indices()->vector().handle());
      task->drawIndexed(indexCount, 1, 0, 0, 0);
    }
    
    void geometry_gbuffer::clear() {}
    void geometry_gbuffer::recreate_pipelines(const game::image_resources_t *resource) {
      this->images_set = resource->set;
  
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout instances_layout = device->setLayout("geometry_rendering_data");
      
      yavf::PipelineLayout deferred_layout2 = device->layout(GEOMETRY_PIPELINE_LAYOUT_NAME);
      if (deferred_layout2 != VK_NULL_HANDLE) {
        device->destroyLayout(GEOMETRY_PIPELINE_LAYOUT_NAME);
        deferred_layout2 = VK_NULL_HANDLE;
      }
      
      {
        yavf::PipelineLayoutMaker plm(device);
        
        deferred_layout2 = plm.addDescriptorLayout(uniform_layout)
                              .addDescriptorLayout(resource->layout)
    //                          .addDescriptorLayout(data->imageSetLayout())
                              //.addDescriptorLayout(storage_layout)
                              .addDescriptorLayout(instances_layout)
                              .create(GEOMETRY_PIPELINE_LAYOUT_NAME);
      }
      
      uint32_t constants[2] = {resource->images, resource->samplers};
      
      if (pipe.handle() != VK_NULL_HANDLE) {
        device->destroyPipeline(GEOMETRY_PIPELINE_NAME);
        pipe = yavf::Pipeline(VK_NULL_HANDLE, VK_NULL_HANDLE);
      }
      
      {
        yavf::raii::ShaderModule vertex  (device, Global::getGameDir() + "shaders/deferred.vert.spv");
        yavf::raii::ShaderModule fragment(device, Global::getGameDir() + "shaders/deferred.frag.spv");
        
        yavf::PipelineMaker pm(device);
        pm.clearBlending();
        
        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                   .addSpecializationEntry(0, 0, sizeof(uint32_t))
                   .addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t))
                   .addData(2*sizeof(uint32_t), constants)
                 .vertexBinding(0, sizeof(struct vertex))
                   .vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(struct vertex, pos))
                   .vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(struct vertex, color))
                   .vertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(struct vertex, tex_coord))
//                  .vertexBinding(1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE)
//                    .vertexAttribute(0, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0 * sizeof(glm::vec4))
//                    .vertexAttribute(1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 1 * sizeof(glm::vec4))
//                    .vertexAttribute(2, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 2 * sizeof(glm::vec4))
//                    .vertexAttribute(3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 3 * sizeof(glm::vec4))
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
    //              .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
    //                          .rasterizationSamples(VK_SAMPLE_COUNT_8_BIT)
                 .create(GEOMETRY_PIPELINE_NAME, deferred_layout2, target->wall_renderpass());
      }
    }
    
#define PARTICLES_PIPELINE_LAYOUT_NAME "particles_pipeline_layout"
#define PARTICLES_PIPELINE_NAME "particles_pipeline"
    
    particles_gbuffer::particles_gbuffer(const create_info &info) : device(info.device), uniform(info.uniform), target(info.target), particles(info.particles), images_set(nullptr) {}
    void particles_gbuffer::begin() {
      
    }
    
    void particles_gbuffer::proccess(context* ctx) {
//       auto data = reinterpret_cast<particles_data*>(particles->data_buffer.ptr());
//       if (data->particles_count == 0) return;
      
      //std::cout << "data->indices_count " << data->indices_count << "\n";
      
      auto task = ctx->graphics();
      
      task->setPipeline(pipe);
      task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), particles->array.vector().descriptorSet()->handle()}, 0);
      task->setVertexBuffer(&particles->indices, 0, sizeof(VkDrawIndirectCommand));
//       task->draw(data->particles_count, 1, 0, 0);
      task->drawIndirect(&particles->indices, 1);
    }
    
    void particles_gbuffer::clear() {}
    void particles_gbuffer::recreate_pipelines(const game::image_resources_t* resource) {
      this->images_set = resource->set;
  
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout instances_layout = particles->layout;
      
      yavf::PipelineLayout patricles_layout = device->layout(PARTICLES_PIPELINE_LAYOUT_NAME);
      if (patricles_layout != VK_NULL_HANDLE) {
        device->destroyLayout(PARTICLES_PIPELINE_LAYOUT_NAME);
        patricles_layout = VK_NULL_HANDLE;
      }
      
      {
        yavf::PipelineLayoutMaker plm(device);
        
        patricles_layout = plm.addDescriptorLayout(uniform_layout)
                              .addDescriptorLayout(resource->layout)
                              .addDescriptorLayout(instances_layout)
                              .create(PARTICLES_PIPELINE_LAYOUT_NAME);
      }
      
      uint32_t constants[2] = {resource->images, resource->samplers};
      
      if (pipe.handle() != VK_NULL_HANDLE) {
        device->destroyPipeline(PARTICLES_PIPELINE_NAME);
        pipe = yavf::Pipeline(VK_NULL_HANDLE, VK_NULL_HANDLE);
      }
      
      {
        yavf::raii::ShaderModule vertex  (device, Global::getGameDir() + "shaders/patricles2.vert.spv");
        yavf::raii::ShaderModule geom    (device, Global::getGameDir() + "shaders/particles2.geom.spv");
        yavf::raii::ShaderModule fragment(device, Global::getGameDir() + "shaders/particles2.frag.spv");
        
        yavf::PipelineMaker pm(device);
        pm.clearBlending();
        
        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_GEOMETRY_BIT, geom)
                .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                  .addSpecializationEntry(0, 0, sizeof(uint32_t))
                  .addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t))
                  .addData(2*sizeof(uint32_t), constants)
                .vertexBinding(0, sizeof(uint32_t))
                  .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                .depthTest(VK_TRUE)
                .depthWrite(VK_TRUE)
                .assembly(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
                .viewport()
                .scissor()
                .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                .colorBlendBegin(VK_FALSE)
                  .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                .colorBlendBegin(VK_FALSE)
                  .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                .create(PARTICLES_PIPELINE_NAME, patricles_layout, target->next_renderpass());
      }
    }
    
#define SKYBOX_PIPELINE_LAYOUT_NAME "skybox_pipeline_layout"
#define SKYBOX_PIPELINE_NAME "skybox_pipeline"
    
    skybox_gbuffer::skybox_gbuffer(const create_info &info) : 
      device(info.device),
      uniform(info.uniform),
      cube_vertices(nullptr),
      target(info.target),
      skybox_set(info.skybox_set)
    {
      {
        cube_vertices = device->create(yavf::BufferCreateInfo::buffer(cube_strip_vertices_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
        yavf::Buffer staging(device, yavf::BufferCreateInfo::buffer(cube_strip_vertices_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
        
        memcpy(staging.ptr(), cube_strip_vertices, cube_strip_vertices_size);
        
        auto trans = device->allocateTransferTask();
        trans->begin();
        trans->copy(&staging, cube_vertices);
        trans->end();
        
        trans->start();
        trans->wait();
        
        device->deallocate(trans);
      }
      
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout skybox_set_layout = device->setLayout(SKYBOX_TEXTURE_LAYOUT_NAME);
      if (skybox_set_layout == VK_NULL_HANDLE) {
        yavf::DescriptorLayoutMaker dlm(device);
        
        skybox_set_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).create(SKYBOX_TEXTURE_LAYOUT_NAME);
      }
      
      yavf::PipelineLayout skybox_layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        
        skybox_layout = plm.addDescriptorLayout(uniform_layout)
                           .addDescriptorLayout(skybox_set_layout)
                           .create(SKYBOX_PIPELINE_LAYOUT_NAME);
      }
      
      {
        yavf::raii::ShaderModule vertex  (device, Global::getGameDir() + "shaders/skybox.vert.spv");
        yavf::raii::ShaderModule fragment(device, Global::getGameDir() + "shaders/skybox.frag.spv");
        
        yavf::PipelineMaker pm(device);
        pm.clearBlending();
        
        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                 .vertexBinding(0, sizeof(struct vertex)) // ?
                   .vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(struct vertex, pos))
                   .vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(struct vertex, color))
                   .vertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(struct vertex, tex_coord))
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_FALSE)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create(SKYBOX_PIPELINE_NAME, skybox_layout, target->next_renderpass());
      }
    }
    
    void skybox_gbuffer::begin() {}
    void skybox_gbuffer::proccess(context* ctx) {
      auto task = ctx->graphics();
      
      task->setPipeline(pipe);
      task->setDescriptor({uniform->descriptorSet()->handle(), skybox_set->handle()}, 0);
      task->setVertexBuffer(cube_vertices, 0);
      task->draw(cube_strip_vertices_count, 1, 0, 0);
    }
    
    void skybox_gbuffer::clear() {}
//     void skybox_gbuffer::recreate_pipelines(const game::image_resources_t* resource) {
//       skybox_set = resource->skybox_set;
//     }

    const uint32_t initial_vertex_count = 1000;
    decal_gbuffer::decal_gbuffer(const create_info &info) : 
      device(info.device), 
      uniform(info.uniform), 
      target(info.target), 
      images_set(nullptr), 
      decal_vertices(nullptr),
      current_vertices_size(initial_vertex_count),
      vertices_count(0), 
      indices_count(0), 
      faces_count(0), 
      instances(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, current_vertices_size/5),
      transforms(info.transforms),
      matrices(info.matrices),
      textures(info.textures)
    {
      decal_vertices = device->create(yavf::BufferCreateInfo::buffer(sizeof(render::vertex)*current_vertices_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      decal_indices  = device->create(yavf::BufferCreateInfo::buffer(sizeof(uint32_t)*(current_vertices_size*1.5f), VK_BUFFER_USAGE_INDEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      
      {
        auto pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
        auto layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
        yavf::DescriptorMaker dm(device);
        
        auto desc = dm.layout(layout).create(pool)[0];
        size_t index = desc->add({instances.vector().handle(), 0, instances.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        instances.vector().setDescriptor(desc, index);
      }
      
      ASSERT(sizeof(instance_data) == sizeof(geometry_optimizer::instance_data));
    }
    
    void decal_gbuffer::begin() {}
    void decal_gbuffer::proccess(context* ctx) {
      if (vertices_count == 0) return;
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), instances.vector().descriptorSet()->handle()}, 0);
      task->setVertexBuffer(decal_vertices, 0);
      task->setIndexBuffer(decal_indices);
      task->drawIndexed(indices_count, 1, 0, 0, 0);
    }
    
    void decal_gbuffer::clear() {
      const uint32_t old_size = current_vertices_size;
      if (vertices_count > current_vertices_size) {
        current_vertices_size = vertices_count;
      } else if (current_vertices_size - vertices_count < 10) {
        current_vertices_size *= 1.2f;
      }
      
      if (old_size != current_vertices_size) {
        decal_vertices->recreate(current_vertices_size*sizeof(render::vertex));
      }
      
      vertices_count = 0;
      indices_count = 0;
      faces_count = 0;
    }
    
#define DECAL_GBUFFER_PIPELINE_NAME "decal_gbuffer_pipeline"
#define DECAL_GBUFFER_PIPELINE_LAYOUT_NAME "decal_gbuffer_pipeline_layout"
    void decal_gbuffer::recreate_pipelines(const game::image_resources_t* resource) {
      images_set = resource->set;
      
      if (pipe.handle() != VK_NULL_HANDLE) {
        device->destroyPipeline(DECAL_GBUFFER_PIPELINE_NAME);
        device->destroyLayout(DECAL_GBUFFER_PIPELINE_LAYOUT_NAME);
      }
      
      auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      
      yavf::PipelineLayout pipeline_layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        
        pipeline_layout = plm.addDescriptorLayout(uniform_layout)
                             .addDescriptorLayout(resource->layout)
                             .addDescriptorLayout(storage_layout)
                             .create(DECAL_GBUFFER_PIPELINE_LAYOUT_NAME);
      }
      
      uint32_t constants[2] = {resource->images, resource->samplers};
      
      {
        yavf::raii::ShaderModule vertex  (device, Global::getGameDir() + "shaders/deferred.vert.spv"); // по идее мне не нужно писать особый шейдер для этого
        yavf::raii::ShaderModule fragment(device, Global::getGameDir() + "shaders/deferred.frag.spv");
        
        yavf::PipelineMaker pm(device);
        pm.clearBlending();
        
        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                   .addSpecializationEntry(0, 0, sizeof(uint32_t))
                   .addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t))
                   .addData(2*sizeof(uint32_t), constants)
                 .vertexBinding(0, sizeof(struct vertex)) // ?
                   .vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(struct vertex, pos))
                   .vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(struct vertex, color))
                   .vertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(struct vertex, tex_coord))
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_FALSE)
                 .depthBias(VK_TRUE, -10.0f, 0.0f, -1.75f)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create(DECAL_GBUFFER_PIPELINE_NAME, pipeline_layout, target->next_renderpass());
      }
    }
    
    // возможно нужно сделать просто большой статический буфер? можно в бегине увеличивать буфер (в clear() лучше)
    void decal_gbuffer::add(const decal_data &data) {
//       std::unique_lock<std::mutex> lock(mutex);
      const uint32_t vertex_offset = vertices_count.fetch_add(data.vertices_size);
      if ((vertex_offset+data.vertices_size) > current_vertices_size) {
        std::cout << "too many decals" << "\n";
        return;
      }
      
      const uint32_t index_offset = indices_count.fetch_add(data.vertices_size+1);
      const uint32_t face_index = faces_count.fetch_add(1);
      
      ASSERT(index_offset+data.vertices_size+1 <= current_vertices_size*1.5f);
      ASSERT(face_index < current_vertices_size/5);
      
      auto vertices = reinterpret_cast<render::vertex*>(decal_vertices->ptr());
      auto indices  = reinterpret_cast<uint32_t*>(decal_indices->ptr());
      for (uint32_t i = 0; i < data.vertices_size; ++i) {
        const uint32_t index = vertex_offset + i;
        const uint32_t index2 = index_offset + i;
        vertices[index] = data.vertices[i];
        //PRINT_VEC4("vertices[index].pos", vertices[index].pos.get_glm())
        vertices[index].color.arr[3] = glm::uintBitsToFloat(face_index);
        indices[index2] = index;
      }
      
      //PRINT("\n")
      
      indices[index_offset+data.vertices_size] = UINT32_MAX;
      
      const simd::vec4 pos = data.transform_index != UINT32_MAX ? transforms->at(data.transform_index).pos : simd::vec4(0.0f, 0.0f, 0.0f, 1.0f);
      const simd::vec4 scale = data.transform_index != UINT32_MAX ? transforms->at(data.transform_index).scale : simd::vec4(1.0f, 1.0f, 1.0f, 1.0f);
      const simd::mat4 matrix = data.matrix_index != UINT32_MAX ? matrices->at(data.matrix_index) : simd::mat4(1.0f);
      ASSERT(data.texture_index != UINT32_MAX);
      const auto texture = textures->at(data.texture_index);
      
      const simd::mat4 trans = simd::translate(simd::mat4(1.0f), pos);
      const simd::mat4 scale_mat = simd::scale(simd::mat4(1.0f), scale);
      const simd::mat4 final_matrix = scale_mat * trans * matrix; // МАТРИЦА ИЗ БУФЕРА СНАЧАЛО!
      
      instances[face_index] = {
        final_matrix,
        texture,
        {0}
      };
    }
    
    lights_optimizer::lights_optimizer(const create_info &info) : device(info.device), uniform(info.uniform), matrix(info.matrix), transforms(info.transforms), light_array(device), data(info.data), target(info.target) {
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout storage_image_layout = device->setLayout("storage_image_layout");
    //   yavf::DescriptorSetLayout matrixes = device->setLayout(MATRICES_BUFFER_LAYOUT_NAME);
      {
        yavf::DescriptorLayoutMaker dlm(device);
        
        if (layout == VK_NULL_HANDLE) {
          layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create(STORAGE_BUFFER_LAYOUT_NAME);
        }
    //     
    //     if (uniform_layout == VK_NULL_HANDLE) {
    //       uniform_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create(UNIFORM_BUFFER_LAYOUT_NAME);
    //     }
        
        if (storage_image_layout == VK_NULL_HANDLE) {
          storage_image_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT).create("storage_image_layout");
        }
      }
      
      yavf::PipelineLayout pipelineLayout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        
        pipelineLayout = plm.addDescriptorLayout(uniform_layout)
                            .addDescriptorLayout(target->layout())
    //                                  .addDescriptorLayout(minmax_layout)
                            .addDescriptorLayout(layout)
    //                         .addDescriptorLayout(matrixes)
                            .addDescriptorLayout(uniform_layout)
                            .addDescriptorLayout(storage_image_layout)
                            .create("image_processing_layout");
      }
      
      {
        yavf::ComputePipelineMaker cpm(device);
        yavf::raii::ShaderModule compute(device, Global::getGameDir() + "shaders/tiling.comp.spv");
        
        pipe = cpm.shader(compute).create("compute_pipeline", pipelineLayout);
      }
      
      yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
      {
//         light_array.construct(device);
        
        yavf::DescriptorMaker dm(device);
        
        yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
        const size_t i = d->add({light_array.vector().handle(), 0, light_array.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        light_array.vector().setDescriptor(d, i);
        
        glm::uvec4* count = light_array.structure_from_begin<glm::uvec4>();
        memset(count, 0, sizeof(glm::uvec4));
      }
    }
    
    lights_optimizer::~lights_optimizer() {
      device->destroy(pipe.layout());
      device->destroy(pipe);
    }
    
    void lights_optimizer::add(const light_data &info) {
      std::unique_lock<std::mutex> lock(mutex);
      
      glm::uvec4* count = light_array.structure_from_begin<glm::uvec4>();
      if (light_array.size() <= count->x + 1) {
        light_array.resize((count->x + 1) * 2);
        count = light_array.structure_from_begin<glm::uvec4>();
      }

      render::light_data* datas = light_array.data_from<glm::uvec4>();

      const Transform &trans = transforms->at(info.transform_index);
      const simd::vec4 vec = simd::vec4(info.light_data.pos.arr[0], info.light_data.pos.arr[1], info.light_data.pos.arr[2], 0.0f);
      basic_vec4 finalVec = vec + trans.pos;
      finalVec.arr[3] = info.light_data.pos.arr[3];
      const render::light_data data{
        finalVec,
        info.light_data.color
      };

      datas[count->x] = data;
      ++count->x;
    }
    
    void lights_optimizer::begin() {}
    void lights_optimizer::proccess(context* ctx) {
      yavf::ComputeTask* task = ctx->compute();
  
      if (data->light_output.info().initialLayout != VK_IMAGE_LAYOUT_GENERAL) task->setBarrier(&data->light_output, VK_IMAGE_LAYOUT_GENERAL);
      
      task->setPipeline(pipe);
      task->setDescriptor({
        uniform->descriptorSet()->handle(), 
        target->current_frame_data()->desc->handle(), 
        light_array.vector().descriptorSet()->handle(), 
        matrix->descriptorSet()->handle(), 
        data->light_output.view()->descriptorSet()->handle()
      }, 0);
      
      const uint32_t xCount = glm::ceil(static_cast<float>(data->light_output.info().extent.width)  / static_cast<float>(workgroup_size));
      const uint32_t yCount = glm::ceil(static_cast<float>(data->light_output.info().extent.height) / static_cast<float>(workgroup_size));
      task->dispatch(xCount, yCount, 1);
      
      task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                      VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
    }
    
    void lights_optimizer::clear() {
      glm::uvec4* count = light_array.structure_from_begin<glm::uvec4>();
      memset(count, 0, sizeof(glm::uvec4));
    }
    
    tone_mapping::tone_mapping(const create_info &info) : device(info.device), data(info.data) {
      yavf::DescriptorSetLayout storage_image_layout = device->setLayout("storage_image_layout");
  
      yavf::PipelineLayout tone_mapping_layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        
        tone_mapping_layout = plm.addDescriptorLayout(storage_image_layout)
                                 .addDescriptorLayout(storage_image_layout)
                                 .create("tone_mapping_layout");
      }
      
      {
        yavf::ComputePipelineMaker cpm(device);
        yavf::raii::ShaderModule compute(device, (Global::getGameDir() + "shaders/toneMapping.comp.spv").c_str());
        pipe = cpm.shader(compute).create("tone_mapping_pipeline", tone_mapping_layout);
      }
    }
    
    tone_mapping::~tone_mapping() {
      device->destroy(pipe.layout());
      device->destroy(pipe);
    }
    
    void tone_mapping::begin() {}
    void tone_mapping::proccess(context* ctx) {
      yavf::ComputeTask* task = ctx->compute();
  
      if (data->tone_mapping_output.info().initialLayout != VK_IMAGE_LAYOUT_GENERAL) task->setBarrier(&data->tone_mapping_output, VK_IMAGE_LAYOUT_GENERAL);
      
      task->setPipeline(pipe);
      task->setDescriptor({data->light_output.view()->descriptorSet()->handle(), data->tone_mapping_output.view()->descriptorSet()->handle()}, 0);
      const uint32_t xCount = glm::ceil(static_cast<float>(data->tone_mapping_output.info().extent.width)  / static_cast<float>(workgroup_size));
      const uint32_t yCount = glm::ceil(static_cast<float>(data->tone_mapping_output.info().extent.height) / static_cast<float>(workgroup_size));
      task->dispatch(xCount, yCount, 1);
      
      task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
    }
    
    void tone_mapping::clear() {}
    
    copy::copy(const create_info &info) : screenshot(false), data(info.data), target(info.target), w(info.w), early_screenshot_img(info.early_screenshot_img) {}
    void copy::begin() {
      if (screenshot) {
        const auto &rect = w->surface.extent;
        early_screenshot_img->recreate({rect.width, rect.height, 1});
      }
    }
    
    void copy::proccess(context* ctx) {
      static const VkImageSubresourceRange range{
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, 1, 0, 1
      };
      
      const auto &rect = w->surface.extent;
      
      const VkImageBlit blit{
        {
          VK_IMAGE_ASPECT_COLOR_BIT,
          0, 0, 1
        },
        {
          {0, 0, 0}, 
          {static_cast<int32_t>(rect.width), static_cast<int32_t>(rect.height), 1}
        },
        {
          VK_IMAGE_ASPECT_COLOR_BIT,
          0, 0, 1
        },
        {
          {0, 0, 0}, 
          {static_cast<int32_t>(rect.width), static_cast<int32_t>(rect.height), 1}
        }
      };
      
      yavf::GraphicTask* task = ctx->graphics();
      
      task->setBarrier(w->image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range, w->present_family, VK_QUEUE_FAMILY_IGNORED);
      task->setBarrier(&data->tone_mapping_output, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
      task->copyBlit(&data->tone_mapping_output, w->image(), blit);
      if (screenshot && early_screenshot_img != nullptr) {
        task->setBarrier(early_screenshot_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        task->copyBlit(&data->tone_mapping_output, early_screenshot_img, blit);
        screenshot = false;
      }
      task->setBarrier(&data->tone_mapping_output, VK_IMAGE_LAYOUT_GENERAL);
      task->setBarrier(w->image(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, range, VK_QUEUE_FAMILY_IGNORED, w->present_family);
      
      task->setBarrier(target->current_frame_data()->depth, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      task->setBarrier(w->depth(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      task->copy(target->current_frame_data()->depth, w->depth(), VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
      task->setBarrier(target->current_frame_data()->depth, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      task->setBarrier(w->depth(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    
    void copy::clear() {}
    void copy::do_screenshot() {
      screenshot = true;
    }
    
    struct gui_vertex {
      glm::vec2 pos;
      glm::vec2 uv;
      uint32_t color;
    };

    #define MAX_VERTEX_BUFFER (2 * 1024 * 1024)
    #define MAX_INDEX_BUFFER (512 * 1024)
    
    gui::gui(const create_info &info) : 
      device(info.device), 
      w(info.w), 
      vertex_gui(device, yavf::BufferCreateInfo::buffer(MAX_VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY), 
      index_gui(device, yavf::BufferCreateInfo::buffer(MAX_INDEX_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY), 
      matrix(device, yavf::BufferCreateInfo::buffer(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY),
      image_set(nullptr)
    {
      yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
      yavf::DescriptorSetLayout uniform_layout       = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      {
        yavf::DescriptorLayoutMaker dlm(device);
        
        if (sampled_image_layout == VK_NULL_HANDLE) {
          sampled_image_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).create(SAMPLED_IMAGE_LAYOUT_NAME);
        }
      }
      
      {
        yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
    
        yavf::DescriptorMaker dm(device);
        auto d = dm.layout(uniform_layout).create(pool)[0];
        const size_t index = d->add({&matrix, 0, matrix.info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
        matrix.setDescriptor(d, index);
      }
    }
    
    void gui::begin() {
      auto data = Global::get<interface::context>();
  
      {
        void* vertices = vertex_gui.ptr();
        void* elements = index_gui.ptr();
        
        /* fill convert configuration */
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
          {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(gui_vertex, pos)},
          {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(gui_vertex, uv)},
          {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(gui_vertex, color)},
          {NK_VERTEX_LAYOUT_END}
        };
        
        // нужен ли мне изменяющийся антиаляисинг?
        const nk_convert_config config{
          1.0f,
          NK_ANTI_ALIASING_ON,
          NK_ANTI_ALIASING_ON,
          22,
          22,
          22,
          data->null, // нулл текстура
          vertex_layout,
          sizeof(gui_vertex), // размер вершины
          alignof(gui_vertex) // алигн вершины
        };
        
        nk_buffer vbuf, ebuf;
        // мы создаем местные аналоги vkBuffer, то есть мета объект для памяти
        // а затем конвертим вершины в удобный нам формат
        nk_buffer_init_fixed(&vbuf, vertices, size_t(MAX_VERTEX_BUFFER));
        nk_buffer_init_fixed(&ebuf, elements, size_t(MAX_INDEX_BUFFER));
        nk_convert(&data->ctx, &data->cmds, &vbuf, &ebuf, &config);
      }
      
      glm::mat4* mat = reinterpret_cast<glm::mat4*>(matrix.ptr());
      *mat = glm::mat4(
        2.0f / w->viewport().width,  0.0f,  0.0f,  0.0f,
        0.0f,  2.0f / w->viewport().height,  0.0f,  0.0f,
        0.0f,  0.0f, -1.0f,  0.0f,
        -1.0f, -1.0f,  0.0f,  1.0f
      );
    }
    
    void gui::proccess(context* ctx) {
      yavf::GraphicTask* task = ctx->graphics();
      auto data = Global::get<interface::context>();
      
      task->setPipeline(pipe);

      task->setVertexBuffer(&vertex_gui, 0);
      task->setIndexBuffer(&index_gui, VK_INDEX_TYPE_UINT16);
      
      yavf::ImageView* tex = data->view;
      const std::vector<VkDescriptorSet> sets = {matrix.descriptorSet()->handle(), tex->descriptorSet()->handle(), image_set->handle()};
      task->setDescriptor(sets, 0);
      
      uint32_t index_offset = 0;
      const nk_draw_command *cmd = nullptr;
      nk_draw_foreach(cmd, &data->ctx, &data->cmds) {
        if (cmd->elem_count == 0) continue;
        
        const render::image i = image_nk_handle(cmd->texture);
        //ASSERT(i.index == UINT32_MAX && i.layer == UINT32_MAX);
        auto data = glm::uvec4(i.container, 0, 0, 0);
    //     PRINT_VEC2("image id", data)
        task->setConsts(0, sizeof(data), &data);

        const glm::vec2 fb_scale = Global::get<input::data>()->fb_scale;
        const VkRect2D scissor{
          {
            static_cast<int32_t>(std::max(cmd->clip_rect.x * fb_scale.x, 0.0f)),
            static_cast<int32_t>(std::max(cmd->clip_rect.y * fb_scale.y, 0.0f)),
          },
          {
            static_cast<uint32_t>(cmd->clip_rect.w * fb_scale.x),
            static_cast<uint32_t>(cmd->clip_rect.h * fb_scale.y),
          }
        };
        
        task->setScissor(scissor);
        task->drawIndexed(cmd->elem_count, 1, index_offset, 0, 0);
        index_offset += cmd->elem_count;
      }
    }
    
    void gui::clear() {
      auto data = Global::get<interface::context>();
      nk_clear(&data->ctx);
    }
    
    void gui::recreate_pipelines(const game::image_resources_t* resource) {
      yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
      yavf::DescriptorSetLayout uniform_layout       = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      
      yavf::PipelineLayout gui_layout = device->layout("gui_layout");
      {
        if (gui_layout != VK_NULL_HANDLE) device->destroy(gui_layout);
        
        yavf::PipelineLayoutMaker plm(device);
        gui_layout = plm.addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(sampled_image_layout)
                        .addDescriptorLayout(resource->layout)
                        .addPushConstRange(0, sizeof(glm::vec2) + sizeof(glm::vec2))
                        .create("gui_layout");
      }
      
      uint32_t constants[2] = {resource->images, resource->samplers};
      image_set = resource->set;
      
      {
        if (pipe.handle() != VK_NULL_HANDLE) device->destroy(pipe);
        yavf::PipelineMaker pm(device);
        pm.clearBlending();
        
        yavf::raii::ShaderModule vertex(device, (Global::getGameDir() + "shaders/gui.vert.spv").c_str());
        yavf::raii::ShaderModule fragment(device, (Global::getGameDir() + "shaders/gui.frag.spv").c_str());
        
        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                   .addSpecializationEntry(0, 0, sizeof(uint32_t))
                   .addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t))
                   .addData(2*sizeof(uint32_t), constants)
                 .vertexBinding(0, sizeof(gui_vertex))
                   .vertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(gui_vertex, pos))
                   .vertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(gui_vertex, uv))
                   .vertexAttribute(2, 0, VK_FORMAT_R8G8B8A8_UINT, offsetof(gui_vertex, color))
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                 .depthTest(VK_FALSE)
                 .depthWrite(VK_FALSE)
                 .clearBlending()
                 .colorBlendBegin()
                   .srcColor(VK_BLEND_FACTOR_SRC_ALPHA)
                   .dstColor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
                   .srcAlpha(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
                   .dstAlpha(VK_BLEND_FACTOR_SRC_ALPHA)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .create("gui_pipeline", gui_layout, w->render_pass);
      }
    }
    
    task_start::task_start(yavf::Device* device) : device(device) {}
    void task_start::begin() {}
    void task_start::proccess(context* ctx) {
      yavf::TaskInterface* task = ctx->interface();
      const VkSubmitInfo &info = task->getSubmitInfo();
      const uint32_t &family = task->getFamily();

      wait_fence = device->submit(family, 1, &info);
    }
    
    void task_start::clear() {}
    void task_start::wait() {
      const VkResult res = vkWaitForFences(device->handle(), 1, &wait_fence.fence, VK_TRUE, 1000000000);
      if (res != VK_SUCCESS) {
        throw std::runtime_error("hlqhvlgvuvfgowqvuqopquwvuonvev");
      }
    }
    
    window_present::window_present(const create_info &info) : w(info.w) {}
    void window_present::begin() {}
    void window_present::proccess(context* ctx) { w->present(); (void)ctx; }
    void window_present::clear() {}
  }
}
