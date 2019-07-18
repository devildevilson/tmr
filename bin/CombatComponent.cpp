#include "CombatComponent.h"

#include "Globals.h"
#include "Utility.h"
#include "Components.h"
#include "EventComponent.h"
#include "BroadphaseInterface.h"

#include <HelperFunctions.h>

void Interaction::setContainers(Container<Transform>* transforms, Container<simd::mat4>* matrices, Container<RotationData>* rotationDatas) {
  Interaction::transforms = transforms;
  Interaction::matrices = matrices;
  Interaction::rotationDatas = rotationDatas;
}

Container<Transform>* Interaction::transforms = nullptr;
Container<simd::mat4>* Interaction::matrices = nullptr;
Container<RotationData>* Interaction::rotationDatas = nullptr;

PhysicsInteraction::PhysicsInteraction(const CreateInfo &info) : 
  Interaction(Interaction::type::physics, info.eventType, info.userData), 
  attackTime(info.attackTime), 
  lastTime(0), 
  currentTime(0), 
  thickness(info.thickness), 
  attackAngle(info.attackAngle), 
  distance(info.distance), 
  interactionType(info.type), 
  pos{info.pos[0], info.pos[1], info.pos[2], info.pos[3]}, 
  dir{info.dir[0], info.dir[1], info.dir[2], info.dir[3]}, 
  lastPos{info.pos[0], info.pos[1], info.pos[2], info.pos[3]}, 
  lastDir{info.dir[0], info.dir[1], info.dir[2], info.dir[3]}, 
  plane{info.plane[0], info.plane[1], info.plane[2], info.plane[3]} {
  // создаем сферу
  const PhysicsObjectCreateInfo phys_info{
    false,
    PhysicsType(false, SPHERE_TYPE, false, true, false, false),
    1,
    1,
    0.0f,
    0.0f,
    0.0f,
    distance,
    UINT32_MAX,
    info.transformIndex,
    UINT32_MAX,
    info.matrixIndex,
    info.rotationIndex,
    Type()
  };
  
  // понятное дело нужно как то обезопасить создание
  Global::physics()->add(phys_info, &container);
}

PhysicsInteraction::~PhysicsInteraction() {
  Global::physics()->remove(&container);
}

void PhysicsInteraction::update_data(const NewData &data) {
  memcpy(lastPos, pos, sizeof(simd::vec4));
  memcpy(lastDir, dir, sizeof(simd::vec4));
  data.pos.storeu(pos);
  data.dir.storeu(dir);
}

void PhysicsInteraction::update(const size_t &time) {
  // че здесь? раньше я бы здесь раскидал все по set'ам и массивам
  // а сейчас этим занимается get_next()
  
  lastTime = currentTime;
  currentTime += time;
  currentTime = std::min(attackTime, currentTime);
  state = 0;
  
  if (currentTime >= attackTime) finished = true;
}

void PhysicsInteraction::cancel() {
  // это скорее всего не нужно
}

PhysUserData* PhysicsInteraction::get_next() {
  const uint32_t triggerSize = Global::physics()->getTriggerPairsIndicesSize();
  const ArrayInterface<uint32_t>* triggerPairs = Global::physics()->getTriggerPairsIndices();
  const ArrayInterface<OverlappingData>* datas = Global::physics()->getOverlappingPairsData();
  
  // найдем 4 точки, 2 из которых это текущее положение и предыдущее
  // + 2 точки предыдущий дир повернутый на предыдущий угол, и текущий дир повернутый на текущий угол
  // плоскость поворота normal
  const float halfAngle = attackAngle / 2.0f;
  const float currentAngle = (currentTime * (attackAngle / float(attackTime))) - halfAngle;
  const float lastAngle = (lastTime * (attackAngle / float(attackTime))) - halfAngle;
  
  const simd::vec4 simd_pos = simd::vec4(pos);
  const simd::vec4 simd_dir = simd::vec4(dir);
  const simd::mat4 attackRot = simd::orientation(simd_dir, simd::normalize(projectVectorOnPlane(Global::physics()->getGravityNorm(), simd_pos, simd_dir)));
  const simd::vec4 attackPlane = attackRot * Global::physics()->getOrientation() * simd::vec4(plane);
  const simd::vec4 vector1 = simd::normalize(simd::rotate(simd::mat4(1.0f), currentAngle, attackPlane) * simd_dir);
  const simd::vec4 vector2 = simd::normalize(simd::rotate(simd::mat4(1.0f), lastAngle, attackPlane) * simd::vec4(lastDir));
  
  const simd::vec4 A1 = simd::vec4(lastPos);
  const simd::vec4 A2 = A1 + vector2 * distance;
  const simd::vec4 B1 = simd_pos;
  const simd::vec4 B2 = B1 + vector1 * distance;
  
  PhysUserData* usrData = nullptr;
  while (state < triggerSize && usrData == nullptr) {
    const uint32_t index = triggerPairs->at(state+1);
    const OverlappingData &data = datas->at(index);
    ++state;
    
    if (data.firstIndex != container.objectDataIndex && data.secondIndex != container.objectDataIndex) continue;
    
    // если мы нашли пару, то мы теперь должны проверить несколько условий
    // 1) находится ли объект внутри текущего угла, как это проверяется?
    //    вообще раньше я составлял полигон по точкам и проверял, это было прямо так скажем быстрее
    //    я так полагаю лучше всего будет сделать SAT по точкам, заодно может отыскать точку пересечения
    //    скоре всего у меня будет точки 4 (ну кстати говоря как раз здесь можно сделать точность, хотя нужно ли это?)
    //    так нет, достаточно проверить чтобы хотя бы одна точка была справа от левой стороны И слева от правой + не выше и не ниже чем thickness
    // тут еще вот что, мне нужно генерить вызов атаки каждый раз потому что мне нужно генерить декали например
    // в пользу этого также то что мне нужно чтоб противник умирал не сразу, что тут можно сделать?
    // тот урон который я должен нанести - это максимальный урон, как наносить урон по частям? скорее всего никак
    // для этого нужно передавать время, либо по разному обрабатывать поверхности и монстров
    // можно делать вызов каждое определенное время (ну то есть как физика работает)
    // но для этого мне нужен alpha, это проблематично, 
    // что можно сделать? можно сделать 1 вызов, никак особ проверять не нужно
    // можно сделать несколько вызовов, но они будут зависеть от fps
    // можно сделать урон по времени, то есть каждое некоторое время вызывать урон
    // это самый наверное адекватный вариант
    // так же можно вызывать атаку и декали отдельно
    // я тут подумал что много декалей - плохо
    // вызывать постепенно, то есть определить сколько раз по одной цели + делай
    // это походу самый адекватный вариант, как это сделать? 
    // правильно нужно здесь возвращать когда подошло время + вызовов меньше чем количество
    
    const uint32_t objIndex = data.firstIndex == container.objectDataIndex ? data.secondIndex : data.firstIndex;
    
    const PhysicsIndexContainer* another = Global::physics()->getIndexContainer(objIndex);
    const BroadphaseProxy* proxy = Global::physics()->getObjectBroadphaseProxy(another);
    
    const bool ticklessObj = (proxy->collisionGroup() & ticklessObjectsType) != 0;
    
    auto itr = objects.find(objIndex);
    if (itr != objects.end()) {
      if (itr->second.count >= tickCount && !ticklessObj) continue;
      
      const auto timePoint = std::chrono::steady_clock::now();
      const auto diff = timePoint - itr->second.point;
      const size_t mcs = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
      if (mcs < tickTime) continue;
    }
    
    const Object &obj = Global::physics()->getObjectData(another);
    if (obj.objType.getObjType() == BBOX_TYPE) {
      const simd::vec4 pos = transforms->at(obj.transformIndex).pos;
      const simd::vec4 extent = Global::physics()->getObjectShapePoints(another)[0];
      const simd::vec4 scale = transforms->at(obj.transformIndex).scale;
      
      const simd::mat4 matrix = obj.coordinateSystemIndex == UINT32_MAX ? simd::mat4(1.0f) * simd::scale(simd::mat4(1.0f), scale) : matrices->at(obj.coordinateSystemIndex) * simd::scale(simd::mat4(1.0f), scale);
  
      // теперь учитывает, возможна проблема со знаками
      uint32_t pointsLeft = 0;
      uint32_t pointsRight = 0;
      uint32_t pointsAbove = 0;
      uint32_t pointsBeneath = 0;
      const uint32_t pointsSize = 8;
      const simd::vec4 trans = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
      for (uint32_t i = 0; i < pointsSize; ++i) {
        const simd::vec4 point = getVertex(pos, extent, matrix, i);
        
        const float sideLeftBorder = sideOf(A1, A2, point, attackPlane);
        const float sideRightBorder = sideOf(B1, B2, point, attackPlane);
        
        const float dotPoint = simd::dot(point, attackPlane);
        const float dotPos = simd::dot(B1, attackPlane);
        
        pointsLeft += uint32_t(sideLeftBorder < 0.0f);
        pointsRight += uint32_t(sideRightBorder >= 0.0f);
        pointsAbove += uint32_t(dotPoint - dotPos >= thickness);
        pointsBeneath += uint32_t(dotPoint - dotPos < -thickness);
      }
      
      if (pointsLeft < pointsSize && pointsRight < pointsSize && pointsAbove < pointsSize && pointsBeneath < pointsSize) {
        usrData = reinterpret_cast<PhysUserData*>(another->userData);
      }
      
//       for (uint8_t i = 0; i < 8; ++i) {
//         const simd::vec4 point = getVertex(pos, extent, matrix, i);
//         
//         const float sideLeftBorder = sideOf(A1, A2, point, attackPlane);
//         const float sideRightBorder = sideOf(B1, B2, point, attackPlane);
//         
//         const float dotPoint = simd::dot(point, attackPlane);
//         const float dotPos = simd::dot(B1, attackPlane);
//         
//         if (sideLeftBorder > 0.0f && sideRightBorder < 0.0f && std::abs(dotPoint - dotPos) < thickness) {
//           usrData = reinterpret_cast<PhysUserData*>(another->userData);
//           break;
//         }
//       }
    } else if (obj.objType.getObjType() == POLYGON_TYPE) {
      const uint32_t pointsSize = Global::physics()->getObjectShapePointsSize(another);
      const simd::vec4* points = Global::physics()->getObjectShapePoints(another);
      
      const simd::vec4 pos = obj.transformIndex == UINT32_MAX ? simd::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(obj.transformIndex).pos;
      const simd::vec4 scale = obj.transformIndex == UINT32_MAX ? simd::vec4(1.0f, 1.0f, 1.0f, 1.0f) : transforms->at(obj.transformIndex).scale;
      const simd::mat4 matrix = obj.coordinateSystemIndex == UINT32_MAX ? simd::mat4(1.0f) * simd::scale(simd::mat4(1.0f), scale) : matrices->at(obj.coordinateSystemIndex) * simd::scale(simd::mat4(1.0f), scale);
      
      // теперь учитывает, возможна проблема со знаками
      uint32_t pointsLeft = 0;
      uint32_t pointsRight = 0;
      uint32_t pointsAbove = 0;
      uint32_t pointsBeneath = 0;
      const simd::vec4 trans = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
      for (uint32_t i = 0; i < pointsSize; ++i) {
        const simd::vec4 point = transform(points[i], trans, matrix);
        
        const float sideLeftBorder = sideOf(A1, A2, point, attackPlane);
        const float sideRightBorder = sideOf(B1, B2, point, attackPlane);
        
        const float dotPoint = simd::dot(point, attackPlane);
        const float dotPos = simd::dot(B1, attackPlane);
        
        pointsLeft += uint32_t(sideLeftBorder < 0.0f);
        pointsRight += uint32_t(sideRightBorder >= 0.0f);
        pointsAbove += uint32_t(dotPoint - dotPos >= thickness);
        pointsBeneath += uint32_t(dotPoint - dotPos < -thickness);
      }
      
      if (pointsLeft < pointsSize && pointsRight < pointsSize && pointsAbove < pointsSize && pointsBeneath < pointsSize) {
        usrData = reinterpret_cast<PhysUserData*>(another->userData);
      }
      
      // нужно переделать, не учитывает ситуацию когда у нас несколько точек находятся по разные стороны, но при этом ниодна не находится внутри
      
//       for (uint32_t i = 0; i < pointsSize; ++i) {
//         const simd::vec4 point = transform(points[i], trans, matrix);
//         
//         const float sideLeftBorder = sideOf(A1, A2, point, attackPlane);
//         const float sideRightBorder = sideOf(B1, B2, point, attackPlane);
//         
//         const float dotPoint = simd::dot(point, attackPlane);
//         const float dotPos = simd::dot(B1, attackPlane);
//         
//         if (sideLeftBorder > 0.0f && sideRightBorder < 0.0f && std::abs(dotPoint - dotPos) < thickness) {
//           usrData = reinterpret_cast<PhysUserData*>(another->userData);
//           break;
//         }
//       }
    } else if (obj.objType.getObjType() == SPHERE_TYPE) {
      throw std::runtime_error("not implemented yet");
      // нужно переделать физическую сферу так или иначе
    }
    
    if (itr != objects.end() && usrData != nullptr) {
      ++itr->second.count;
      itr->second.point = std::chrono::steady_clock::now();
    } else if (itr == objects.end() && usrData != nullptr) {
      objects.insert(std::make_pair(objIndex, ObjData{0, std::chrono::steady_clock::now()}));
    }
  }
  
  return usrData;
}

InteractionComponent::InteractionComponent(const CreateInfo &info) : system(info.system), events(nullptr), trans(nullptr) {}
InteractionComponent::~InteractionComponent() {}

void InteractionComponent::update(const size_t &time) {
  for (size_t i = 0; i < interactions.size(); ++i) {
    interactions[i]->update_data({trans->pos(), trans->rot()});
    interactions[i]->update(time);
    
    PhysUserData* data = interactions[i]->get_next();
    while (data != nullptr) {
      if (data->events != nullptr) {
        const EventData ed{
          interactions[i],
          //interactions[i]->user_data() // здесь наверное и будем хранить то что нужно передать
          getEntity()
        };
        
        // нам скорее нужно сделать многопоточную функцию, чем ограничить вызов одним потоком
        // нет оставим ограничение, так гораздо проще контролировать процесс
        data->events->fireEvent_save(interactions[i]->event_type(), ed);
      }
      
      data = interactions[i]->get_next();
    }
    
    if (interactions[i]->isFinished()) {
      // когда взаимодействие заканчивается мы должны его удалить 
      // не будет ли это долго? ну память выделять ему не нужно
      // без профилера я конечно долго тут буду
      // пока что так сделаем
      deleteInteraction(interactions[i]);
      
      std::swap(interactions[i], interactions.back());
      interactions.pop_back();
      --i;
    }
  }
  
  // что с характеристиками? по идее изменения мы должны складывать отдельно чтобы потом их применить, таким образом data race не получится
  // как определить когда что нужно передавать в качестве EventData? в общем то можно положить просто указатель в Interaction и передавать это дело таким образом
  // изменения в характеристиках будет складываться отдельно (у нас еще может быть временный бонус, как с ним быть? чаще всего это некий эффект, который будет вообще-то и много других вещей делать)
  // (поэтому временные штуки тоже будут не здесь обрабатываться, как кстати накладывать эти эффекты? должен быть какой то список че и с какой силой накладывать по эвенту)
  // с эффектами еще разбираться
  
  // кажется это все
}

void InteractionComponent::init(void* userData) {
  (void)userData;
  
  events = getEntity()->get<EventComponent>().get();
  if (events == nullptr) {
    Global::console()->printE("Could not create InteractionComponent without events");
    throw std::runtime_error("Could not create InteractionComponent without events");
  }
  
  trans = getEntity()->get<TransformComponent>().get();
  if (events == nullptr) {
    Global::console()->printE("Could not create InteractionComponent without transform");
    throw std::runtime_error("Could not create InteractionComponent without transform");
  }
}

void InteractionComponent::deleteInteraction(Interaction* inter) {
  switch (inter->type()) {
    case Interaction::type::target: {
      
      break;
    }
    
    case Interaction::type::ray: {
      
      break;
    }
    
    case Interaction::type::physics: {
      PhysicsInteraction* phys = reinterpret_cast<PhysicsInteraction*>(inter);
      std::unique_lock<std::mutex> lock(creationMutex);
      physInt.deleteElement(phys);
      
      break;
    }
    
    case Interaction::type::projectile: {
      
      break;
    }
    
    case Interaction::type::aura: {
      
      break;
    }
  }
}

InteractionSystem::InteractionSystem(const CreateInfo &info) : pool(info.pool) {}
InteractionSystem::~InteractionSystem() {}

void InteractionSystem::update(const uint64_t &time) {
  static const auto func = [&] (const size_t &start, const size_t &count) {
    for (size_t i = start; i < start+count; ++i) {
      components[i]->update(time);
    }
  };
  
//   static const auto unique_pair_interaction = [&] (const size_t &index) {
//     // вообще у нас есть четкий список вещей которые мы должны сделать при таком типе взаимодействия
//     // у нас несколько типов взаимодействия: использование, атака/хил, поднятие предмета (взаимодействие с проджектайлами)
//     // и например только при атаке и проджектайлах нужно передать противнику эффекты
//     // а при использовании стоит передать например инвентарь, для того чтобы узнать есть ли у игрока ключ
//     // что это означает? что ничего кроме типа взаимодействия и энтити передавать не нужно? (тип нужные данные мы выведем из энтити)
//     // этот вариант хорош тем что мы наиболее обще подойдем к решению проблемы, но плох производительностью
//     // так еще мы собираемся вычислять это дело в одном потоке (но опять же этих вычислений будет крайне мало)
//     // что конкретно нам мешает сделать эти взаимодействия в разных потоках? нужно будет все используемые функции сделать многопоточными
//     // это проблема? скорее всего нет с учетом того что у нас будут опять же разнесены во времени вычисление урона и собственно применение этих чисел к характеристикам
//     // следовательно наверное лучше будет аккуратно сделать многопоточные функции со взаимодействием
//     
//     // с другой стороны нам нужно будет четко понимать какая функция у нас может вызваться из одного потока, а какая нет
//     // и это потенциально может привести к куче проблем
//     
//     // у нас есть возможность вызвать функцию из одного потока, скорее всего только в этом случае у нас будут и волки сыты и овцы целы
//     // + ко всему функции будут заметно быстрее
//   };
  
  pairs.clear();
  
  const size_t count = std::ceil(float(components.size()) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, components.size()-start);
    if (jobCount == 0) break;

    pool->submitnr(func, start, jobCount);

    start += jobCount;
  }
  
  pool->compute();
  pool->wait();
  
  // теперь делаем подругому
//   // увеличиваем количество Interaction Pair
//   // после находим уникальные пары
//   
//   uniquePairs();
//   
//   // либо пары уникальны внутри батча, либо батчи уникальны для каждого потока
//   
//   for (size_t i = 0; i < uniqueData.size(); ++i) {
//     for (size_t j = uniqueData[i].start; j < uniqueData[i].start+uniqueData[j].count; ++j) {
// //       const size_t count = std::ceil(float(uniqueData[j].count) / float(pool->size()+1));
// //       size_t start = 0;
// //       for (uint32_t i = 0; i < pool->size()+1; ++i) {
// //         const size_t jobCount = std::min(count, uniqueData[j].count-start);
// //         if (jobCount == 0) break;
// // 
// //         pool->submitnr(func, start, jobCount);
// // 
// //         start += jobCount;
// //       }
//       
//       pool->submitnr(unique_pair_interaction, j);
//     }
//     
//     pool->compute();
//     pool->wait();
//   }
  
  // короче я понял что поиск уникальности занимает слишком много времени и на самом деле не так уж необходим
  // как я написал в функции uniquePairs скорее всего эти взаимодействия будут последним что я вычислю в кадре
  // а значит их можно сделать в одном отдельном потоке в конце кадра во время сна, что мне потребуется?
  // опять же система в которую будут приходить собранные функции, которую затем мы будем запускать в самом конце
  // просто массив функций к выполнению похоже возможно нужно вернуть future
}

void InteractionSystem::addInteractionPair(const PairData &data) {
  std::unique_lock<std::mutex> lock(mutex);
  pairs.push_back(data);
}

struct PairCompareFunc {
  bool operator()(const InteractionSystem::PairData &first, const InteractionSystem::PairData &second) {
    return first.batchID < second.batchID;
  }
};

void InteractionSystem::uniquePairs() {
  // что тут? как найти максимально уникальную конфигурацию?
  // во первых в каждом кадре уникальных пар будет не очень много и впринципе однопоточная реализация будет не сильно проигрывать по скорости
  // во вторых поиск уникальности тоже требует вычислительных затрат
  // в третьих операция взаимодействия двух объектов это похоже что будет самой последней операцией в кадре вообще
  // в четвертых идеальную функцию поиска уникальности сделать практически невозможно (либо медленно но больше потоков параллелиться либо относительно быстро но качество параллельности под вопросом)
  
  // как то так выглядит раскидывание пар по батчам, не особо быстрое так и не особо точное
  // больше проблем чем пользы...
  
  static const auto func = [&] (std::atomic<size_t> &pairsCount, const size_t &counter, const uint32_t &index, bool* updated) {
    static thread_local std::unordered_set<uint32_t> uniqueObjects;
    
    if (pairs[index].batchID != UINT32_MAX) return;
    
    const uint32_t threadIndex = pool->thread_index(std::this_thread::get_id());
    if (!updated[threadIndex]) {
      uniqueObjects.clear();
      updated[threadIndex] = true;
    }
    
    const uint32_t &batchID = (pool->size()+1) * counter + threadIndex;
    auto itr1 = uniqueObjects.find(pairs[index].pair.first);
    auto itr2 = uniqueObjects.find(pairs[index].pair.second);
    
    if (itr1 != uniqueObjects.end() && itr2 != uniqueObjects.end()) {
      pairs[index].batchID = batchID;
      --pairsCount;
    }
  };
  
  std::atomic<size_t> pairsCount(pairs.size());
  bool* updated = new bool[pool->size()+1];
  memset(updated, 0, (pool->size()+1)*sizeof(bool));
  size_t counter = 0;
  while (pairsCount > 0) {
    for (size_t i = 0; i < pairs.size(); ++i) {
      pool->submitnr(func, std::ref(pairsCount), counter, i, updated);
    }
    
    pool->compute();
    pool->wait();
    ++counter;
    memset(updated, 0, (pool->size()+1)*sizeof(bool));
  }
  delete [] updated;
  
  std::sort(pairs.begin(), pairs.end(), PairCompareFunc());
  
  uniqueData.clear();
  
  uint32_t currentBatch = pairs[0].batchID;
  uint32_t countCurrentBatchID = 0;
  uint32_t start = 0;
  for (size_t i = 0; i < pairs.size(); ++i) {
    if (pairs[i].batchID == currentBatch) {
      ++countCurrentBatchID;
    } else {
      uniqueData.push_back({start, countCurrentBatchID});
      start = i;
      countCurrentBatchID = 1;
      currentBatch = pairs[i].batchID;
    }
  }
}

// void AttackState::setTransform(TransformComponent* trans) {
//   this->trans = trans;
// }
// 
// Slashing::Slashing(const std::string &name, 
//                    const uint64_t &attackTime, 
//                    const float &distance, 
//                    const glm::vec3 &attackPlane, 
//                    const float &angle, 
//                    bool blocking, bool blockingMovement) : AttackState(name, attackTime, distance) {
//   this->blocking = blocking;
//   this->blockingMovement = blockingMovement;
//   this->attackPlane = attackPlane;
//   this->angle = angle;
// }
// 
// Slashing::~Slashing() {}
// 
// // float sideOf(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &point, const glm::vec3 &normal) {
// //   glm::vec3 vec = b - a;
// //   vec = glm::cross(vec, normal);
// //   return glm::dot(vec, point - a);
// // }
// 
// void Slashing::update(const uint64_t &time) {
//   // на основе предыдущей точки атаки, предыдущего направления, текущей точки, текущего направления
//   // мне нужно создать Polygon
//   // насколько здесь необходима нормализация?
//   
//   if (finished) return;
//   
//   if (resetB) {
//     prevDir = glm::normalize(glm::rotate(trans->rot, -(angle / 2.0f), attackPlane));
//     prevPos = trans->pos;
//     resetB = false;
//   }
//   
//   float num = float(time) / float(attackTime);
//   float currentAngle = angle * num;
//   accumAngle += currentAngle;
//   glm::vec3 secondDir = glm::normalize(glm::rotate(trans->rot, accumAngle - (angle / 2.0f), attackPlane));
//   
//   glm::vec3 d1 = prevPos + prevDir*distance;
//   
//   Polygon p;
//   p.normal = attackPlane;
//   p.points.push_back(d1);
//   p.points.push_back(prevPos);
//   if (!vec_eq(prevPos, trans->pos)) {
//     p.points.push_back(trans->pos);
//     if (sideOf(prevPos, d1, trans->pos, attackPlane) > 0.0f) {
//       std::swap(p.points[1], p.points[2]);
//     }
//   }
//   p.points.push_back(p.points.back() + secondDir*distance);
//   
//   for (uint8_t i = 0; i < p.points.size(); ++i) {
//     PRINT_VEC3("Point "+std::to_string(i), p.points[i])
//   }
//   
// //   if (p.points.size() > 3 && sideOf(prevPos, d1, trans->pos, attackPlane) > 0.0f) {
// //     std::swap(p.points[1], p.points[2]);
// //   }
//   
//   // чекаем на пересечение этого Polygon с окружающим миром (readonly)
//   p.computeCenter();
//   Sphere s(p.barycenter, distance/2.0f);
//   std::vector<CollisionComponent*> objs;
//   std::unordered_set<CollisionComponent*> set;
//   // octree->getObjCollideSphere(&s, objs);
//   for (auto* obj : objs) {
//     size_t size = set.size();
//     set.insert(obj);
//     if (size == set.size()) continue;
//     
//     if (obj->collide(&p)) {
//       // attackFunction(trans->getEntity(), obj->getEntity());
//     }
//   }
//   // запускаем функцию, которая различным образом будет воздействовать на разные объекты (не readonly)
// 
//   prevDir = secondDir;
//   prevPos = trans->pos;
//   
//   if (accumAngle > angle) finished = true;
// }
// 
// void Slashing::reset() {
//   resetB = true;
//   accumAngle = 0.0f;
// }
// 
// bool Slashing::isFinished() const {
//   return finished;
// }
// 
// bool Slashing::isBlocking() const {
//   return blocking;
// }
// 
// bool Slashing::isBlockingMovement() const {
//   return blockingMovement;
// }
// 
// Combat::Combat() {}
// Combat::~Combat() {}
// 
// void Combat::update(const uint64_t &time) {}
// 
// void Combat::init(void* userData) {}
// 
// void Combat::addChild(Controller* c) { (void)c; }
// 
// void Combat::changeState(const Type &type) {
//   auto itr = states.find(type.getType());
//   if (itr == states.end()) {
//     current = nullptr;
//     return;
//   }
//   
//   current = itr->second;
// }
// 
// void Combat::reset() {
//   if (current != nullptr) current->reset();
// }
// 
// bool Combat::isFinished() const {
//   if (current != nullptr) return current->isFinished();
//   
//   return true;
// }
// 
// bool Combat::isBlocking() const {
//   if (current != nullptr) return current->isBlocking();
//   
//   return false;
// }
// 
// bool Combat::isBlockingMovement() const {
//   if (current != nullptr) return current->isBlockingMovement();
//   
//   return false;
// }
// 
// void Combat::addAttack(const Type &type, AttackState* attack) {
//   auto itr = states.find(type.getType());
//   if (itr != states.end()) {
//     Global::console()->printW("Entity " + std::to_string(getEntity()->getId()) + " already has got attack state " + attack->getName());
//   }
//   
//   states[type.getType()] = attack;
// }
