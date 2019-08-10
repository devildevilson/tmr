#include "DecalSystem.h"

#include "Utility.h"
#include "Globals.h"
#include "Physics.h"
#include "MemoryPool.h"
#include "UserData.h"
#include "DecalComponent.h"
#include "Components.h"

#include "CompositeType.h"

struct DecalUserData {
  
};

struct DecalPhysContainer {
  PhysicsIndexContainer container;
  DecalUserData userData;
  PendingDecalData data;
  yacs::entity* decalEntity;
  DecalComponent* comp;
  uint32_t matrixIndex;
  TransformComponent* transform;
};

static MemoryPool<DecalPhysContainer, sizeof(DecalPhysContainer)*20> indexContainerPool;

void DecalSystem::setContainer(Container<simd::mat4>* matrixContainer) {
  DecalSystem::matrix = matrixContainer;
}

void DecalSystem::setContainer(Container<Transform>* transformContainer) {
  DecalSystem::transforms = transformContainer;
}

DecalSystem::DecalSystem() {}
DecalSystem::~DecalSystem() {
  
}

// #define EPSILON 0.000001f

void DecalSystem::update(const uint64_t &time) {
  // значит
  // во вторых мы должны все полученные точки клипать и получить несколько кусочков декали
  // понятно что мне нужно будет еще проверить на лишние точки 
  
  // для каждой декали которую нужно обработать
  for (size_t i = 0; i < pendingPhysics.size(); ++i) {
    // для каждой стены которую пересекает декаль
    
    const PendingDecalData &decalData = pendingPhysics[i]->data;
    const DecalPhysContainer* decalContainer = pendingPhysics[i];
    
    // получаем все пересечения с декалью
    // у меня все пересечения может и приходят в отсортированном виде, но мне от этого не легче
    // нужно подкорректировать интерфейс физики
    const ArrayInterface<uint32_t>* triggerPairs = Global::physics()->getTriggerPairsIndices();
    const ArrayInterface<OverlappingData>* collisionPairs = Global::physics()->getOverlappingPairsData();
    const uint32_t triggerSize = Global::physics()->getTriggerPairsIndicesSize();
    
    for (size_t j = 0; j < triggerSize; ++j) {
      const uint32_t index = triggerPairs->at(j);
      const OverlappingData &pair = collisionPairs->at(index);
      const PhysicsIndexContainer* cont1 = Global::physics()->getIndexContainer(pair.firstIndex);
      const PhysicsIndexContainer* cont2 = Global::physics()->getIndexContainer(pair.secondIndex);
      
      if (!(&pendingPhysics[i]->container == cont1 || &pendingPhysics[i]->container == cont2)) continue;
      
      // вычисляем кусочек декали
      const auto* objPtr = &pendingPhysics[i]->container == cont1 ? cont2 : cont1;
      
      PhysUserData* objData = reinterpret_cast<PhysUserData*>(objPtr->userData);
      if (objData->decalContainer == nullptr) continue;
      
      const uint32_t objPointsSize = Global::physics()->getObjectShapePointsSize(objPtr);
      const simd::vec4* objPoints = Global::physics()->getObjectShapePoints(objPtr);
      const uint32_t objFacesSize = Global::physics()->getObjectShapeFacesSize(objPtr);
      const simd::vec4* objFaces = Global::physics()->getObjectShapeFaces(objPtr);
      
      const simd::vec4 wallNormal = objFaces[0];
      // у декали должна быть нормаль для того чтобы мы не рисовали ее с другой стороны
      const simd::vec4 decalNormal = decalData.orn[2]; // по идее это orn[2]
      
      // в этом случае рисовать ненужно
      if (simd::dot(wallNormal, decalNormal) < 0.0f) continue;
      
      // нам нужны базисные векторы декали 
      const simd::vec4 normals[2] = {decalData.orn[0], decalData.orn[1]};
      // по идее даже вычислять не нужно
      
      // нужно еще скейл учесть
      
      // инвертированный базис для того чтобы привести точки в пространство декали
      const uint32_t matrixIndex = Global::physics()->getMatrixIndex(objPtr);
      const uint32_t transformIndex = Global::physics()->getTransformIndex(objPtr);
      // ротатион
      const simd::mat4 objMatrix = matrixIndex == UINT32_MAX ? simd::mat4(1.0f) : matrix->at(matrixIndex);
      const simd::vec4 trans = transformIndex == UINT32_MAX ? simd::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(transformIndex).pos;
      const simd::mat4 objBasis = objMatrix * simd::translate(simd::mat4(1.0f), trans);
      const simd::mat4 invObjBasis = simd::inverse(objBasis);
      
      // как то так по идее
      const simd::mat4 decalMatrix = simd::transpose(simd::mat4(decalData.orn[0], decalData.orn[1], decalData.orn[2], decalData.pos));
      const simd::mat4 basis = decalMatrix * invObjBasis;
      const simd::mat4 invBasis = simd::inverse(decalMatrix) * objBasis;
      
      // заведем массив вершин
      // посчитаем их в новом спейсе
      std::vector<simd::vec4> verts;
      for (uint32_t k = 0; k < objPointsSize; ++k) {
        verts.push_back(invBasis * objPoints[k]);
      }
      
      // ВАЖНО!!! декаль у нас может пересечь стену уже немного пододвинутую (то есть открытую дверь)
      // это значит что точки объекта нужно сначало привести в мировые координаты
      // как это сделать? по идее нужно rotation * matrix * (pos+point) и вот это умножить на invBasis
      // и тоже самое нужно сделать (только в обратную сторону) когда мы вычисляем позицию уже непосредственно декалины
      // то есть базис это не просто ориентация декали, а orn * rotation * matrix и проще будет наверное еще сделать трансформ матрицу
      
      const float width = 1.0f;
      const float height = decalData.scale;
      
      const float sizeX = width * decalData.size;
      const float sizeY = height * decalData.size;
      
      const float halfSizeX = sizeX / 2.0f;
      const float halfSizeY = sizeY / 2.0f;
      
      // начинаем клипать, клипать лучше всего по каждой нормали по отдельности
      // по идее клипать мы можем и текстурные координаты
      std::vector<simd::vec4> clippedVerts;
      clip(verts, clippedVerts,  normals[0],  halfSizeX); // это должны быть нормализованные координаты декали
      clip(clippedVerts, verts,  normals[1],  halfSizeY); // то есть условная высота взятая как 1 и ширина - часть высоты
      clip(verts, clippedVerts, -normals[0], -halfSizeX); // делить на 2 не забыть
      clip(clippedVerts, verts, -normals[1], -halfSizeY); // 0.5 подходит для квадратной текстуры
      
      // теперь мы должны выкинуть повторяющиеся и совпадающие точки
      for (size_t k = 0; k < verts.size(); ++k) {
        const size_t checkPointIndex = (k+1)%verts.size();
        const size_t nextIndex = (k+2)%verts.size();
        const bool side = sideOf(verts[k], verts[nextIndex], verts[checkPointIndex], decalNormal) > EPSILON;
        
        if (!side) {
          verts.erase(verts.begin() + checkPointIndex);
          --k;
        }
      }
      
      // если по итогу точек останется меньше 3, то значит это не то что нам нужно
      if (verts.size() < 3) continue;
      
      // и после всего этого мы должны составить фигуру
      // то есть умножить точки обратно на базис
      // приладить к этому нормаль
      // и посчитать текстурную координату
//       const uint32_t texturesIndex = textures.size();
//       textures.push_back(decalData.texture);
      float normalArr[4];
      //decalNormal.storeu(normalArr);
      wallNormal.storeu(normalArr);
      
//       size_t offset = indices.size();
//       size_t size = verts.size()+1;
      
      std::vector<Vertex> vertices;
      
      for (size_t k = 0; k < verts.size(); ++k) {
        float arr[4];
        verts[i].storeu(arr);
        
        // как считать текстурную координату?
        // у нас же verts в пространстве декали, следовательно мы можем просто взять xy
        // нам нужно только правильно вычесть или прибавить половинку нормализованной высоты/ширины
        //const float texX = (arr[0] - 0.5f) / (-0.5f - 0.5f);
        const float texX = arr[0] - halfSizeX;
        const float texY = arr[1] - halfSizeY;
        
//         indices.push_back(vertices.size());
        const Vertex vert{
          basis * verts[i],
          simd::vec4(normalArr[0], normalArr[1], normalArr[2], 0.0f),
          glm::vec2(texX, texY)
        };
        vertices.push_back(vert);
      }
//       indices.push_back(UINT32_MAX);

      // также мы должны раскидать кусочки декалей по энтитям
      // декалКомпонент.аддДекал(offset, size)
      // + надо запомнить индекс текстурки, чтобы потом по нему найти текстурку при отрисовке
      // тут тоже вообще то могут быть проблемы с компиляцией
      {
        UniqueID id = objData->decalContainer->addDecal(vertices, 0, decalContainer->comp->textureContainerIndex());
        const DecalComponent::ContainerData data{
          objData->decalContainer,
          id
        };
        decalContainer->comp->add(data);
      }
      
      // как то так
    }
    
    Global::physics()->remove(&pendingPhysics[i]->container);
    
    const ComputedDecals computedData{
      decalContainer->decalEntity,
      decalContainer->comp,
      decalContainer->matrixIndex
    };
    decals.push_back(computedData);
  }
  
  for (size_t i = 0; i < pendingPhysics.size(); ++i) {
//     matrix->erase(pendingPhysics[i]->matrixIndex);
//     transforms->erase(pendingPhysics[i]->transformIndex);
    indexContainerPool.deleteElement(pendingPhysics[i]);
  }
  pendingPhysics.clear();
  
  // во первых мы должны создать для декали компонент
  // я чувствую что это может не скомпилиться на венде
  // компонент найдет нам все стены на которые можно налепить декали
  // для того чтобы все сделать правильно, мне нужно 
  // уметь удалять физический объект (я не помню умею ли я сейчас)
  // нам еще позицию добавить в контейнер нужно
//   for (size_t i = 0; i < pendingDecals.size(); ++i) {
//     const float width = 1.0f;
//     const float height = pendingDecals[i].scale;
//     
//     const float sizeX = width * pendingDecals[i].size;
//     const float sizeY = height * pendingDecals[i].size;
//     
//     const uint32_t matrixIndex = matrix->insert(pendingDecals[i].orn);
//     const uint32_t transformIndex = transforms->insert({pendingDecals[i].pos, pendingDecals[i].orn[2], simd::vec4(sizeX, sizeY, 1.0f, 0.0f)});
//     
//     const PhysicsObjectCreateInfo obj{
//       false,
//       PhysicsType(false, BBOX_TYPE, false, true, false, true),
//       1, // coll group
//       1, // coll filter, я уже забыл что есть что, мне нужно чтобы декали пересекали только стены
//       0.0f,
//       1.0f,
//       0.0f,
//       0.0f,
//       UINT32_MAX,
//       transformIndex,
//       UINT32_MAX,
//       matrixIndex,
//       UINT32_MAX,
//       Type::get("boxShape")
//     };
//     
//     yacs::Entity* ent = Global::world()->createEntity();
//     auto comp = ent->add<DecalComponent>(pendingDecals[i].texture);
//     
//     auto ptr = indexContainerPool.newElement();
//     ptr->data = pendingDecals[i];
//     ptr->matrixIndex = matrixIndex;
//     ptr->transformIndex = transformIndex;
//     ptr->decalEntity = ent;
//     ptr->comp = comp.get();
//     Global::physics()->add(obj, &ptr->container);
//     pendingPhysics.push_back(ptr);
//     
// //     CompositeType<uint32_t> type("wall");
// //     CompositeType<uint32_t> type1("wall");
// //     auto type3 = type & type1;
//     
//     // по идее это все что мне нужно
//     // этот объект мы удалим в следующем кадре
//     // необходимо только заполнить верно несколько полей
//   }
//   
//   pendingDecals.clear();
}

yacs::entity* DecalSystem::add(const PendingDecalData &data) {
//   pendingDecals.push_back(data);
  const float width = 1.0f;
  const float height = data.scale;
  
  const float sizeX = width * data.size;
  const float sizeY = height * data.size;
  
  const uint32_t matrixIndex = matrix->insert(data.orn);
//   const uint32_t transformIndex = transforms->insert({data.pos, data.orn[2], simd::vec4(sizeX, sizeY, 1.0f, 0.0f)});
  
  yacs::entity* ent = Global::world()->createEntity();
  auto comp = ent->add<DecalComponent>(data.texture);
  auto transform = ent->add<TransformComponent>(data.pos, data.orn[2], simd::vec4(sizeX, sizeY, 1.0f, 0.0f));
  
  const PhysicsObjectCreateInfo obj{
    false,
    PhysicsType(false, BBOX_TYPE, false, true, false, true),
    1, // coll group
    1, // coll filter, я уже забыл что есть что, мне нужно чтобы декали пересекали только стены
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    UINT32_MAX,
    transform->transformIndex,
    UINT32_MAX,
    matrixIndex,
    UINT32_MAX,
    Type::get("boxShape")
  };
  
  auto ptr = indexContainerPool.newElement();
  ptr->data = data;
  ptr->matrixIndex = matrixIndex;
  ptr->transform = transform.get();
  ptr->decalEntity = ent;
  ptr->comp = comp.get();
  Global::physics()->add(obj, &ptr->container);
  pendingPhysics.push_back(ptr);
}

void DecalSystem::remove(yacs::entity* ent) {
  for (size_t i = 0; i < decals.size(); ++i) {
    if (decals[i].ent == ent) {
//       std::swap(decals[i], decals.back());
//       Global::world()->removeEntity(decals.back().ent);
//       matrix->erase(decals.back().matrixIndex);
//       decals.pop_back();
//       break;
      decals[i].comp->clear();
      Global::world()->removeEntity(decals[i].ent);
      matrix->erase(decals[i].matrixIndex);
      decals.erase(decals.begin()+i);
      break;
    }
  }
}

void DecalSystem::removeOld() {
  decals.front().comp->clear();
  Global::world()->removeEntity(decals.front().ent);
  matrix->erase(decals.front().matrixIndex);
  decals.erase(decals.begin());
}

void DecalSystem::changePlace(yacs::entity* ent, const PendingDecalData &data) {
  for (size_t i = 0; i < decals.size(); ++i) {
    if (decals[i].ent == ent) {
      const float width = 1.0f;
      const float height = data.scale;
      
      const float sizeX = width * data.size;
      const float sizeY = height * data.size;
      
      //const uint32_t matrixIndex = matrix->insert(data.orn);
      matrix->at(decals[i].matrixIndex) = data.orn;
    //   const uint32_t transformIndex = transforms->insert({data.pos, data.orn[2], simd::vec4(sizeX, sizeY, 1.0f, 0.0f)});
      
    //   yacs::Entity* ent = Global::world()->createEntity();
    //   auto comp = ent->add<DecalComponent>(data.texture);
    //   auto transform = ent->add<TransformComponent>(data.pos, data.orn[2], simd::vec4(sizeX, sizeY, 1.0f, 0.0f));
      auto transform = decals[i].ent->get<TransformComponent>();
      transform->pos() = data.pos;
      transform->rot() = data.orn[2];
      transform->scale() = simd::vec4(sizeX, sizeY, 1.0f, 0.0f);
      decals[i].comp->setTexture(data.texture);
      
      const PhysicsObjectCreateInfo obj{
        false,
        PhysicsType(false, BBOX_TYPE, false, true, false, true),
        1, // coll group
        1, // coll filter, я уже забыл что есть что, мне нужно чтобы декали пересекали только стены
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        UINT32_MAX,
        transform->transformIndex,
        UINT32_MAX,
        decals[i].matrixIndex,
        UINT32_MAX,
        Type::get("boxShape")
      };
      
      auto ptr = indexContainerPool.newElement();
      ptr->data = data;
      ptr->matrixIndex = decals[i].matrixIndex;
      ptr->transform = transform.get();
      ptr->decalEntity = ent;
      ptr->comp = decals[i].comp;
      Global::physics()->add(obj, &ptr->container);
      pendingPhysics.push_back(ptr);
      break;
    }
  }
}

void DecalSystem::clip(const std::vector<simd::vec4> &inVerts, std::vector<simd::vec4> &outVerts, const simd::vec4 &normal, const float &dist) {
  outVerts.clear();
  if (inVerts.empty()) return;
  
  // тут наверное лучше транспозить с помощью simd
  const simd::vec4 tmp = normal + simd::vec4(0.0f, 0.0f, 0.0f, -dist);
  const simd::mat4 matNorm = simd::transpose(simd::mat4(tmp, tmp, tmp, tmp));
  
  simd::vec4 prev = inVerts.back();
  float prevDot = simd::dot(prev, tmp);
  for (size_t k = 0; k < inVerts.size(); k+=4) {
//           simd::vec4 vert; // каждую вершину умножим на invBasis
    // если p1 находится внутри декали dot(vert, normalS), dot(vert, normalT), dot(vert, -normalS), dot(vert, -normalT) < 0.0f
    // и если p(size-1) находится внутри, добавляем p1
    
    float arrVert1[4];
    inVerts[k+0].storeu(arrVert1);
    float arrVert2[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    if (k+1 < inVerts.size()) inVerts[k+1].storeu(arrVert2);
    float arrVert3[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    if (k+2 < inVerts.size()) inVerts[k+2].storeu(arrVert3);
    float arrVert4[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    if (k+3 < inVerts.size()) inVerts[k+3].storeu(arrVert4);
    
    const simd::vec4 dotRes = simd::dot4(simd::vec4(arrVert1[0], arrVert2[0], arrVert3[0], arrVert4[0]), simd::vec4(arrVert1[1], arrVert2[1], arrVert3[1], arrVert4[1]), 
                                         simd::vec4(arrVert1[2], arrVert2[2], arrVert3[2], arrVert4[2]), simd::vec4(arrVert1[3], arrVert2[3], arrVert3[3], arrVert4[3]), 
                                         matNorm[0], matNorm[1], matNorm[2], matNorm[3]);
    float dotResArr[4];
    dotRes.storeu(dotResArr);
    
    const int res = (dotRes < simd::vec4(0.0f)).movemask() | ((prevDot < 0.0f)<<4);
    
    const uint8_t count = std::min(inVerts.size()-k-1, size_t(4));
    for (uint8_t v = 0; v < count; ++v) {
      const bool inside = bool(res & (1<<(3-v))); // pk
      const bool insidePrev = bool(res & (1<<(4-v))); // p(k-1)
      const size_t pkindex = k+v;
      const size_t pk1index = k+v == 0 ? inVerts.size()-1 : k+v-1;
      
      if (inside) {
        if (insidePrev) {
          // добавляем pk
          outVerts.push_back(inVerts[pkindex]);
        } else {
          // находим пересечение p(k-1)-pk с одной из нормалей
          // добавляем пересечение
          // добавляем pk
          
          simd::vec4 point;
          intersect(normal*dist, normal, inVerts[pkindex], inVerts[pk1index], point);
          outVerts.push_back(point);
          outVerts.push_back(inVerts[pkindex]);
        }
      } else {
        if (insidePrev) {
          // находим пересечение pk-p(k-1) с одной из нормалей
          // добавляем пересечение
          simd::vec4 point;
          intersect(normal*dist, normal, inVerts[pk1index], inVerts[pkindex], point);
          outVerts.push_back(point);
        }
      }
    }
    
    // p(k-1) = pk
    const size_t last = std::min(inVerts.size()-1, k+3);
    prev = inVerts[last];
    prevDot = dotResArr[last-k];
  }
}

Container<simd::mat4>* DecalSystem::matrix = nullptr;
Container<Transform>* DecalSystem::transforms = nullptr;
