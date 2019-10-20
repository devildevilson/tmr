#include "AttributesComponent.h"

#include "EventComponent.h"
#include "PhysicsComponent.h"
#include "Globals.h"

void AttributeComponent::setContainer(Container<ExternalData>* cont) {
  AttributeComponent::externalDatas = cont;
}

AttributeComponent::AttributeComponent(const CreateInfo &info) : fcount(info.float_attribs.size()), icount(info.int_attribs.size()), attribsf(nullptr), attribsi(nullptr), phys(info.phys), events(info.events), updateTime(info.updateTime), currentTime(SIZE_MAX) {
  attribsf = fcount > 0 ? new Attribute<FLOAT_ATTRIBUTE_TYPE>[fcount] : nullptr;
  attribsi = icount > 0 ? new Attribute<INT_ATTRIBUTE_TYPE>[icount] : nullptr;
  
  for (size_t i = 0; i < fcount; ++i) {
    switch (attribsf[i].type().data_type()) {
      case ATTRIBUTE_CURRENT_SPEED: {
        attribsf[i].setType(info.float_attribs[i].type);
        attribsf[i].setBase(0.0f);
        attribsf[i].setValue(0.0f);
        break;
      }
      
      case ATTRIBUTE_UPDATE_EXTERNAL_DATA_MAX_SPEED: {
        attribsf[i].setType(info.float_attribs[i].type);
        attribsf[i].setBase(0.0f);
        break;
      }
      
      case ATTRIBUTE_UPDATE_EXTERNAL_DATA_ACCELERATION: {
        attribsf[i].setType(info.float_attribs[i].type);
        attribsf[i].setBase(0.0f);
        break;
      }
      
      case ATTRIBUTE_DATA_TYPE_NONE:
      case ATTRIBUTE_DATA_TYPE_COUNT:
        attribsf[i].setType(info.float_attribs[i].type);
        attribsf[i].setBase(info.float_attribs[i].baseValue);
    }
  }
  
  for (size_t i = 0; i < icount; ++i) {
    attribsi[i].setType(info.int_attribs[i].type);
    attribsi[i].setBase(info.int_attribs[i].baseValue);
  }
}

AttributeComponent::~AttributeComponent() {
  if (attribsf != nullptr) delete [] attribsf;
  if (attribsi != nullptr) delete [] attribsi;
}

struct AttributeReactionPointers {
  AttributeReaction* reaction;
  void* attribute;
};

void AttributeComponent::update() {
  static const std::function<void(Attribute<FLOAT_ATTRIBUTE_TYPE>*, const Bonus &)> float_attrib_funcs[ATTRIB_BONUS_TYPE_COUNT] = {
    [] (Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib, const Bonus &bonus) {
      attrib->addBonus(bonus);
    },
    [] (Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib, const Bonus &bonus) {
      attrib->removeBonus(bonus);
    },
    [] (Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib, const Bonus &bonus) {
      attrib->addFinalBonus(bonus);
    },
    [] (Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib, const Bonus &bonus) {
      attrib->removeFinalBonus(bonus);
    }
  };

  static const std::function<void(Attribute<INT_ATTRIBUTE_TYPE>*, const Bonus &)> int_attrib_funcs[ATTRIB_BONUS_TYPE_COUNT] = {
    [] (Attribute<INT_ATTRIBUTE_TYPE>* attrib, const Bonus &bonus) {
      attrib->addBonus(bonus);
    },
    [] (Attribute<INT_ATTRIBUTE_TYPE>* attrib, const Bonus &bonus) {
      attrib->removeBonus(bonus);
    },
    [] (Attribute<INT_ATTRIBUTE_TYPE>* attrib, const Bonus &bonus) {
      attrib->addFinalBonus(bonus);
    },
    [] (Attribute<INT_ATTRIBUTE_TYPE>* attrib, const Bonus &bonus) {
      attrib->removeFinalBonus(bonus);
    }
  };

  static const std::function<void(EventComponent*, AttributeComponent*, Attribute<FLOAT_ATTRIBUTE_TYPE>*, AttributeReaction*)> float_attrib_reactions[static_cast<uint32_t>(AttributeReaction::comparison::count)] = {
    [] (EventComponent* events, AttributeComponent* comp, Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib, AttributeReaction* reaction) {
      if (attrib->value() < reaction->value) {
        AttributeReactionPointers p{
          reaction,
          attrib
        };

        const EventData ed{
          comp,
          &p
        };
        events->fireEvent(reaction->event, ed);
      }
    },
    [] (EventComponent* events, AttributeComponent* comp, Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib, AttributeReaction* reaction) {
      if (attrib->value() > reaction->value) {
        AttributeReactionPointers p{
          reaction,
          attrib
        };

        const EventData ed{
          comp,
          &p
        };
        events->fireEvent(reaction->event, ed);
      }
    },
    [] (EventComponent* events, AttributeComponent* comp, Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib, AttributeReaction* reaction) {
      if (f_eq(attrib->value(), reaction->value)) {
        AttributeReactionPointers p{
          reaction,
          attrib
        };

        const EventData ed{
          comp,
          &p
        };
        events->fireEvent(reaction->event, ed);
      }
    }
  };

  static const std::function<void(EventComponent*, AttributeComponent*, Attribute<INT_ATTRIBUTE_TYPE>*, AttributeReaction*)> int_attrib_reactions[static_cast<uint32_t>(AttributeReaction::comparison::count)] = {
    [] (EventComponent* events, AttributeComponent* comp, Attribute<INT_ATTRIBUTE_TYPE>* attrib, AttributeReaction* reaction) {
      if (attrib->value() < reaction->value) {
        AttributeReactionPointers p{
          reaction,
          attrib
        };

        const EventData ed{
          comp,
          &p
        };
        events->fireEvent(reaction->event, ed);
      }
    },
    [] (EventComponent* events, AttributeComponent* comp, Attribute<INT_ATTRIBUTE_TYPE>* attrib, AttributeReaction* reaction) {
      if (attrib->value() > reaction->value) {
        AttributeReactionPointers p{
          reaction,
          attrib
        };

        const EventData ed{
          comp,
          &p
        };
        events->fireEvent(reaction->event, ed);
      }
    },
    [] (EventComponent* events, AttributeComponent* comp, Attribute<INT_ATTRIBUTE_TYPE>* attrib, AttributeReaction* reaction) {
      if (f_eq(attrib->value(), reaction->value)) {
        AttributeReactionPointers p{
          reaction,
          attrib
        };

        const EventData ed{
          comp,
          &p
        };
        events->fireEvent(reaction->event, ed);
      }
    }
  };

  if (currentTime < updateTime) return;
  
  AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> float_finder(fcount, attribsf);
  AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> int_finder(icount, attribsi);

  for (size_t i = 0; i < datas.size(); ++i) {
    if (datas[i].attribType.float_type()) {
      const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_float_type();
      Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib = float_finder.find(type);

      float_attrib_funcs[datas[i].type](attrib, datas[i].b);
    } else {
      const AttributeType<INT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_int_type();
      Attribute<INT_ATTRIBUTE_TYPE>* attrib = int_finder.find(type);

      int_attrib_funcs[datas[i].type](attrib, datas[i].b);
    }
  }

  for (size_t i = 0; i < fcount; ++i) {
    switch (attribsf[i].type().data_type()) {
      case ATTRIBUTE_CURRENT_SPEED: {
        if (phys != nullptr) break;
        attribsf[i].setBase(phys->getSpeed());
        attribsf[i].setValue(phys->getSpeed());
        break;
      }

      case ATTRIBUTE_UPDATE_EXTERNAL_DATA_MAX_SPEED: {
        if (phys != nullptr) break;
        attribsf[i].calculate(float_finder, int_finder);
        const uint32_t index = phys->getExternalDataIndex();
        externalDatas->at(index).maxSpeed = attribsf[i].value();
        break;
      }

      case ATTRIBUTE_UPDATE_EXTERNAL_DATA_ACCELERATION: {
        if (phys != nullptr) break;
        attribsf[i].calculate(float_finder, int_finder);
        const uint32_t index = phys->getExternalDataIndex();
        externalDatas->at(index).acceleration = attribsf[i].value();
        break;
      }

      case ATTRIBUTE_DATA_TYPE_NONE:
      case ATTRIBUTE_DATA_TYPE_COUNT:
        attribsf[i].calculate(float_finder, int_finder);
    }
  }

  for (size_t i = 0; i < icount; ++i) {
    attribsi[i].calculate(float_finder, int_finder);
  }

  // как сделать смерть? мы должны вызвать какой то эвент, когда определенный аттрибут достигает определенного значения
  // то есть массив типов с неким значением, после вычисления всех аттрибутов мы обходим массив
  // должно ли это быть внутри типа или внутри энтити? можно создавать типы
  // как сделать отрывание частей тел как в брутал думе? по идее нам для этого нужно только 4 вещи: позиции игрока и монстра, направление взгляда игрока и монстра
  // смерть по идее должна добавлять задачи в delayed work system

  for (size_t i = 0; i < reactions.size(); ++i) {
    if (reactions[i].attribType.float_type()) {
      const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_float_type();
      Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib = float_finder.find(type);

      float_attrib_reactions[static_cast<uint32_t>(reactions[i].comp)](events, this, attrib, &reactions[i]);
    } else {
      const AttributeType<INT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_int_type();
      Attribute<INT_ATTRIBUTE_TYPE>* attrib = int_finder.find(type);

      int_attrib_reactions[static_cast<uint32_t>(reactions[i].comp)](events, this, attrib, &reactions[i]);
    }
  }

  // сделано специально, чтобы аттрибуты обновлялись два кадра
  if (datas.empty()) currentTime = 0;

  datas.clear();
}

template <>
const Attribute<FLOAT_ATTRIBUTE_TYPE>* AttributeComponent::get(const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type) const {
  return AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>(fcount, attribsf).find(type);
}

template <>
const Attribute<INT_ATTRIBUTE_TYPE>* AttributeComponent::get(const AttributeType<INT_ATTRIBUTE_TYPE> &type) const {
  return AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>(icount, attribsi).find(type);
}

template <>
const Attribute<FLOAT_ATTRIBUTE_TYPE>* AttributeComponent::get(const TypelessAttributeType &type) const {
  if (!type.float_type()) return nullptr;
  
  return AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>(fcount, attribsf).find(type.get_float_type());
}

template <>
const Attribute<INT_ATTRIBUTE_TYPE>* AttributeComponent::get(const TypelessAttributeType &type) const {
  if (!type.int_type()) return nullptr;
  
  return AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>(icount, attribsi).find(type.get_int_type());
}

template <>
AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> AttributeComponent::get_finder() const {
  return {fcount, attribsf};
}

template <>
AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> AttributeComponent::get_finder() const {
  return {icount, attribsi};
}

void AttributeComponent::change_attribute(const AttribChangeData &data) {
  datas.push_back(data);
  currentTime = SIZE_MAX;
}

const AttribChangeData* AttributeComponent::get_attribute_change(const TypelessAttributeType &type, size_t &counter) const {
  if (datas.size() <= counter) {
    counter = 0;
    return nullptr;
  }
  
  const AttribChangeData* ptr = &datas[counter];
  
  while (ptr->attribType != type) {
    ++counter;
    ptr = &datas[counter];
  }
  
  ++counter;
  
  return ptr;
}

// void AttributeComponent::clear_counter() {
//   counter = 0;
// }

void AttributeComponent::addReaction(const AttributeReaction &reaction) {
  reactions.push_back(reaction);
}

//size_t & AttributeComponent::internalIndex() {
//  return index;
//}

Container<ExternalData>* AttributeComponent::externalDatas = nullptr;

// AttributeCreatorComponent::AttributeCreatorComponent(const CreateInfo &info) : floatInit(info.floatInit), intInit(info.intInit) {}
// 
// AttributeComponent* AttributeCreatorComponent::create(yacs::entity* ent, const std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &floatInit, const std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &intInit) const {
//   auto ptr = ent->get<PhysicsComponent>();
//   auto comp = ent->add<AttributeComponent>(AttributeComponent::CreateInfo{ptr.get(), nullptr, SIZE_MAX, floatInit, intInit});
//   return comp.get();
// }
// 
// AttributeComponent* AttributeCreatorComponent::create(yacs::entity* ent) const {
//   auto ptr = ent->get<PhysicsComponent>();
//   auto comp = ent->add<AttributeComponent>(AttributeComponent::CreateInfo{ptr.get(), nullptr, SIZE_MAX, floatInit, intInit});
//   return comp.get();
// }

AttributeSystem::AttributeSystem(const CreateInfo &info) : pool(info.pool) {}
AttributeSystem::~AttributeSystem() {}

void AttributeSystem::update(const size_t &time) {
  (void)time;

  static const auto func = [&] (const size_t &start, const size_t &count) {
    for (size_t i = start; i < start+count; ++i) {
      //components[i]->update(time);
      auto handle = Global::world()->get_component<AttributeComponent>(i);
      handle->update();
    }
  };

  const size_t componentCount = Global::world()->count_components<AttributeComponent>();
  const size_t count = std::ceil(float(componentCount) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, componentCount-start);
    if (jobCount == 0) break;

    pool->submitnr(func, start, jobCount);

    start += jobCount;
  }
  
  pool->compute();
  pool->wait();
}

//void AttributeSystem::add(AttributeComponent* comp) {
//  comp->internalIndex() = components.size();
//  components.push_back(comp);
//}
//
//void AttributeSystem::remove(AttributeComponent* comp) {
//  components.back()->internalIndex() = comp->internalIndex();
//  std::swap(components[comp->internalIndex()], components.back());
//  components.pop_back();
//}
