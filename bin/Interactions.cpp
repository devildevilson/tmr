#include "Interactions.h"

#include "EntityComponentSystem.h"
#include "AttributesComponent.h"
#include "EffectComponent.h"
#include "UserDataComponent.h"
#include "TransformComponent.h"
#include "BroadphaseInterface.h"
#include "HelperFunctions.h"
#include "PhysicsComponent.h"

#include "EntityComponentSystem.h"
#include "Globals.h"

//const Type damage_event = Type::get("damage");

void interaction(const Type &type, const yacs::entity* ent1, yacs::entity* ent2, const Interaction* inter) {
  // для этого я вычисляю unordered_map
  const yacs::const_component_handle<AttributeComponent> enemy_attribs = ent1->get<AttributeComponent>();
  const yacs::const_component_handle<EffectComponent> enemy_effects = ent1->get<EffectComponent>();
  const auto userData = ent1->get<UserDataComponent>();

  auto attribs = ent2->get<AttributeComponent>();
  auto effects = ent2->get<EffectComponent>();

  const bool validInteraction = enemy_attribs.valid() && enemy_effects.valid() && userData.valid() && attribs.valid() && effects.valid();

  if (validInteraction) {
    // эффекты перегоняем из одного энтити другому, кажется этого достаточно
    size_t counter = 0;
    const Effect* effect = enemy_effects->getNextEventEffect(type, counter);
    while (effect != nullptr) {
      bool founded = effects->hasEffect(effect->id());
      if (founded && effect->type().timer_reset()) effects->resetEffectTimer(effect->id());

      bool add = (founded && effect->type().stackable()) || (!founded);
      if (add) {
        ComputedEffectContainer ret(effect->baseValues().size());
        effect->compute(enemy_attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), enemy_attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &ret);
        effect->resist(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &ret);

        effects->addEffect(ret, effect, userData->aiComponent); // синхронизация нужна здесь
      }

      effect = enemy_effects->getNextEventEffect(type, counter);
    }
  }

  // аттрибуты нужны для вычисления урона, нужно ли для этого что то еще?
  // будем вычислять так же как и эффекты

  // как сделать частицы и декали?

  // мы здесь берем компоненты, и делаем примерно то же самое

  // как сделать дебаг отрисовку например? можно найти компонент в нем вызвать функцию
  // но как теперь не рисовать в других местах? нужно как то запоминать стейт (добавляем в массив, смотрим потом какой цвет, массив чистим)
  // индекс то у нас остается, что с ним делать? индекс обнуляем когда обходим вещи второй раз, как то вот так

  if (inter->type() == Interaction::type::ray) {
    // дебаг

  }
}

TargetInteraction::TargetInteraction(const CreateInfo &info) : Interaction(Interaction::type::target, info.event), delayTime(info.delayTime), currentTime(0), obj(info.obj), target(info.target) {}
TargetInteraction::~TargetInteraction() {}

void TargetInteraction::update(const size_t &time) {
  currentTime += time;

  if (currentTime >= delayTime) {
    interaction(event_type(), obj, target, this);
    finished = true;
  }
}

RayInteraction::RayInteraction(const CreateInfo &info)
  : Interaction(Interaction::type::ray, info.event),
    interaction_type(info.interaction_type),
    rayIndex(UINT32_MAX),
    lastPos{0.0f, 0.0f, 0.0f, 0.0f},
    lastDir{0.0f, 0.0f, 0.0f, 0.0f},
//    pos{info.pos[0], info.pos[1], info.pos[2], info.pos[3]},
//    dir{info.dir[0], info.dir[1], info.dir[2], info.dir[3]},
    maxDist(info.maxDist),
    minDist(info.minDist),
    ignoreObj(info.ignoreObj),
    filter(info.filter),
    obj(info.obj),
    transform(info.transform),
    currentTime(0),
    delayTime(info.delayTime),
    state(0) {
  transform->pos().storeu(lastPos);
  transform->rot().storeu(lastDir);
}

RayInteraction::~RayInteraction() {}

void RayInteraction::update(const size_t &time) {
  // в текущем виде лучи будут опаздывать на один кадр, нормально ли это?
  // как сделать так чтобы они не опаздывали?
  // либо пускать их каждый кадр (например в небольшом промежутке)

  currentTime += time;

  if (currentTime >= delayTime) {
    if (rayIndex != UINT32_MAX) {
      // обходим энтити
      // что делать в конце? нужно ли пересоздавать лучики?
      // в предыдущем варианте было легче так как я удалял интеракцию сразу после конца
      // здесь же я пока еще не уверен в правильном механизме удаления всего этого дела
      // но явно должно быть примерно тоже самое, я должен узнавать о конце и добавлять в очередь к удалению

      const uint32_t rayDataCount = Global::physics()->getRayTracingSize();
      const auto rayData = Global::physics()->getRayTracingData();

      switch (interaction_type) {
        case RayInteraction::type::first_min: {
          for (uint32_t i = 0; i < rayDataCount; ++i) {
            const OverlappingData &data = rayData->at(i);
            if (data.firstIndex != rayIndex) continue;

            const PhysicsIndexContainer* another = Global::physics()->getIndexContainer(data.secondIndex);
            auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);
            interaction(event_type(), obj, usrData->entity, this);

            break;
          }

          break;
        }
        case RayInteraction::type::first_max: {
          for (int32_t i = rayDataCount-1; i >= 0; --i) {
            const OverlappingData &data = rayData->at(i);
            if (data.firstIndex != rayIndex) continue;

            const PhysicsIndexContainer* another = Global::physics()->getIndexContainer(data.secondIndex);
            auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);
            interaction(event_type(), obj, usrData->entity, this);

            break;
          }
        }
        case RayInteraction::type::any: {
          for (uint32_t i = 0; i < rayDataCount; ++i) {
            const OverlappingData &data = rayData->at(i);
            if (data.firstIndex != rayIndex) continue;

            const PhysicsIndexContainer* another = Global::physics()->getIndexContainer(data.secondIndex);
            auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);
            interaction(event_type(), obj, usrData->entity, this);
            // как бы сделать через это еще и дебаг отрисовку
            // то есть здесь мы чудесно находим все объекты пересекающие наш луч видимости
            // нам просто по идее нужно произвольную дополнительную функцию запустить
            // + понять какой объект ближайший
            // передать в интеракцию индекс объекта?

            // у меня же тип есть, first_min - вот решение
          }
        }
      }

      // в общем по аналогии с аурами, мы просто эту интеракцию держим дольше, то есть пересоздаем в конце луч

      const RayData ray(transform->pos(), transform->rot(), ignoreObj, filter);
      rayIndex = Global::physics()->add(ray);
      finished = true;
    }

    if (rayIndex == UINT32_MAX) {
      const RayData ray(transform->pos(), transform->rot(), ignoreObj, filter);
      rayIndex = Global::physics()->add(ray);
    }
  }

//  memcpy(lastPos, pos, sizeof(simd::vec4));
//  memcpy(lastDir, dir, sizeof(simd::vec4));
//  transform->pos().storeu(pos);
//  transform->rot().storeu(dir);

  transform->pos().storeu(lastPos);
  transform->rot().storeu(lastDir);

  // это нужно чтоб потом была возможность интерполировать
  // но только в основном нужно только предыдущее значение позиции
}

SlashingInteraction::SlashingInteraction(const CreateInfo &info)
  : Interaction(Interaction::type::slashing, info.eventType),
    delay(info.delayTime),
    attackTime(info.attackTime),
    lastTime(0),
    currentTime(0),
    tickTime(info.tickTime),
//    transformIndex(info.transformIndex),
    //transformIndex(UINT32_MAX),
    tickCount(info.tickCount),
    ticklessObjectsType(info.ticklessObjectsType),
    thickness(info.thickness),
    attackAngle(info.attackAngle),
    distance(info.distance),
    attackSpeed(info.attackSpeed),
    obj(info.obj),
    phys(info.phys),
    transform(info.transform),
    pos{0.0f, 0.0f, 0.0f, 0.0f},
    dir{0.0f, 0.0f, 0.0f, 0.0f},
//    lastPos{info.pos[0], info.pos[1], info.pos[2], info.pos[3]},
//    lastDir{info.dir[0], info.dir[1], info.dir[2], info.dir[3]},
    lastPos{0.0f, 0.0f, 0.0f, 0.0f},
    lastDir{0.0f, 0.0f, 0.0f, 0.0f},
    plane{info.plane[0], info.plane[1], info.plane[2], info.plane[3]} {
//  if (transformIndex == UINT32_MAX) transformIndex = transforms->insert({simd::vec4(info.pos), simd::vec4(info.dir), simd::vec4()});

//  container = {
//    UINT32_MAX,
//    UINT32_MAX,
//    UINT32_MAX,
//    UINT32_MAX,
//    UINT32_MAX,
//    UINT32_MAX,
//    nullptr
//  };
//
//  // создаем сферу
//  const PhysicsObjectCreateInfo phys_info{
//    false,
//    PhysicsType(false, SPHERE_TYPE, false, true, false, false),
//    info.sphereCollisionGroup,
//    info.sphereCollisionFilter,
//    0.0f,
//    0.0f,
//    0.0f,
//    distance,
//    UINT32_MAX,
//    transform->index(),
//    UINT32_MAX,
////     info.matrixIndex,
////     info.rotationIndex,
//    UINT32_MAX,
//    UINT32_MAX,
//    Type()
//  };
//
//  // понятное дело нужно как то обезопасить создание, в принципе если будет внешний мьютекс, то норм
//  Global::physics()->add(phys_info, &container);

  transform->pos().storeu(pos);
  transform->rot().storeu(dir);
  transform->pos().storeu(lastPos);
  transform->rot().storeu(lastDir);
}

SlashingInteraction::~SlashingInteraction() {
//  Global::physics()->remove(&container);
//  transforms->erase(transformIndex); // нужно понять откуда у нас этот индекс
  // лучше сделаю по другому
}

void SlashingInteraction::update(const size_t &time) {
  currentTime += time;

  if (currentTime >= delay) {
    if (currentTime >= delay + attackTime) finished = true;

    const uint32_t triggerSize = Global::physics()->getTriggerPairsIndicesSize();
    const ArrayInterface<uint32_t>* triggerPairs = Global::physics()->getTriggerPairsIndices();
    const ArrayInterface<OverlappingData>* datas = Global::physics()->getOverlappingPairsData();

    const size_t finalCurrentTime = currentTime - delay;
    const size_t finalLastTime = lastTime > delay ? lastTime - delay : 0;
    // найдем 4 точки, 2 из которых это текущее положение и предыдущее
    // + 2 точки предыдущий дир повернутый на предыдущий угол, и текущий дир повернутый на текущий угол
    // плоскость поворота normal
    const float halfAngle = attackAngle / 2.0f;
    const float currentAngle = (finalCurrentTime * (attackAngle / float(attackTime))) - halfAngle;
    const float lastAngle = (finalLastTime * (attackAngle / float(attackTime))) - halfAngle;

    // TODO: атака должна быть в две стороны

    const simd::vec4 simd_pos = simd::vec4(pos);
    const simd::vec4 simd_dir = simd::vec4(dir);
    //const simd::mat4 attackRot = simd::orientation(simd_dir, simd::normalize(projectVectorOnPlane(Global::physics()->getGravityNorm(), simd_pos, simd_dir)));
    const simd::mat4 attackRot = simd::orientation(simd_dir, simd::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    const simd::vec4 attackPlane = attackRot * PhysicsEngine::getOrientation() * simd::vec4(plane);
    const simd::vec4 vector1 = simd::normalize(simd::rotate(simd::mat4(1.0f), currentAngle, attackPlane) * simd_dir);
    const simd::vec4 vector2 = simd::normalize(simd::rotate(simd::mat4(1.0f), lastAngle, attackPlane) * simd::vec4(lastDir));

    // TODO: должна быть проверка на то что сами точки находятся слева/справа
    // то есть если мы атакуем в иную сторону чем двигается камера
    const simd::vec4 A1 = simd::vec4(lastPos);
    const simd::vec4 A2 = A1 + vector2 * distance;
    const simd::vec4 B1 = simd_pos;
    const simd::vec4 B2 = B1 + vector1 * distance;

    auto container = phys->getIndexContainer();

    for (uint32_t i = 0; i < triggerSize; ++i) {
      const uint32_t index = triggerPairs->at(i+1); // TODO: +1?
      const OverlappingData &data = datas->at(index);

      if (data.firstIndex != container.objectDataIndex && data.secondIndex != container.objectDataIndex) continue;

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

      bool founded = false;
      const Object &anotherObj = Global::physics()->getObjectData(another);
      if (anotherObj.objType.getObjType() == BBOX_TYPE) {
        const simd::vec4 pos = transforms->at(anotherObj.transformIndex).pos;
        const simd::vec4 extent = Global::physics()->getObjectShapePoints(another)[0];
        const simd::vec4 scale = transforms->at(anotherObj.transformIndex).scale;

        const simd::mat4 matrix = anotherObj.coordinateSystemIndex == UINT32_MAX ? simd::mat4(1.0f) * simd::scale(simd::mat4(1.0f), scale) : matrices->at(anotherObj.coordinateSystemIndex) * simd::scale(simd::mat4(1.0f), scale);

        // теперь учитывает, возможна проблема со знаками
        uint32_t pointsLeft = 0;
        uint32_t pointsRight = 0;
        uint32_t pointsAbove = 0;
        uint32_t pointsBeneath = 0;
        const uint32_t pointsSize = 8;
        const simd::vec4 trans = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        for (uint32_t j = 0; j < pointsSize; ++j) {
          const simd::vec4 point = getVertex(pos, extent, matrix, j);

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
          // мы нашли объект
          // вообще у нас уже есть юзер дата компонент, может как-то его использовать? там пока что хранится не очень много полезных штук
          auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);

          interaction(event_type(), obj, usrData->entity, this);
          founded = true;
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
      } else if (anotherObj.objType.getObjType() == POLYGON_TYPE) {
        const uint32_t pointsSize = Global::physics()->getObjectShapePointsSize(another);
        const simd::vec4* points = Global::physics()->getObjectShapePoints(another);

        const simd::vec4 pos = anotherObj.transformIndex == UINT32_MAX ? simd::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(anotherObj.transformIndex).pos;
        const simd::vec4 scale = anotherObj.transformIndex == UINT32_MAX ? simd::vec4(1.0f, 1.0f, 1.0f, 1.0f) : transforms->at(anotherObj.transformIndex).scale;
        const simd::mat4 matrix = anotherObj.coordinateSystemIndex == UINT32_MAX ? simd::mat4(1.0f) * simd::scale(simd::mat4(1.0f), scale) : matrices->at(anotherObj.coordinateSystemIndex) * simd::scale(simd::mat4(1.0f), scale);

        // теперь учитывает, возможна проблема со знаками
        uint32_t pointsLeft = 0;
        uint32_t pointsRight = 0;
        uint32_t pointsAbove = 0;
        uint32_t pointsBeneath = 0;
        const simd::vec4 trans = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        for (uint32_t j = 0; j < pointsSize; ++j) {
          const simd::vec4 point = ::transform(points[j], trans, matrix);

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
          auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);

          interaction(event_type(), obj, usrData->entity, this);
          founded = true;
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
      } else if (anotherObj.objType.getObjType() == SPHERE_TYPE) {
        throw std::runtime_error("not implemented yet");
        // нужно переделать физическую сферу так или иначе
        // переделал уже
      }

      if (itr != objects.end() && founded) {
        ++itr->second.count;
        itr->second.point = std::chrono::steady_clock::now();
      } else if (itr == objects.end() && founded) {
        objects.insert(std::make_pair(objIndex, ObjData{0, std::chrono::steady_clock::now()}));
      }
    }
  }

  memcpy(lastPos, pos, sizeof(simd::vec4));
  memcpy(lastDir, dir, sizeof(simd::vec4));
  transform->pos().storeu(pos);
  transform->rot().storeu(dir);
}

StabbingInteraction::StabbingInteraction(const CreateInfo &info)
  : Interaction(Interaction::type::stabbing, info.eventType),
    delayTime(info.delayTime),
    attackTime(info.attackTime),
    currentTime(0),
    lastTime(0),
    tickTime(info.tickTime),
    tickCount(info.tickCount),
    ticklessObjectsType(info.ticklessObjectsType),
    thickness(info.thickness),
    minDistance(info.minDistance),
    maxDistance(info.maxDistance),
    attackSpeed(info.attackSpeed),
    stabAngle(info.stabAngle),
    obj(info.obj),
    phys(info.phys),
    transform(info.transform) {}

StabbingInteraction::~StabbingInteraction() {
//  Global::physics()->remove(&container);
}

void StabbingInteraction::update(const size_t &time) {
  // пока что непонятно как точно обеспечить именно эффект атаки вперед
  // возможно нужно сделать все точно так же как и в slash
  // полагаю, что это проще всего

  currentTime += time;
  if (currentTime >= delayTime) {
    if (currentTime >= delayTime + attackTime) finished = true;

    auto container = phys->getIndexContainer();

    const size_t updateTime = currentTime - delayTime;
    const float diff = maxDistance - minDistance;
    const float inc = std::min((float(updateTime) / float(attackTime)) * diff, diff);
    const float currentRadius = glm::uintBitsToFloat(Global::physics()->getObjectData(&container).faceCount);

    const simd::vec4 simd_pos = transform->pos();
    const simd::vec4 simd_dir = transform->rot();

    const simd::mat4 attackRot = simd::orientation(simd_dir, simd::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    const simd::vec4 attackPlane = attackRot * PhysicsEngine::getOrientation() * simd::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    // здесь нам нужно три точки: позиция + две точки на сфере
    const float halfAngle = stabAngle / 2.0f;
    const simd::vec4 vector1 = simd::normalize(simd::rotate(simd::mat4(1.0f),  halfAngle, attackPlane) * simd_dir);
    const simd::vec4 vector2 = simd::normalize(simd::rotate(simd::mat4(1.0f), -halfAngle, attackPlane) * simd_dir);

    const simd::vec4 A = simd_pos;
    const simd::vec4 B = A + vector1 * currentRadius;
    const simd::vec4 C = A + vector2 * currentRadius;

    const uint32_t triggerSize = Global::physics()->getTriggerPairsIndicesSize();
    const ArrayInterface<uint32_t>* triggerPairs = Global::physics()->getTriggerPairsIndices();
    const ArrayInterface<OverlappingData>* datas = Global::physics()->getOverlappingPairsData();
    for (uint32_t i = 0; i < triggerSize; ++i) {
      const uint32_t index = triggerPairs->at(i+1);
      const OverlappingData &data = datas->at(index);

      if (data.firstIndex != container.objectDataIndex && data.secondIndex != container.objectDataIndex) continue;

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

      bool founded = false;
      const Object &anotherObj = Global::physics()->getObjectData(another);
      if (anotherObj.objType.getObjType() == BBOX_TYPE) {
        const simd::vec4 pos = transforms->at(anotherObj.transformIndex).pos;
        const simd::vec4 extent = Global::physics()->getObjectShapePoints(another)[0];
        const simd::vec4 scale = transforms->at(anotherObj.transformIndex).scale;

        const simd::mat4 matrix = anotherObj.coordinateSystemIndex == UINT32_MAX ? simd::mat4(1.0f) * simd::scale(simd::mat4(1.0f), scale) : matrices->at(anotherObj.coordinateSystemIndex) * simd::scale(simd::mat4(1.0f), scale);

        // теперь учитывает, возможна проблема со знаками
        uint32_t pointsLeft = 0;
        uint32_t pointsRight = 0;
        uint32_t pointsAbove = 0;
        uint32_t pointsBeneath = 0;
        const uint32_t pointsSize = 8;
        const simd::vec4 trans = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        for (uint32_t j = 0; j < pointsSize; ++j) {
          const simd::vec4 point = getVertex(pos, extent, matrix, j);

          const float sideLeftBorder = sideOf(A, B, point, attackPlane);
          const float sideRightBorder = sideOf(A, C, point, attackPlane);

          const float dotPoint = simd::dot(point, attackPlane);
          const float dotPos = simd::dot(A, attackPlane);

          pointsLeft += uint32_t(sideLeftBorder < 0.0f);
          pointsRight += uint32_t(sideRightBorder >= 0.0f);
          pointsAbove += uint32_t(dotPoint - dotPos >= thickness);
          pointsBeneath += uint32_t(dotPoint - dotPos < -thickness);
        }

        if (pointsLeft < pointsSize && pointsRight < pointsSize && pointsAbove < pointsSize && pointsBeneath < pointsSize) {
          // мы нашли объект
          // вообще у нас уже есть юзер дата компонент, может как-то его использовать? там пока что хранится не очень много полезных штук
          auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);

          interaction(event_type(), obj, usrData->entity, this);
          founded = true;
        }
      } else if (anotherObj.objType.getObjType() == POLYGON_TYPE) {
        const uint32_t pointsSize = Global::physics()->getObjectShapePointsSize(another);
        const simd::vec4* points = Global::physics()->getObjectShapePoints(another);

        const simd::vec4 pos = anotherObj.transformIndex == UINT32_MAX ? simd::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(anotherObj.transformIndex).pos;
        const simd::vec4 scale = anotherObj.transformIndex == UINT32_MAX ? simd::vec4(1.0f, 1.0f, 1.0f, 1.0f) : transforms->at(anotherObj.transformIndex).scale;
        const simd::mat4 matrix = anotherObj.coordinateSystemIndex == UINT32_MAX ? simd::mat4(1.0f) * simd::scale(simd::mat4(1.0f), scale) : matrices->at(anotherObj.coordinateSystemIndex) * simd::scale(simd::mat4(1.0f), scale);

        // теперь учитывает, возможна проблема со знаками
        uint32_t pointsLeft = 0;
        uint32_t pointsRight = 0;
        uint32_t pointsAbove = 0;
        uint32_t pointsBeneath = 0;
        const simd::vec4 trans = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        for (uint32_t j = 0; j < pointsSize; ++j) {
          const simd::vec4 point = ::transform(points[j], trans, matrix);

          const float sideLeftBorder = sideOf(A, B, point, attackPlane);
          const float sideRightBorder = sideOf(A, C, point, attackPlane);

          const float dotPoint = simd::dot(point, attackPlane);
          const float dotPos = simd::dot(A, attackPlane);

          pointsLeft += uint32_t(sideLeftBorder < 0.0f);
          pointsRight += uint32_t(sideRightBorder >= 0.0f);
          pointsAbove += uint32_t(dotPoint - dotPos >= thickness);
          pointsBeneath += uint32_t(dotPoint - dotPos < -thickness);
        }

        if (pointsLeft < pointsSize && pointsRight < pointsSize && pointsAbove < pointsSize && pointsBeneath < pointsSize) {
          auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);

          interaction(event_type(), obj, usrData->entity, this);
          founded = true;
        }
      } else if (anotherObj.objType.getObjType() == SPHERE_TYPE) {
        throw std::runtime_error("not implemented yet");
        // нужно переделать физическую сферу так или иначе
        // переделал уже
      }

      if (itr != objects.end() && founded) {
        ++itr->second.count;
        itr->second.point = std::chrono::steady_clock::now();
      } else if (itr == objects.end() && founded) {
        objects.insert(std::make_pair(objIndex, ObjData{0, std::chrono::steady_clock::now()}));
      }
    }

    const float nextRadius = minDistance + inc;
    Global::physics()->getObjectData(&container).faceCount = glm::floatBitsToUint(nextRadius);
  }
}

//AuraInteraction::AuraInteraction(const CreateInfo &info);
//AuraInteraction::~AuraInteraction();
//
//void AuraInteraction::update(const size_t &time) override;

ImpactInteraction::ImpactInteraction(const CreateInfo &info) : Interaction(Interaction::type::impact, info.eventType), delayTime(info.delayTime), currentTime(0), obj(info.obj), transform(info.transform) {}
ImpactInteraction::~ImpactInteraction() {}

void ImpactInteraction::update(const size_t &time) {
  currentTime += time;

  if (currentTime >= delayTime) {
    const uint32_t triggerSize = Global::physics()->getTriggerPairsIndicesSize();
    const ArrayInterface<uint32_t>* triggerPairs = Global::physics()->getTriggerPairsIndices();
    const ArrayInterface<OverlappingData>* datas = Global::physics()->getOverlappingPairsData();

    auto container = phys->getIndexContainer();

    for (uint32_t i = 0; i < triggerSize; ++i) {
      const uint32_t index = triggerPairs->at(i + 1);
      const OverlappingData &data = datas->at(index);

      if (data.firstIndex != container.objectDataIndex && data.secondIndex != container.objectDataIndex) continue;

      const uint32_t objIndex = data.firstIndex == container.objectDataIndex ? data.secondIndex : data.firstIndex;
      const PhysicsIndexContainer* another = Global::physics()->getIndexContainer(objIndex);
      auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);

      interaction(event_type(), obj, usrData->entity, this);
    }
  }
}

InteractionSystem::InteractionSystem(const CreateInfo &info) : pool(info.pool) {}
void InteractionSystem::update(const size_t &time) {
  static const auto targetFunc = [&] (const size_t &time) {
    const size_t count = Global::world()->count_components<TargetInteraction>();
    for (size_t i = 0; i < count; ++i) {
      auto handle = Global::world()->get_component<TargetInteraction>(i);
      handle->update(time);
    }
  };
  
  static const auto rayFunc = [&] (const size_t &time) {
    const size_t count = Global::world()->count_components<RayInteraction>();
    for (size_t i = 0; i < count; ++i) {
      auto handle = Global::world()->get_component<RayInteraction>(i);
      handle->update(time);
    }
  };
  
  static const auto slashFunction = [&] (const size_t &start, const size_t &count, const size_t &time) {
    for (size_t i = start; i < start+count; ++i) {
      auto handle = Global::world()->get_component<SlashingInteraction>(i);
      handle->update(time);
    }
  };
  
  static const auto stabFunction = [&] (const size_t &start, const size_t &count, const size_t &time) {
    for (size_t i = start; i < start+count; ++i) {
      auto handle = Global::world()->get_component<StabbingInteraction>(i);
      handle->update(time);
    }
  };
  
  static const auto impactFunction = [&] (const size_t &start, const size_t &count, const size_t &time) {
    for (size_t i = start; i < start+count; ++i) {
      auto handle = Global::world()->get_component<ImpactInteraction>(i);
      handle->update(time);
    }
  };
  
  pool->submitnr(targetFunc, time);
  pool->submitnr(rayFunc, time);
  
  {
    const size_t &componentsCount = Global::world()->count_components<SlashingInteraction>();
    const size_t count = std::ceil(float(componentsCount) / float(pool->size()+1));
    size_t start = 0;
    for (uint32_t i = 0; i < pool->size()+1; ++i) {
      const size_t jobCount = std::min(count, componentsCount-start);
      if (jobCount == 0) break;

      pool->submitnr(slashFunction, start, jobCount, time);

      start += jobCount;
    }
  }
  
  {
    const size_t &componentsCount = Global::world()->count_components<StabbingInteraction>();
    const size_t count = std::ceil(float(componentsCount) / float(pool->size()+1));
    size_t start = 0;
    for (uint32_t i = 0; i < pool->size()+1; ++i) {
      const size_t jobCount = std::min(count, componentsCount-start);
      if (jobCount == 0) break;

      pool->submitnr(stabFunction, start, jobCount, time);

      start += jobCount;
    }
  }
  
  {
    const size_t &componentsCount = Global::world()->count_components<ImpactInteraction>();
    const size_t count = std::ceil(float(componentsCount) / float(pool->size()+1));
    size_t start = 0;
    for (uint32_t i = 0; i < pool->size()+1; ++i) {
      const size_t jobCount = std::min(count, componentsCount-start);
      if (jobCount == 0) break;

      pool->submitnr(impactFunction, start, jobCount, time);

      start += jobCount;
    }
  }
  
  pool->compute();
  pool->wait();
}
