#include "GPUPhysics.h"
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <functional>

#include "Globals.h"
#include "Utility.h"

//#include "GPUOctreeBroadphase.h"
//#include "CPUOctreeBroadphase.h"

#include <iostream>
#include <chrono>
// #include <bitset>

GPUPhysics::GPUPhysics(yavf::Device* device, yavf::ComputeTask* task, const GPUPhysicsCreateInfo &info) {
  this->defaultStaticMatrixIndex = info.buffers->defaultStaticMatrixIndex;
  this->defaultDynamicMatrixIndex = info.buffers->defaultDynamicMatrixIndex;

  setBuffers(*info.buffers);

  construct(device, task, info.b, info.n, info.s, info.sorter);
}

GPUPhysics::~GPUPhysics() {
  //device->deallocate(task);
}

void computeAirVelocity(const glm::vec4 &oldVelocity,
                        const uint32_t &dt,
                        const glm::vec4 &additionalForce,
                        const glm::vec4 &additionalData,
                        glm::vec4 &velocity,
                        float &velocityScalar) {
  float dt1 = MCS_TO_SEC(dt);

  glm::vec4 vn = velocityScalar == 0.0f ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : oldVelocity / velocityScalar;

  //glm::vec4 a = -vn*additionalData.z + additionalForce + globalData.gravity;
  glm::vec4 a = -vn*additionalData.z + additionalForce + glm::vec4(0.0f, 9.8f, 0.0f, 0.0f);

  velocity = oldVelocity + a * dt1;
  velocityScalar = length(velocity);
}

void GPUPhysics::update(const uint64_t &time) {
  // первый шаг здесь это обновить все PhysicComponent
  // затем мы передаем слово броадфазе, нарров фазе и солверу
  // и на этом заканчиваем (или нет?)
  
  // ТУПАЯ ОШИБКА СВЯЗАНА БЫЛА С РАСПОЛОЖЕНИЕМ ДАННЫХ В СТРУКТУРАХ
  // ОТПРАВЛЯЕМЫХ НА ГПУ, КОНКРЕТНО В ЭТОМ СЛУЧАЕ Я ОТПРАВЛЯЛ ВРЕМЯ ВМЕСТО КОЛИЧЕСТВА ОБЪЕКТОВ
  
  // ТРИЖДЫ ПРОВЕРИТЬ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
  
  Gravity* data = uniform.data();
  data->objCount = physicsDatas.size();
  data->time = time;
  data->gravity = gravity;// glm::vec4(0.0f, 9.8f, 0.0f, 0.0f);
  data->gravityNormal = gravityNorm;// glm::normalize(data->gravity);
  data->length2 = gravLength2;// glm::length2(data->gravity);
  data->length = gravLength;// glm::sqrt(data->length2);

  updateBuffers();
  broad->updateBuffers(objects.size(), physicsDatas.size(), rays.size(), frustums.size());
  narrow->updateBuffers(overlappingPairCache.size(), staticOverlappingPairCache.size());
  solver->updateBuffers(overlappingPairCache.size() + staticOverlappingPairCache.size(), rayPairCache.size());
  
  // не знаю почему я подумал что будет охуенной идеей сразу для гпу написать физику
  // я на пол пути исправления ошибок солвера
  // в данный момент у меня проблемы:
  // 1. не понятно что делать с -1 в calcOverlappingPairs (та -1 которая должна помочь привести от колическа к индексам)
  // 2. иногда я прохожу сквозь рандомных объектов
  // 3. кажется не пересекаюсь с объект 5010 всегда
  // 4. иногда в нодах количество объектов становится UINT32_MAX 
  
  // некоторые мысли по поводу островов:
  // идеальный вариант делать острова на основе положения в октодереве
  // взять минимальный индекс нода в качестве индекса острова + параллельно вычислять только на одной глубине
  // (то есть например все пары которые оказались на глубине 4 расспараллелить по нодам)
  // неплохо было бы разделить пары со статиками от пар только с динамиками
  // и сначало параллельно вычилить пары со статиками (синхронизации для них не требуется)
  // так будет гораздо лучше и быстрее чем сейчас
  
  // у всех пар посчитать дополнительные данные и засунуть их в буфер (OverlappingData)
  // на основе этих данных сделать еще один буфер только с парами у которых есть триггер (то есть для них вызывается игровая логика)
  // таких пар должно получиться значительно меньше, что облегчит мне задачу их всех просмотреть
  // всего данных OverlappingData должно получиться что то вроде количество_динамических_объектов * 5 (это скорее всего прямо впритык)
  
  // подумать на счет обновления октодерева, скорее всего то что я делаю сейчас не совсем оптимально
  // достаточно простой вариант добавления заключается в том, чтобы держать пару (nodeIndex, proxyIndex)
  // в качестве индексов объектов, и этот буфер сортировать по индексам нодов
  // легкое добавление объектов, но не знаю пока как эти объекты быстро удалять и передобавлять (или просто индекс нода менять)
  // в этом случае может спасти бинарный поиск, но это может оказаться относительно долгим решением
  // с другой стороны в этом случае контейнер индекс хранить не получится (точнее придется обходить все объекты нода и менять им индексы)
  // тут всплывает еще несколько подводных камней
  // по всей видимости тут только бинарный поиск, в принципе он O(logn) + количество объектов сриди которых будет искать мало
  // может оказаться что это более лучшее решение
  // для этого потребуется хорошая сортировка, возможно стоит еще поискать в нете что нибудь на эту тему (там как то мало всего на тему параллельных сортировок =( )

  if (transforms->at(physicsDatas[0].transformIndex).pos.x != transforms->at(physicsDatas[0].transformIndex).pos.x) throw std::runtime_error("NAN");

  task->begin();
  task->setPipeline(first);
  task->setDescriptor({objects.vector().descriptorSet()->handle(), //physicsDatas.vector().descriptor(), 
                       inputDesc->handle(), //inputs.vector().descriptor(), 
                       transformDesc->handle(), //transforms.vector().descriptor(), 
                       externalsDesc->handle(), //staticPhysDataDesc,
                       indices.vector().descriptorSet()->handle(), 
                       //globalVel.vector().descriptor(), 
                       uniform.buffer()->descriptorSet()->handle()}, 0);
  // здесь наверное будет только перевычисление PhysicData так как остальное непосредственно зависит от самой фазы
  task->dispatch(1, 1, 1);

  task->end();
  
  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();  
  std::cout << "Velocity time: " << ns << " mcs" << "\n";

//   PRINT_VEC3("velocity first obj", physicsDatas[0].velocity)
//   PRINT_VEC4("glob vel first obj", globalVel[0])
//   PRINT_VEC4("pos first obj", transforms->at(physicsDatas[0].transformIndex).pos)
  if (transforms->at(physicsDatas[0].transformIndex).pos.x != transforms->at(physicsDatas[0].transformIndex).pos.x) throw std::runtime_error("NAN");

  broad->update();

  broad->calculateOverlappingPairs();
  
  sorter->sort(&overlappingPairCache, 1);

  //narrow->calculateIslands();
  //narrow->calculateBatches();
  narrow->checkIdenticalPairs();
  
  sorter->sort(&overlappingPairCache);
  sorter->sort(&staticOverlappingPairCache);
  //sorter->barrier();
  narrow->postCalculation();

  solver->calculateData();

//   PRINT_VEC3("velocity first obj", physicsDatas[0].velocity)
//   PRINT_VEC4("glob vel first obj", globalVel[0])
//   PRINT_VEC4("pos first obj", transforms->at(physicsDatas[0].transformIndex).pos)
  if (transforms->at(physicsDatas[0].transformIndex).pos.x != transforms->at(physicsDatas[0].transformIndex).pos.x) throw std::runtime_error("NAN");

  //solver->solve();
  // не работатет из-за солвера!!!!
  solver->solve();

//   PRINT_VEC3("velocity first obj", physicsDatas[0].velocity)
//   PRINT_VEC4("glob vel first obj", globalVel[0])
//   PRINT_VEC4("pos first obj", transforms->at(physicsDatas[0].transformIndex).pos)
  if (transforms->at(physicsDatas[0].transformIndex).pos.x != transforms->at(physicsDatas[0].transformIndex).pos.x) throw std::runtime_error("NAN");

  //broad->printStats();

  broad->update();

  broad->calculateRayTests();
  broad->calculateFrustumTests();
  
  solver->calculateRayData();

  sorter->sort(&frustumTestsResult);
  sorter->sort(&overlappingData, &dataIndices, 0);
  sorter->sort(&raysData, &raysIndices, 1);

  overlappingDataSize = dataIndices.data()->count;
  rayTracingSize = raysIndices.data()->temporaryCount;
//   overlappingDataSize = 10;
//   rayTracingSize = 10;
  frustumTestSize = frustumTestsResult[0].firstIndex;

  //broad->postlude();

  memset(rays.data(), UINT32_MAX, sizeof(RayData));
  memset(frustums.data(), UINT32_MAX, sizeof(FrustumStruct));
  //memset(broad->getFrustumTestsResult()->data(), 0, sizeof(BroadphasePair));
  memset(frustumTestsResult.data(), 0, sizeof(BroadphasePair));
  
  //memset(solver->getRayIndices()->data(), 0, sizeof(DataIndixies));
  
  indices[0] = 0;

  end = std::chrono::steady_clock::now() - start;
  ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
}

void GPUPhysics::setBuffers(const PhysicsExternalBuffers &buffers) {
  this->defaultStaticMatrixIndex = buffers.defaultStaticMatrixIndex;
  this->defaultDynamicMatrixIndex = buffers.defaultDynamicMatrixIndex;

  this->inputs = buffers.inputs;
  this->transforms = buffers.transforms;
  this->matrices = buffers.matrices;
  this->rotationDatasCount = buffers.rotationDatasCount;
  this->rotationDatas = buffers.rotationDatas;
  this->externalDatas = buffers.externalDatas;

  yavf::Buffer* buffer;
  buffers.inputs->gpu_buffer(&buffer);
  inputDesc = buffer->descriptorSet();
  buffers.transforms->gpu_buffer(&buffer);
  transformDesc = buffer->descriptorSet();
  buffers.matrices->gpu_buffer(&buffer);
  matrixDesc = buffer->descriptorSet();
  buffers.rotationDatasCount->gpu_buffer(&buffer);
  rotationCountDesc = buffer->descriptorSet();
  buffers.rotationDatas->gpu_buffer(&buffer);
  rotationDesc = buffer->descriptorSet();
  buffers.externalDatas->gpu_buffer(&buffer);
  externalsDesc = buffer->descriptorSet();
}

void GPUPhysics::registerShape(const std::string &name, const uint32_t shapeType, const RegisterNewShapeInfo &info) {
  auto itr = shapes.find(name);
  if (itr != shapes.end()) throw std::runtime_error("Shape type " + name + " is already exist!");

  if (shapeType == SPHERE_TYPE) throw std::runtime_error("Dont need to register sphere");
  if (!(shapeType == BBOX_TYPE || shapeType == POLYGON_TYPE)) throw std::runtime_error("This type is not supported yet");

  if (info.faces.empty()) throw std::runtime_error("Wrong creation data");

  uint32_t additionalFaceOffset = 0;
  uint32_t controlCount = 0;
  if (shapeType == BBOX_TYPE) {
    if (info.faces.empty() || info.faces.size() > 1) throw std::runtime_error("Wrong box data");

    controlCount = 1; 
  } else {
    additionalFaceOffset = 1;
    controlCount = info.points.size() + info.faces.size() + 1;

    if (info.points.empty() || info.faces.empty() || controlCount < 6) throw std::runtime_error("Wrong polygon data");

    // for (uint32_t i = 0; i < info.faces.size(); ++i) {
    //   if (info.faces[i].w == 0.0f) throw std::runtime_error("Vertex index must be given for each face. EXPERIMENTAL ERROR");
    // }
  }

  ShapeInfo s{
    shapeType,
    UINT32_MAX,
    0,
    0
  };
    
//     if (freeVert != UINT32_MAX) {
      
  uint32_t oldIndex = UINT32_MAX;
  s.offset = freeVert;
  while (s.offset != UINT32_MAX && glm::floatBitsToUint(verts[s.offset].y) != controlCount) {
    oldIndex = s.offset;
    s.offset = glm::floatBitsToUint(verts[s.offset].x);
  }
  
  if (s.offset != UINT32_MAX) {
    if (oldIndex == UINT32_MAX) {
      freeVert = glm::floatBitsToUint(verts[freeVert].x);
    } else {
      verts[oldIndex].x = verts[s.offset].x;
    }
    
    //verts[s.offset] = info.faces[0];
  } else {
    // TODO: может быть ненужно ресайзить каждый раз?
    s.offset = verts.size();
    verts.resize(verts.size() + controlCount);
  }

  s.pointCount = info.points.size();
  s.faceCount = info.faces.size();
  
  glm::vec4 localCenter = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  for (uint32_t i = 0; i < info.points.size(); ++i) {
    verts[s.offset + i] = info.points[i];
    localCenter += info.points[i];
  }
  localCenter.x /= float(info.points.size());
  localCenter.y /= float(info.points.size());
  localCenter.z /= float(info.points.size());
  localCenter.w = 1.0f;

  verts[s.offset + s.pointCount] = localCenter;
  
  for (uint32_t i = 0; i < info.faces.size(); ++i) {
    verts[s.offset + s.pointCount + additionalFaceOffset + i] = info.faces[i];
  }

  shapes[name] = s;
}

// uint32_t GPUPhysics::registerMatrix(const glm::mat4 &matrix) {
//   uint32_t index = coordinateSystems.size();
//   coordinateSystems.vector().push_back(matrix);
//   coordinateSystems.update();

//   return index;
// }

void GPUPhysics::add(const PhysicsObjectCreateInfo &info, PhysicsIndexContainer* container) {
  if (defaultStaticMatrixIndex == UINT32_MAX || defaultDynamicMatrixIndex == UINT32_MAX) throw std::runtime_error("No default matrix");

  container->objectDataIndex = UINT32_MAX;
  container->physicDataIndex = UINT32_MAX;
  container->transformIndex = UINT32_MAX;
  container->inputIndex = UINT32_MAX;
  container->rotationIndex = UINT32_MAX;

  if (info.transformIndex == UINT32_MAX && info.type.getObjType() != POLYGON_TYPE) {
    // std::cout << "Obj type " << info.type.getObjType() << " must be " << POLYGON_TYPE << '\n';
    // std::bitset<32> y(info.type.type);
    // std::bitset<32> y1(info.type.type >> 1);
    // std::bitset<32> y2((info.type.type >> 1) & 0x7);
    // std::cout << "Type bits " << y << '\n';
    // std::cout << "Type bits " << y1 << '\n';
    // std::cout << "Type bits " << y2 << '\n';

    throw std::runtime_error("Cannot create box or sphere without transform");
  }
  if (info.inputIndex == UINT32_MAX && info.type.isDynamic()) throw std::runtime_error("Cannot create dynamic object without input");
  if (info.externalDataIndex == UINT32_MAX && info.type.isDynamic()) throw std::runtime_error("Cannot create dynamic object without external data");

  container->internalIndex = components.size();
  components.push_back(container);
  
  //std::cout << "Obj type " << info.type.type << "\n";
  
  //BroadphaseProxy* proxy = broad->createProxy();
  
//   PhysicData* datas = (PhysicData*)physicsDatas2->ptr();

  uint32_t staticPhysIndex = 0;
  if (freeObj != UINT32_MAX) {
    container->objectDataIndex = freeObj;
    staticPhysIndex = freeObj;
    freeObj = objects[freeObj].objectId;
    
    //memset(&objects[comp->objectDataIndex], 0, sizeof(Object));
    
    //objects[comp->objectDataIndex].entityId = 0; // это нужно будет взять из компонента
    //objects[comp->objectDataIndex].objectId = comp->objectDataIndex;
    
    // и прочее (прокси создать, физикдату ...)
  } else {
    container->objectDataIndex = objects.size();
    objects.vector().emplace_back();
    objects.update();

    staticPhysIndex = staticPhysicDatas.size();
    staticPhysicDatas.vector().emplace_back();
    staticPhysicDatas.update();

    //updateStaticPhysDataDesc();
    
    //memset(&objects[comp->objectDataIndex], 0, sizeof(Object));
    
    // и тоже самое что и сверху
  }

  // if (freeStaticPhys != UINT32_MAX) {

  // }
  
  if (info.type.isDynamic()) {
    if (freePhys != UINT32_MAX) {
      container->physicDataIndex = freePhys;
      freePhys = glm::floatBitsToUint(physicsDatas[freePhys].velocity.x);
    } else {
      container->physicDataIndex = physicsDatas.size();
      physicsDatas.vector().emplace_back();
      physicsDatas.update();
    }
  }
  
  // if (!info.type.isDynamic() && info.type.getObjType() == POLYGON_TYPE) container->transformIndex = UINT32_MAX;
  // else {
  //   if (freeTrans != UINT32_MAX) {
  //     container->transformIndex = freeTrans;
  //     freeTrans = glm::floatBitsToUint(transforms[freeTrans].pos.x);
  //   } else {
  //     container->transformIndex = transforms.size();
  //     transforms.vector().emplace_back();
  //     transforms.update();
  //   }
  // }

  container->transformIndex = info.transformIndex;
  container->inputIndex = info.inputIndex;
  
  // if (info.type.isDynamic()) {
  //   if (freeInput != UINT32_MAX) {
  //     container->inputIndex = freeInput;
  //     freeInput = glm::floatBitsToUint(inputs[freeInput].right.x);
  //   } else {
  //     container->inputIndex = inputs.size();
  //     inputs.vector().emplace_back();
  //     inputs.update();
  //   }
  // } else container->inputIndex = UINT32_MAX;
  
//   PRINT_VAR("input index", container->inputIndex)
//   PRINT_VAR("phys data index", container->physicDataIndex)
  
//   if (container->inputIndex > 10) {
//     std::cout << "AAAAAAAAAAAAAAAAAAAAAAAA input index" << "\n";
//     exit(1);
//   }

  container->rotationIndex = info.rotationIndex;
  //uint32_t rotationIndex = UINT32_MAX;
  // if (info.rotation) {
  //   if (freeRotation != UINT32_MAX) {
  //     container->rotationIndex = freeRotation;
  //     freeRotation = glm::floatBitsToUint(rotationDatas[freeRotation].anchorDir.x);
  //   } else {
  //     container->rotationIndex = rotationDatas.size();
  //     rotationDatas.vector().emplace_back();
  //     rotationDatas.update();
  //   }

  //   const RotationData rd{
  //     glm::vec3(0.0f, 0.0f, 0.0f),
  //     0.0f,

  //     0.0f,
  //     0.0f,
  //     0.0f,
  //     0,

  //     glm::mat4(1.0f)
  //   };

  //   rotationDatas[container->rotationIndex] = rd;
  // } 
  
  const uint32_t proxyIndex = broad->createProxy({}, container->objectDataIndex, info.type, info.collisionGroup, info.collisionFilter);
  
  const Object obj{
    //info.entityIndex,
    //container->objectDataIndex,
    container->internalIndex,
    proxyIndex,
    //container->physicDataIndex,
    staticPhysIndex,
    container->transformIndex,
    
    0,
    0,
    0,
    info.type,
    
    //container->transformIndex,
    info.matrixIndex == defaultDynamicMatrixIndex || info.matrixIndex == defaultStaticMatrixIndex || info.matrixIndex == UINT32_MAX ? 
                          (info.type.isDynamic() ? defaultDynamicMatrixIndex : defaultStaticMatrixIndex) : info.matrixIndex,
    //info.matrixIndex < 2 || info.matrixIndex == UINT32_MAX ? (info.type.isDynamic() ? 1 : 0) : info.matrixIndex, // по идее это как то так должно быть
    UINT32_MAX,
    container->rotationIndex,
    0
  };
  
  objects[container->objectDataIndex] = obj;

  // const StaticPhysicData spd{
  //   info.groundFricion,
  //   info.overbounce,
  //   container->objectDataIndex,
  //   container->physicDataIndex
  // };

  const Constants c{
    info.groundFricion,
    info.overbounce,
    info.stairHeight,
    container->physicDataIndex
  };

  staticPhysicDatas[staticPhysIndex] = c;

  //std::cout << "Proxy index " << proxyIndex << '\n';
  
//   PhysicData data{
//     glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//     glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//     glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//     
//     0.0f, // ?
//     0.0f, // ?
//     0.0f, // ?
//     0.0f,
//     
//     container->objectDataIndex,
//     container->inputIndex,
//     0xFFFFFFFF,
//     0xFFFFFFFF,
//     
//     info.overbounce,
//     info.stairHeight,
//     container->transformIndex,
//     0.0f
//   };
  
  if (container->physicDataIndex != UINT32_MAX) {
    // const PhysicData data{
    //   glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
    //   glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
    //   glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      
    //   0.0f, // ?
    //   info.acceleration,
    //   info.stairHeight,
    //   0.0f,
      
    //   container->objectDataIndex,
    //   //staticPhysIndex,
    //   container->inputIndex,
    //   0xFFFFFFFF,
    //   0xFFFFFFFF,
      
    //   // info.overbounce,
    //   // info.stairHeight,
    //   container->transformIndex,
    //   staticPhysIndex,
    //   0,
    //   0
    // };

    const PhysData2 data{
      glm::vec3(0.0f, 0.0f, 0.0f),
      0.0f,
      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),

      container->objectDataIndex,
      container->inputIndex,
      0xFFFFFFFF,
      0xFFFFFFFF,

      info.externalDataIndex,
      container->transformIndex,
      staticPhysIndex,
      0
    };
    
    //PRINT_VAR("input index in data", data.inputIndex)
    
    physicsDatas[container->physicDataIndex] = data;
  }
  
  //datas[container->physicDataIndex] = data;
  
  //PRINT_VAR("input index in data2", physicsDatas[container->physicDataIndex].inputIndex)
  
  // if (container->transformIndex != UINT32_MAX) {
  //   const Transform trans{
  //     info.pos, //glm::vec4(0.0f),
  //     glm::mat4(0.0f)
  //   };
    
  //   transforms[container->transformIndex] = trans;
  // }
  
  //uint32_t vertOffset = 0xFFFFFFFF, vertSize = 0, faceSize = 0;
  
//   // типов у меня всего три: сфера (нужен радиус), бокс (нужен vec4 для размеров), мэш (?) (нужны вершины + нормали)
//   // следовательно для сферы ничего не нужно (радиус = 1 флоат)
//   // как быть?
//   // для бокса достаточно 1 верта
//   // так же проблема правильно обработать мэш (там минимум 3 + 3 + 1 = 7 vec4)
//   // точнее даже проблема придумать какой-то эффектиный способ удалять/вставлять
//   if (info.type.getObjType() == SPHERE_TYPE) {
//     transforms[container->transformIndex].pos.w = info.radius;
//   }
  
//   if (info.type.getObjType() == BBOX_TYPE) {
//     if (info.faces.empty() || info.faces.size() > 1) {
//       std::cout << "Wrong box data" << "\n";
//       exit(1);
//     }
    
// //     if (freeVert != UINT32_MAX) {
      
//       uint32_t oldIndex = UINT32_MAX;
//       vertOffset = freeVert;
//       while (vertOffset != UINT32_MAX && glm::floatBitsToUint(verts[vertOffset].y) != 1) {
//         oldIndex = vertOffset;
//         vertOffset = glm::floatBitsToUint(verts[vertOffset].x);
//       }
      
//       if (vertOffset != UINT32_MAX) {
//         if (oldIndex == UINT32_MAX) {
//           freeVert = glm::floatBitsToUint(verts[freeVert].x);
//         } else {
//           verts[oldIndex].x = verts[vertOffset].x;
//         }
        
//         verts[vertOffset] = info.faces[0];
//       } else {
//         vertOffset = verts.size();
//         verts.vector().push_back(info.faces[0]);
//         verts.update();
//       }
      
//       faceSize = 1;
// //     }
//   }
  
//   if (info.type.getObjType() == POLYGON_TYPE) {
//     const uint32_t count = info.points.size() + info.faces.size();
    
//     if (info.points.empty() || info.faces.empty() || count < 4) {
//       std::cout << "Wrong polygon data" << "\n";
//       exit(1);
//     }
    
//     uint32_t oldIndex = UINT32_MAX;
//     vertOffset = freeVert;
//     while (vertOffset != UINT32_MAX && glm::floatBitsToUint(verts[vertOffset].y) != count) {
//       oldIndex = vertOffset;
//       vertOffset = glm::floatBitsToUint(verts[vertOffset].x);
//     }
    
//     if (vertOffset != UINT32_MAX) {
//       if (oldIndex == UINT32_MAX) {
//         freeVert = glm::floatBitsToUint(verts[freeVert].x);
//       } else {
//         verts[oldIndex].x = verts[vertOffset].x;
//       }
      
//       vertSize = info.points.size();
//       faceSize = info.faces.size();
      
//       for (uint32_t i = 0; i < info.points.size(); ++i) {
//         verts[vertOffset + i] = info.points[i];
//       }
      
//       for (uint32_t i = 0; i < info.faces.size(); ++i) {
//         verts[vertOffset + vertSize + i] = info.faces[i];
//       }
//     } else {
//       vertOffset = verts.size();
//       vertSize = info.points.size();
//       faceSize = info.faces.size();
      
//       verts.resize(verts.size() + count);
      
//       for (uint32_t i = 0; i < info.points.size(); ++i) {
//         verts[vertOffset + i] = info.points[i];
//       }
      
//       for (uint32_t i = 0; i < info.faces.size(); ++i) {
//         verts[vertOffset + vertSize + i] = info.faces[i];
//       }
//     }
//   }
  
  // objects[container->objectDataIndex].vertexOffset = vertOffset;
  // objects[container->objectDataIndex].vertexCount = vertSize;
  // objects[container->objectDataIndex].faceCount = faceSize;

  auto itr = shapes.find(info.shapeName);
  if (itr == shapes.end()) throw std::runtime_error("Shape " + info.shapeName + " was not registered");
  if (itr->second.shapeType != info.type.getObjType()) throw std::runtime_error("Wrong shape for object");

  objects[container->objectDataIndex].vertexOffset = itr->second.offset;
  objects[container->objectDataIndex].vertexCount = itr->second.pointCount;
  objects[container->objectDataIndex].faceCount = itr->second.faceCount;
  
  //if (container->objectDataIndex > 5111) throw std::runtime_error("wtf");

  // std::cout << "Obj index   " << container->objectDataIndex << '\n';
  // std::cout << "Proxy index " << objects[container->objectDataIndex].proxyIndex << '\n';

  static uint32_t firstObjCounter = 0;

  if (!info.type.isDynamic()) {
    const uint32_t id = indices[0];
    ++indices[0];

    if (id+1 >= indices.size()) indices.resize(indices.size()*1.5f);

    indices[id+1] = container->objectDataIndex;
    if (container->objectDataIndex == 0) ++firstObjCounter;
    assert(firstObjCounter < 2);
  }

  firstObjCounter = 0;
}

void GPUPhysics::remove(PhysicsIndexContainer* comp) {
  components.back()->internalIndex = comp->internalIndex;
  std::swap(components[comp->internalIndex], components.back());
  components.pop_back();
  
  // const uint32_t offset = objects[comp->objectDataIndex].vertexOffset;
  // const uint32_t count = objects[comp->objectDataIndex].vertexCount + objects[comp->objectDataIndex].faceCount;
  // if (offset != UINT32_MAX) {
  //   verts[offset].x = glm::uintBitsToFloat(freeVert);
  //   verts[offset].y = glm::uintBitsToFloat(count);
  //   freeVert = offset;
  // }
  
  //broad->destroyProxy(objects[comp->objectDataIndex].proxyIndex);
  
  //memset(&objects[comp->objectDataIndex], 0, sizeof(Object));
  broad->destroyProxy(objects[comp->objectDataIndex].proxyIndex);

  objects[comp->objectDataIndex].objectId = freeObj;
  freeObj = comp->objectDataIndex;
  
  if (comp->physicDataIndex != UINT32_MAX) {
    physicsDatas[comp->physicDataIndex].velocity.x = glm::uintBitsToFloat(freePhys);
    physicsDatas[comp->physicDataIndex].objectIndex = 0xFFFFFFFF;
    freePhys = comp->physicDataIndex;
  }
  
  // if (comp->transformIndex != UINT32_MAX) {
  //   transforms[comp->transformIndex].pos.x = glm::uintBitsToFloat(freeTrans);
  //   freeTrans = comp->transformIndex;
  // }
  
  // if (comp->inputIndex != UINT32_MAX) {
  //   inputs[comp->inputIndex].right.x = glm::uintBitsToFloat(freeInput);
  //   freeInput = comp->inputIndex;
  // }

  // if (comp->rotationIndex != UINT32_MAX) {
  //   rotationDatas[comp->rotationIndex].anchorDir.x = glm::uintBitsToFloat(freeRotation);
  //   freeRotation = comp->rotationIndex;
  // }
}

uint32_t GPUPhysics::add(const RayData &ray) {
  uint32_t index = rays.vector().size();
  // нормализация??
  rays.vector().push_back(ray);
  rays.update();

  return index;
}

uint32_t GPUPhysics::add(const glm::mat4 &frustum, const glm::vec4 &pos) {
  uint32_t index = frustums.vector().size();
  // здесь он будет все пересчитывать
  frustums.vector().emplace_back(frustum);
  frustums.update();

  frustumPoses.vector().push_back(pos);
  frustumPoses.update();

  return index;
}

Object & GPUPhysics::getObjectData(const uint32_t &index) {
  return objects[index];
}

const Object & GPUPhysics::getObjectData(const uint32_t &index) const {
  return objects[index];
}

PhysData2 & GPUPhysics::getPhysicData(const uint32_t &index) {
  return physicsDatas[index];
}

const PhysData2 & GPUPhysics::getPhysicData(const uint32_t &index) const {
  return physicsDatas[index];
}

void* GPUPhysics::getUserData(const uint32_t &objIndex) const {
  throw std::runtime_error("not yet");
}

// Transform & GPUPhysics::getTransform(const uint32_t &index) {
//   return transforms[index];
// }

// const Transform & GPUPhysics::getTransform(const uint32_t &index) const {
//   return transforms[index];
// }

// RotationData & GPUPhysics::getRotationData(const uint32_t &index) {
//   return rotationDatas[index];
// }

// const RotationData & GPUPhysics::getRotationData(const uint32_t &index) const {
//   return rotationDatas[index];
// }

void GPUPhysics::setGravity(const glm::vec4 &g) {
  gravity = g;
  gravityNorm = glm::normalize(g);
  gravLength2 = glm::length2(g);
  gravLength = glm::sqrt(gravLength2);
  orientation = glm::orientation(glm::vec3(-gravityNorm), glm::vec3(0.0f, 1.0f, 0.0f));
  
  //coordinateSystems[1] = orientation;
  matrices->at(defaultDynamicMatrixIndex) = orientation;
}

// Object GPUPhysics::getObjectData(const uint32_t &index) const {
//   return objects[index];
// }
// 
// PhysicData GPUPhysics::getPhysicData(const uint32_t &index) const {
//   return physicsDatas[index];
// }
// 
// Transform GPUPhysics::getTransform(const uint32_t &index) const {
//   return transforms[index];
// }

void GPUPhysics::updateMaxSpeed(const uint32_t &physicDataIndex, const float &maxSpeed) {
  //physicsDatas[physicDataIndex].maxSpeed = maxSpeed;
  throw std::runtime_error("not implemented yet");
}

// void GPUPhysics::setInput(const uint32_t &index, const InputData &input) {
//   inputs[index] = input;
// }

uint32_t GPUPhysics::setShapePointsAndFaces(const uint32_t &objectDataIndex, const std::vector<glm::vec4> &points, const std::vector<glm::vec4> &faces) {
  throw std::runtime_error("not implemented yet");
}

ArrayInterface<OverlappingData>* GPUPhysics::getOverlappingPairsData() {
  //return solver->getOverlappingData();
  return &overlappingData;
}

const ArrayInterface<OverlappingData>* GPUPhysics::getOverlappingPairsData() const {
  return &overlappingData;
}

ArrayInterface<OverlappingData>* GPUPhysics::getRayTracingData() {
  return &raysData;
  //return solver->getRayIntersectData();
}

const ArrayInterface<OverlappingData>* GPUPhysics::getRayTracingData() const {
  return &raysData;
}

ArrayInterface<BroadphasePair>* GPUPhysics::getFrustumPairs()  {
  return &frustumTestsResult;
  //return broad->getFrustumTestsResult();
}

const ArrayInterface<BroadphasePair>* GPUPhysics::getFrustumPairs() const  {
  return &frustumTestsResult;
}

void GPUPhysics::printStats() {
  // мне тут пригодится выводить сколько всего объектов, сколько статиков (динамиков)
  // сколько занимает памяти разные вещи

  const size_t memoryUsed = verts.vector().size()*sizeof(verts[0]) + 
                            objects.vector().size()*sizeof(objects[0]) + 
                            physicsDatas.vector().size()*sizeof(physicsDatas[0]) + 
                            //transforms.vector().size()*sizeof(transforms[0]) + 
                            //inputs.vector().size()*sizeof(inputs[0]) + 
                            indices[0]*sizeof(indices[0]) + 
                            //coordinateSystems.vector().size()*sizeof(coordinateSystems[0]) + 
                            globalVel.vector().size()*sizeof(globalVel[0]) + sizeof(Gravity);

  const size_t memory = verts.vector().buffer_size() + 
                        objects.vector().buffer_size() + 
                        physicsDatas.vector().buffer_size() + 
                        //transforms.vector().buffer_size() + 
                        //inputs.vector().buffer_size() + 
                        indices.vector().buffer_size() + 
                        //coordinateSystems.vector().buffer_size() + 
                        globalVel.vector().buffer_size() + sizeof(Gravity);

  std::cout << "Physics engine data" << '\n';
  std::cout << "Object count           " << objects.size() << '\n';
  std::cout << "Dynamic object count   " << physicsDatas.size() << '\n';
  std::cout << "Total memory usage     " << memory << " bytes" << "\n";
  std::cout << "Total memory with data " << memoryUsed << " bytes" << "\n";
  std::cout << "Free memory            " << (memory - memoryUsed) << " bytes" << '\n';
  std::cout << "Physics class size     " << sizeof(GPUPhysics) << " bytes" << '\n';
  
  broad->printStats();
  narrow->printStats();
  solver->printStats();
  sorter->printStats();
}

void GPUPhysics::construct(yavf::Device* device, yavf::ComputeTask* task, Broadphase* b, Narrowphase* n, Solver* s, PhysicsSorter* sorter) {
  this->broad = b;
  this->narrow = n;
  this->solver = s;
  this->sorter = sorter;
  
  this->device = device;
  this->task = task;
  
  // сейчас мне не обязательно резервировать здесь места
  // но скорее всего желательно
  verts.construct(device);
  verts.vector().reserve(3000);
  verts.update();

  objects.construct(device);
  objects.vector().reserve(3000);
  objects.update();
  staticPhysicDatas.construct(device);
  staticPhysicDatas.vector().reserve(3000);
  staticPhysicDatas.update();

  // coordinateSystems.construct(device);
  // coordinateSystems.resize(2);
  // coordinateSystems[0] = glm::mat4(1.0f);
  // coordinateSystems[1] = glm::mat4(1.0f);
  
  physicsDatas.construct(device);
  physicsDatas.vector().reserve(3000);
  physicsDatas.update();
  // transforms.construct(device);
  // transforms.vector().reserve(3000);
  // transforms.update();

  // inputs.construct(device);
  // inputs.vector().reserve(3000);
  // inputs.update();
  indices.construct(device);
//   indices.vector().reserve(300);
//   indices.update();
  indices.resize(3000);

  //rotationDatas.construct(device);

  rays.construct(device);
  memset(rays.data(), UINT32_MAX, sizeof(RayData));
  frustums.construct(device);
  memset(frustums.data(), UINT32_MAX, sizeof(FrustumStruct));
  frustumPoses.construct(device);

  globalVel.construct(device, 3000);

  uniform.construct(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  
  yavf::DescriptorPool pool = device->descriptorPool("physics_descriptor_pool");
  
  {
    yavf::DescriptorPoolMaker dpm(device);
    
    if (pool == VK_NULL_HANDLE) {
      pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 40).poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5).create("physics_descriptor_pool");
    }
  }
    
  yavf::DescriptorSetLayout layout = device->setLayout("physics_compute_layout");
  yavf::DescriptorSetLayout layoutUniform = device->setLayout("physics_uniform_layout");
  yavf::DescriptorSetLayout objectLayout = device->setLayout("physics_object_layout");
  //yavf::DescriptorSetLayout phys_data_layout = device->setLayout("phys_data_layout");
  {
    yavf::DescriptorLayoutMaker dlm(device);
    
    if (layout == VK_NULL_HANDLE) {
      layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("physics_compute_layout");
    }
    
    if (layoutUniform == VK_NULL_HANDLE) {
      layoutUniform = dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("physics_uniform_layout");
    }
    
    if (objectLayout == VK_NULL_HANDLE) {
      objectLayout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                         binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                         binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                         binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                         binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("physics_object_layout");
    }

    // if (phys_data_layout == VK_NULL_HANDLE) {
    //   phys_data_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
    //                          binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create("phys_data_layout");
    // }
  }
  
  yavf::PipelineLayout pipeline_layout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);
    
    pipeline_layout = plm.addDescriptorLayout(objectLayout).
                          addDescriptorLayout(layout).
                          addDescriptorLayout(layout).
                          addDescriptorLayout(layout).
                          addDescriptorLayout(layout).
                          addDescriptorLayout(layoutUniform).create("velocity_pipeline_layout");
  }
  
  yavf::Pipeline pipe;
  {
    yavf::ComputePipelineMaker pm(device);
    yavf::raii::ShaderModule compute(device, (Global::getGameDir() + "shaders/velocity.spv").c_str());
    
    float data[5] = {0.0f, glm::uintBitsToFloat(256), 5.0f, 5.0f, 4.0f};
    pipe = pm.addSpecializationEntry(0, 0*sizeof(float),    sizeof(float)).
              addSpecializationEntry(1, 1*sizeof(float),    sizeof(uint32_t)).
              addSpecializationEntry(2, 2*sizeof(uint32_t), sizeof(float)).
              addSpecializationEntry(3, 3*sizeof(float),    sizeof(float)).
              addSpecializationEntry(4, 4*sizeof(float),    sizeof(float)).
              addData(5*sizeof(float), data).
              shader(compute).create("velocity_pipeline", pipeline_layout);
  }
  
  first = pipe;
  
  // буферы броадфазы
  overlappingPairCache.construct(device, 4097);
  staticOverlappingPairCache.construct(device, 4097);
  rayPairCache.construct(device, 1000);
  frustumTestsResult.construct(device, 10000); // этот буфер должен быть внешним
  
  // буферы для нарров фазы
  islands.construct(device, 1000);
  staticIslands.construct(device, 1000);
  
  // буферы солвера
  overlappingData.construct(device, 4097);                                                                 // } 
  dataIndices.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT); // } эти буферы должны быть внешними по идее
  raysData.construct(device, 4097);                                                                        // }
  raysIndices.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT); // }
  triggerIndices.construct(device, 4097);
  
  {
    yavf::DescriptorMaker dm(device);
    // auto descriptors = dm.layout(phys_data_layout).layout(layout).layout(layout).layout(layout).
    //                       layout(layout).layout(layout).layout(layout).layout(layout).layout(layout).
    //                       layout(layoutUniform).layout(objectLayout).create(pool);
    
    {
      yavf::DescriptorSet* d = dm.layout(objectLayout).create(pool)[0];
      
      size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      objects.vector().setDescriptor(d, index);

      index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      verts.vector().setDescriptor(d, index);

      index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      physicsDatas.vector().setDescriptor(d, index);

      index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      staticPhysicDatas.vector().setDescriptor(d, index);

      index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      globalVel.vector().setDescriptor(d, index);
    }
    
    // {
    //   //yavf::Descriptor d = dm.layout(layout).create(pool)[0];
      
    //   yavf::DescriptorUpdate du{
    //     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //     0,
    //     0,
    //     descriptors[1]
    //   };
      
    //   inputs.vector().setDescriptorData(du);
    // }
    
    // {
    //   //yavf::Descriptor d = dm.layout(layout).create(pool)[0];
      
    //   yavf::DescriptorUpdate du{
    //     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //     0,
    //     0,
    //     descriptors[2]
    //   };
      
    //   transforms.vector().setDescriptorData(du);
    // }
    
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
      
      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      indices.vector().setDescriptor(d, index);
    }

    // {
    //   //yavf::Descriptor d = dm.layout(layout).create(pool)[0];
      
    //   yavf::DescriptorUpdate du{
    //     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //     0,
    //     0,
    //     descriptors[4]
    //   };

    //   globalVel.vector().setDescriptorData(du);
    // }

    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];

      const size_t index = d->add({rays.vector().handle(), 0, rays.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      rays.vector().setDescriptor(d, index);
    }

    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];

      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      frustums.vector().setDescriptor(d, index);
    }

    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
      
      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      frustumPoses.vector().setDescriptor(d, index);
    }

    // {
    //   yavf::DescriptorUpdate du{
    //     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //     0,
    //     0,
    //     descriptors[8]
    //   };

    //   staticPhysicDatas.vector().setDescriptorData(du);
    // }

    // {
    //   yavf::DescriptorUpdate du{
    //     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //     0,
    //     0,
    //     descriptors[7]
    //   };

    //   rotationDatas.vector().setDescriptorData(du);
    // }

    //staticPhysDataDesc = descriptors[8];
    
    {
      yavf::DescriptorSet* d = dm.layout(layoutUniform).create(pool)[0];

      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
      uniform.buffer()->setDescriptor(d, index);
    }
    
    // {
    //   //yavf::Descriptor d = dm.layout(objectLayout).create(pool)[0];
      
    //   yavf::DescriptorUpdate du{
    //     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //     0,
    //     0,
    //     descriptors[6]
    //   };
      
    //   objects.vector().setDescriptorData(du);
      
    //   du.bindingNum = 1;
    //   verts.vector().setDescriptorData(du);
      
    //   du.bindingNum = 2;
    //   coordinateSystems.vector().setDescriptorData(du);

    //   du.bindingNum = 3;
    //   staticPhysicDatas.vector().setDescriptorData(du);
    // }
    
//     {
//       yavf::Descriptor d = dm.layout(layout).create(pool)[0];
//       
//       yavf::DescriptorUpdate du{
//         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//         0,
//         0,
//         d
//       };
//       
//       physicsDatas2->setDescriptorData(du);
//     }

    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];

      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      islands.vector().setDescriptor(d, index);
    }
    
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];

      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      staticIslands.vector().setDescriptor(d, index);
    }
    
    // буферы броадфазы
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];

      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      overlappingPairCache.vector().setDescriptor(d, index);
    }
    
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];

      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      staticOverlappingPairCache.vector().setDescriptor(d, index);
    }
    
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
      
      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      rayPairCache.vector().setDescriptor(d, index);
    }
    
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
      
      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      frustumTestsResult.vector().setDescriptor(d, index);
    }
    
    // буферы солвера
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
      
      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      overlappingData.vector().setDescriptor(d, index);
    }
    
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
      
      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      dataIndices.buffer()->setDescriptor(d, index);
    }
    
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
      
      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      raysData.vector().setDescriptor(d, index);
    }
    
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
      
      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      raysIndices.buffer()->setDescriptor(d, index);
    }
    
    {
      yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
      
      const size_t index = d->add({objects.vector().handle(), 0, objects.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      triggerIndices.vector().setDescriptor(d, index);
    }
  }

  //updateStaticPhysDataDesc();
  
  updateInputOutput();
  
//   {
//     Broadphase::InputBuffers input{
//       &indices,
//       &verts, 
//       &objects, 
//       matrices, 
//       transforms, 
//       rotationDatas, 
//       &rays, 
//       &frustums, 
//       &frustumPoses
//     };
//     
//     broad->setInputBuffers(input);
//     
//     Broadphase::OutputBuffers output{
//       &overlappingPairCache,
//       &staticOverlappingPairCache,
//       &rayPairCache,
//       &frustumTestsResult
//     };
//     
//     // нужно будет сюда добавить индирект буфер
//     broad->setOutputBuffers(output);
//   }
//   
//   //broad->setCollisionTestBuffer(&indices);
//   //broad->setRayTestBuffer();
//   //broad->setUpdateBuffers({&verts, &objects, &coordinateSystems, &transforms, &rotationDatas, &rays, &frustums, &frustumPoses});
//   //broad->setUpdateBuffers({&verts, &objects, matrices, transforms, rotationDatas, &rays, &frustums, &frustumPoses});
//   
//   {
//     Narrowphase::InputBuffers input{
//       &overlappingPairCache,
//       &staticOverlappingPairCache,
//     };
//     
//     narrow->setInputBuffers(input);
//     
//     Narrowphase::OutputBuffers output{
//       &islands, 
//       &staticIslands
//     };
//     
//     narrow->setOutputBuffers({&islands, &staticIslands});
//   }
// 
//   //narrow->setPairBuffer(broad->getOverlappingPairCache(), broad->getIndirectPairBuffer());
//   //narrow->setInputBuffers({broad->getOverlappingPairCache(), broad->getStaticOverlappingPairCache()}, broad->getIndirectPairBuffer());
//   //narrow->setOutputBuffers({&islands, &staticIslands});
//   
//   {
//     Solver::InputBuffers input{
//       &objects, 
//       &verts, 
//       matrices, 
//       &physicsDatas, 
//       transforms, 
//       &staticPhysicDatas, 
//       rotationDatas,
//       
//       &overlappingPairCache,
//       &staticOverlappingPairCache,
//       
//       &islands, 
//       &staticIslands,
//       
//       &indices, 
//       &globalVel, 
//       &uniform, 
//       &rays, 
//       
//       &rayPairCache
//     };
//     
//     // нужно добавить индирект буферы
//     solver->setInputBuffers(input);
//     
//     Solver::OutputBuffers output{
//       &overlappingData,
//       &dataIndices,
//       &raysData,
//       &raysIndices,
//       &triggerIndices
//     };
//     
//     solver->setOutputBuffers(output);
//   }

  // solver->setBuffers({&objects, &verts, &coordinateSystems, 
  //                     &physicsDatas, &transforms, &staticPhysicDatas, 
  //                     &rotationDatas, broad->getOverlappingPairCache(), 
  //                     narrow->getIslandDataBuffer(), &indices, &globalVel, 
  //                     &uniform, &rays, broad->getRayPairCache()}, narrow->getIndirectIslandCount(), broad->getIndirectPairBuffer());

//   solver->setBuffers({&objects, 
//                       &verts, 
//                       matrices, 
//                       &physicsDatas, 
//                       transforms, 
//                       &staticPhysicDatas, 
//                       rotationDatas, 
//                      
//                       broad->getOverlappingPairCache(), 
//                       broad->getStaticOverlappingPairCache(),
//                      
//                       &islands, 
//                       &staticIslands,
//                      
//                       &indices, 
//                       &globalVel, 
//                       &uniform, 
//                       &rays, 
//                       broad->getRayPairCache()}, narrow->getIndirectIslandCount(), broad->getIndirectPairBuffer());
}

void GPUPhysics::updateStaticPhysDataDesc() {
  yavf::DescriptorUpdater du(device);

  du.currentSet(staticPhysDataDesc->handle()).
     begin(0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).
     buffer(staticPhysicDatas.vector().handle(), 0, staticPhysicDatas.vector().buffer_size()).update();
}

void GPUPhysics::updateBuffers() {
  if (objects.size()+1 > indices.size()) indices.resize(objects.size()+1);
  if (physicsDatas.size() > globalVel.size()) globalVel.resize(physicsDatas.size());
}

void GPUPhysics::updateInputOutput() {
  {
    Broadphase::InputBuffers input{
      &indices,
      &verts, 
      &objects, 
      matrices, 
      transforms, 
      rotationDatas, 
      &rays, 
      &frustums, 
      &frustumPoses
    };
    
    broad->setInputBuffers(input);
    
    Broadphase::OutputBuffers output{
      &overlappingPairCache,
      &staticOverlappingPairCache,
      &rayPairCache,
      &frustumTestsResult
    };
    
    // нужно будет сюда добавить индирект буфер
    broad->setOutputBuffers(output);
  }
  
  {
    Narrowphase::InputBuffers input{
      &overlappingPairCache,
      &staticOverlappingPairCache,
    };
    
    narrow->setInputBuffers(input);
    
    Narrowphase::OutputBuffers output{
      &islands, 
      &staticIslands
    };
    
    narrow->setOutputBuffers(output);
  }
  
  {
    Solver::InputBuffers input{
      &objects, 
      &verts, 
      matrices, 
      &physicsDatas, 
      transforms, 
      &staticPhysicDatas, 
      rotationDatas,
      
      &overlappingPairCache,
      &staticOverlappingPairCache,
      
      &islands, 
      &staticIslands,
      
      &indices, 
      &globalVel, 
      &uniform, 
      &rays, 
      
      &rayPairCache
    };
    
    // нужно добавить индирект буферы
    solver->setInputBuffers(input);
    
    Solver::OutputBuffers output{
      &overlappingData,
      &dataIndices,
      &raysData,
      &raysIndices,
      &triggerIndices
    };
    
    solver->setOutputBuffers(output);
  }
}

GPUPhysicsDirectPipeline::GPUPhysicsDirectPipeline(yavf::Device* device, yavf::ComputeTask* task, const GPUPhysicsCreateInfo &info) : 
  GPUPhysics(device, task, info) {}

GPUPhysicsDirectPipeline::~GPUPhysicsDirectPipeline() {
  device->destroy(first);

  device->destroyLayout("velocity_pipeline_layout");
  device->destroySetLayout("physics_compute_layout");
  device->destroySetLayout("physics_uniform_layout");
  device->destroySetLayout("physics_object_layout");

  device->destroyDescriptorPool("physics_descriptor_pool");
}

void GPUPhysicsDirectPipeline::update(const uint64_t &time) {
  Gravity* data = uniform.data();
  data->objCount = physicsDatas.size();
  data->time = time;
  data->gravity = gravity;// glm::vec4(0.0f, 9.8f, 0.0f, 0.0f);
  data->gravityNormal = gravityNorm;// glm::normalize(data->gravity);
  data->length2 = gravLength2;// glm::length2(data->gravity);
  data->length = gravLength;// glm::sqrt(data->length2);

  //broad->updateBuffers(objects.size(), inputs.size(), rays.size(), frustums.size());
//   broad->updateBuffers(objects.size(), physicsDatas.size(), rays.size(), frustums.size());
//   narrow->updateBuffers(broad->getOverlappingPairCache()->at(0).firstIndex);
//   solver->updateBuffers(broad->getOverlappingPairCache()->at(0).firstIndex, broad->getRayPairCache()->at(0).firstIndex);

//   broad->updateBuffers(objects.size(), physicsDatas.size(), rays.size(), frustums.size());
//   narrow->updateBuffers(broad->getOverlappingPairCache()->size(), broad->getStaticOverlappingPairCache()->size());
//   solver->updateBuffers(broad->getOverlappingPairCache()->size(), broad->getRayPairCache()->size());

  broad->updateBuffers(objects.size(), physicsDatas.size(), rays.size(), frustums.size());
  narrow->updateBuffers(overlappingPairCache.size(), staticOverlappingPairCache.size());
  solver->updateBuffers(overlappingPairCache.size() + staticOverlappingPairCache.size(), rayPairCache.size());

  task->begin();
  
  task->setPipeline(first);
  // task->setDescriptor({physicsDatas.vector().descriptor(), 
  //                      inputs.vector().descriptor(), 
  //                      transforms.vector().descriptor(), 
  //                      indices.vector().descriptor(), 
  //                      globalVel.vector().descriptor(), 
  //                      uniform.buffer()->descriptor()}, 0);

  task->setDescriptor({objects.vector().descriptorSet()->handle(), 
                       inputDesc->handle(), 
                       transformDesc->handle(), 
                       externalsDesc->handle(),
                       indices.vector().descriptorSet()->handle(), 
                       uniform.buffer()->descriptorSet()->handle()}, 0);
  // здесь наверное будет только перевычисление PhysicData так как остальное непосредственно зависит от самой фазы
  task->dispatch(1, 1, 1);

  sorter->barrier();
  
  broad->update();
  sorter->barrier();
  broad->calculateOverlappingPairs();
  sorter->barrier();

  //narrow->calculateIslands();
  narrow->calculateBatches();
  sorter->barrier();
  //narrow->checkIdenticalPairs();
  //narrow->sortPairs();
  sorter->sort(&overlappingPairCache);
  sorter->barrier();
  narrow->postCalculation();

  solver->calculateData();
  sorter->barrier();

  solver->solve();
  sorter->barrier();

  broad->update();
  sorter->barrier();

  broad->calculateRayTests();
  broad->calculateFrustumTests();
  sorter->barrier();
  
  solver->calculateRayData();
  sorter->barrier();

  sorter->sort(&frustumTestsResult);
  sorter->sort(&overlappingData, &dataIndices, 0);
  sorter->sort(&raysData, &raysIndices, 1);

  task->end();
  
  auto start = std::chrono::steady_clock::now();
  task->start();
  task->wait();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  
  std::cout << "Physics update time: " << ns << " mcs" << "\n";

  overlappingDataSize = dataIndices[0].temporaryCount;
  rayTracingSize = raysIndices[0].temporaryCount;
  frustumTestSize = frustumTestsResult[0].firstIndex;

  memset(rays.data(), UINT32_MAX, sizeof(RayData));
  memset(frustums.data(), UINT32_MAX, sizeof(FrustumStruct));
  memset(frustumTestsResult.data(), 0, sizeof(BroadphasePair));
  memset(raysIndices.data(), 0, sizeof(DataIndices));
}
