#include "CollisionInteractionSystem.h"

#include "Physics.h"
#include "Globals.h"
#include "UserDataComponent.h"
#include "EntityComponentSystem.h"
#include "InventoryComponent.h"
#include "Attributes.h"
#include "EffectComponent.h"
#include "AIComponent.h"
#include "global_components_indicies.h"

void pickup_item(yacs::entity* ent, InventoryComponent* entInv, yacs::entity* item, PickupItem* itemInfo, const EntityAI* itemAi) {
  auto attribs = ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX);
  auto effects = ent->at<EffectComponent>(EFFECT_COMPONENT_INDEX);
  auto second_attribs = item->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX);
  
  const auto float_finder = attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>();
  const auto int_finder = attribs->get_finder<INT_ATTRIBUTE_TYPE>();
  if (itemInfo->check_pickup(float_finder, int_finder, effects.get(), entInv)) {
    if (itemInfo->effect() != nullptr) {
      ComputedEffectContainer cec(itemInfo->effect()->baseValues().size());
      itemInfo->effect()->compute(second_attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), second_attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cec);
      itemInfo->effect()->resist(float_finder, int_finder, &cec);
      effects->addEffect(cec, itemInfo->effect(), itemAi);
    }
    
    if (itemInfo->type() != nullptr && itemInfo->quantity() != 0) {
      entInv->add(itemInfo->type(), itemInfo->quantity());
    }
  }
}

// как это распараллелить?
void CollisionInteractionSystem::update(const size_t& time) {
  const auto trigger = Global::get<PhysicsEngine>()->getTriggerPairsIndices();
  const auto pairs = Global::get<PhysicsEngine>()->getOverlappingPairsData();
  const auto size = Global::get<PhysicsEngine>()->getTriggerPairsIndicesSize();
  
  for (size_t i = 0; i < size; ++i) {
    const uint32_t index = trigger->at(i);
    const auto pair = pairs->at(index);
    auto first = Global::get<PhysicsEngine>()->getIndexContainer(pair.firstIndex);
    auto second = Global::get<PhysicsEngine>()->getIndexContainer(pair.secondIndex);
    auto firstUsrData = reinterpret_cast<UserDataComponent*>(first->userData);
    auto secondUsrData = reinterpret_cast<UserDataComponent*>(second->userData);
    yacs::entity* firstEnt = firstUsrData->entity;
    yacs::entity* secondEnt = secondUsrData->entity;
    
    // первый тип взаимодействия по коллизии - подбор айтема
    {
      auto firstInv = firstEnt->at<InventoryComponent>(INVENTORY_COMPONENT_INDEX);
      auto firstItem = firstEnt->get<PickupItem>();
      
      auto secondInv = secondEnt->at<InventoryComponent>(INVENTORY_COMPONENT_INDEX);
      auto secondItem = secondEnt->get<PickupItem>();
      
      const bool valid1 = firstInv.valid() && secondItem.valid();
      const bool valid2 = secondInv.valid() && firstItem.valid();
      if (valid1 && valid2) throw std::runtime_error("wtf");
      
      if (valid1) pickup_item(firstEnt, firstInv.get(), secondEnt, secondItem.get(), secondUsrData->aiComponent);
      if (valid2) pickup_item(secondEnt, secondInv.get(), firstEnt, firstItem.get(), firstUsrData->aiComponent);
    }
    
    // второй тип взаимодействия по коллизии - событие (?)
    {
      // что такое событие? после того как игрок сталкивается с каким то объектом
      // проиходит некое действие в мире, например наступает на нажимную плиту
      // и поднимается лифт и проч
      // как это делать? для всех этих вещей необходима какая то логика
      // то есть я должен проверить какие то вещи if-els'ами
      // я думал что можно запихать простую логику в функцию в еще одном классе ии
      // как с ней работать? что должно происходить?
      // отправлять некий заданный пользователем триггер? или просто пробуждать?
      // триггер заданый пользователем должен проверяться в самой функции или отдельно?
      // как определить кто пробудил? могут ли действия различаться?
      // скорее всего более менее адекватным действием будет отправить раздражитель (пользовательский тип)
      // и кто это сделал, пробуждать функцию и пусть она сама проверяет че за бред происходит вокруг
      // нужно учесть что пользователь может триггерить каждый кадр - то есть это не должно стать проблемой
      // нужно использовать кольцевой буфер (раздражитель, источник), как его чистить? чистить сразу при вызове
      // есть еще кнопки, с ними что? для кнопок сделать связь от кнопки к объекту, что такое связь?
      // наверное по связи нужно передать все раздражители которые приходят к другому объекту
      // связь в одну сторону, какие бывают раздражители? звук, удар, использование, коллизия
      // нужно ли передавать какие то дополнительные данные? думаю что пока незачем
      // кто должен принимать раздражитель? ии, наверное
      
      // другие раздражители приходят из других мест
      firstUsrData->aiComponent->addStimulus(Type::get("collision"), secondUsrData->aiComponent);
      secondUsrData->aiComponent->addStimulus(Type::get("collision"), firstUsrData->aiComponent);
    }
    
    // третий тип взаимодействия по коллизии - интеракция (?)
    {
      // интеракция по идее описана в другом месте (собственно в интеракциях)
      // наверное пусть лучше там останется
    }
  }
}
