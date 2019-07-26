#include "AttributesComponent.h"

#include "Components.h"
#include "EventComponent.h"

const size_t LAST_SIZE_T_BIT = 1 << (SIZE_WIDTH - 1);

TypelessAttributeType::TypelessAttributeType(const TypelessAttributeType &type) : data(type.data) {}
TypelessAttributeType::TypelessAttributeType(const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type) : data(type.id() | LAST_SIZE_T_BIT) {}
TypelessAttributeType::TypelessAttributeType(const AttributeType<INT_ATTRIBUTE_TYPE> &type) : data(type.id() & (~LAST_SIZE_T_BIT)) {}

bool TypelessAttributeType::float_type() const {
  return (data & LAST_SIZE_T_BIT) == LAST_SIZE_T_BIT;
}

bool TypelessAttributeType::int_type() const {
  return (data & LAST_SIZE_T_BIT) == 0;
}

AttributeType<FLOAT_ATTRIBUTE_TYPE> TypelessAttributeType::get_float_type() const {
  if (float_type()) return AttributeType<FLOAT_ATTRIBUTE_TYPE>(data & (~LAST_SIZE_T_BIT));
  return AttributeType<FLOAT_ATTRIBUTE_TYPE>();
}

AttributeType<INT_ATTRIBUTE_TYPE> TypelessAttributeType::get_int_type() const {
  if (int_type()) return AttributeType<INT_ATTRIBUTE_TYPE>(data & (~LAST_SIZE_T_BIT));
  return AttributeType<INT_ATTRIBUTE_TYPE>();
}

TypelessAttributeType & TypelessAttributeType::operator=(const TypelessAttributeType &type) {
  data = type.data;
  return *this;
}

TypelessAttributeType & TypelessAttributeType::operator=(const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type) {
  data = type.id() | LAST_SIZE_T_BIT;
  return *this;
}

TypelessAttributeType & TypelessAttributeType::operator=(const AttributeType<INT_ATTRIBUTE_TYPE> &type) {
  data = type.id() & (~LAST_SIZE_T_BIT);
  return *this;
}

bool TypelessAttributeType::operator==(const TypelessAttributeType &type) const {
  return data == type.data;
}

bool TypelessAttributeType::operator!=(const TypelessAttributeType &type) const {
  return data != type.data;
}

AttribChangeType::AttribChangeType() : container(0) {}
AttribChangeType::AttribChangeType(const bool raw, const bool add) : container(0) {
  make(raw, add);
}

void AttribChangeType::make(const bool raw, const bool add) {
  container |= (raw * (1 << 0)) | (add * (1 << 1));
}

bool AttribChangeType::bonus_type_raw() const {
  const uint32_t mask = 1 << 0;
  return (container & mask) == mask;
}

bool AttribChangeType::bonus_math_add() const {
  const uint32_t mask = 1 << 1;
  return (container & mask) == mask;
}

void AttributeComponent::setContainer(Container<ExternalData>* cont) {
  AttributeComponent::externalDatas = cont;
}

AttributeComponent::AttributeComponent(const CreateInfo &info) : fcount(info.float_arrtibs.size()), icount(info.int_arrtibs.size()), attribsf(nullptr), attribsi(nullptr), phys(nullptr), system(info.system) {
  attribsf = fcount > 0 ? new Attribute<FLOAT_ATTRIBUTE_TYPE>[fcount] : nullptr;
  attribsi = icount > 0 ? new Attribute<INT_ATTRIBUTE_TYPE>[icount] : nullptr;
  
  for (size_t i = 0; i < fcount; ++i) {
    switch (attribsf[i].type().data_type()) {
      case ATTRIBUTE_CURRENT_SPEED: {
        attribsf[i].setType(info.float_arrtibs[i].type);
        attribsf[i].setBase(0.0f);
        attribsf[i].setValue(0.0f);
        break;
      }
      
      case ATTRIBUTE_UPDATE_EXTERNAL_DATA_MAX_SPEED: {
        attribsf[i].setType(info.float_arrtibs[i].type);
        attribsf[i].setBase(0.0f);
        break;
      }
      
      case ATTRIBUTE_UPDATE_EXTERNAL_DATA_ACCELERATION: {
        attribsf[i].setType(info.float_arrtibs[i].type);
        attribsf[i].setBase(0.0f);
        break;
      }
      
      case ATTRIBUTE_DATA_TYPE_NONE:
      case ATTRIBUTE_DATA_TYPE_COUNT:
        attribsf[i].setType(info.float_arrtibs[i].type);
        attribsf[i].setBase(info.float_arrtibs[i].baseValue);
    }
  }
  
  for (size_t i = 0; i < icount; ++i) {
    attribsi[i].setType(info.int_arrtibs[i].type);
    attribsi[i].setBase(info.int_arrtibs[i].baseValue);
  }
  
  system->add(this);
}

AttributeComponent::~AttributeComponent() {
  if (attribsf != nullptr) delete [] attribsf;
  if (attribsi != nullptr) delete [] attribsi;
  
  system->remove(this);
}

void AttributeComponent::update(const uint64_t &time) {
  (void)time;
  
  AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> float_finder(fcount, attribsf);
  AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> int_finder(icount, attribsi);
  
  for (size_t i = 0; i < datas.size(); ++i) {
    if (datas[i].attribType.float_type()) {
      const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_float_type();
      Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib = float_finder.find(type);
      
      if      ( datas[i].type.bonus_type_raw() &&  datas[i].type.bonus_math_add()) attrib->addBonus(datas[i].b);
      else if (!datas[i].type.bonus_type_raw() &&  datas[i].type.bonus_math_add()) attrib->addFinalBonus(datas[i].b);
      else if ( datas[i].type.bonus_type_raw() && !datas[i].type.bonus_math_add()) attrib->removeBonus(datas[i].b);
      else attrib->removeFinalBonus(datas[i].b);
    } else {
      const AttributeType<INT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_int_type();
      Attribute<INT_ATTRIBUTE_TYPE>* attrib = int_finder.find(type);
      
      if      ( datas[i].type.bonus_type_raw() &&  datas[i].type.bonus_math_add()) attrib->addBonus(datas[i].b);
      else if (!datas[i].type.bonus_type_raw() &&  datas[i].type.bonus_math_add()) attrib->addFinalBonus(datas[i].b);
      else if ( datas[i].type.bonus_type_raw() && !datas[i].type.bonus_math_add()) attrib->removeBonus(datas[i].b);
      else attrib->removeFinalBonus(datas[i].b);
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
  
  struct AttributeReactionPointers {
    AttributeReaction* reaction;
    void* attribute;
  };
  
  for (size_t i = 0; i < reactions.size(); ++i) {
    switch (reactions[i].comp) {
      case AttributeReaction::comparison::less: {
        if (reactions[i].attribType.float_type()) {
          const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_float_type();
          Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib = float_finder.find(type);
          
          if (attrib->value() < reactions[i].value) {
            // запускаем эвент с аттрибутом, реакцией?, неплохо было бы еще просмотреть изменения аттрибута
            // изменения именно этого аттрибута? (вообще логично что этого, для просмотра мне нужен по идее только тип)
            // 
            AttributeReactionPointers p{
              &reactions[i],
              attrib
            };
            
            const EventData ed{
              this,
              &p
            };
            events->fireEvent(reactions[i].event, ed);
          }
        } else {
          const AttributeType<INT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_int_type();
          Attribute<INT_ATTRIBUTE_TYPE>* attrib = int_finder.find(type);
          
          if (attrib->value() < reactions[i].value) {
            AttributeReactionPointers p{
              &reactions[i],
              attrib
            };
            
            const EventData ed{
              this,
              &p
            };
            events->fireEvent(reactions[i].event, ed);
          }
        }
        
        break;
      }
      
      case AttributeReaction::comparison::more: {
        if (reactions[i].attribType.float_type()) {
          const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_float_type();
          Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib = float_finder.find(type);
          
          if (attrib->value() > reactions[i].value) {
            AttributeReactionPointers p{
              &reactions[i],
              attrib
            };
            
            const EventData ed{
              this,
              &p
            };
            events->fireEvent(reactions[i].event, ed);
          }
        } else {
          const AttributeType<INT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_int_type();
          Attribute<INT_ATTRIBUTE_TYPE>* attrib = int_finder.find(type);
          
          if (attrib->value() > reactions[i].value) {
            AttributeReactionPointers p{
              &reactions[i],
              attrib
            };
            
            const EventData ed{
              this,
              &p
            };
            events->fireEvent(reactions[i].event, ed);
          }
        }
        
        break;
      }
      
      case AttributeReaction::comparison::equal: {
        if (reactions[i].attribType.float_type()) {
          const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_float_type();
          Attribute<FLOAT_ATTRIBUTE_TYPE>* attrib = float_finder.find(type);
          
          if (std::abs(attrib->value() - reactions[i].value) < EPSILON) {
            AttributeReactionPointers p{
              &reactions[i],
              attrib
            };
            
            const EventData ed{
              this,
              &p
            };
            events->fireEvent(reactions[i].event, ed);
          }
        } else {
          const AttributeType<INT_ATTRIBUTE_TYPE> &type = datas[i].attribType.get_int_type();
          Attribute<INT_ATTRIBUTE_TYPE>* attrib = int_finder.find(type);
          
          if (std::abs(attrib->value() - reactions[i].value) < EPSILON) {
            AttributeReactionPointers p{
              &reactions[i],
              attrib
            };
            
            const EventData ed{
              this,
              &p
            };
            events->fireEvent(reactions[i].event, ed);
          }
        }
        
        break;
      }
    }
  }
  
  datas.clear();
}

void AttributeComponent::init(void* userData) {
  (void)userData;
  
  phys = getEntity()->get<PhysicsComponent2>().get();
  
  // тут нет гарантии что PhysicsComponent2 заинитится быстрее чем AttributeComponent
  // нужно просто взять данные инитиализации
  
  if (phys != nullptr) {
    for (size_t i = 0; i < fcount; ++i) {
      switch (attribsf[i].type().data_type()) {
        case ATTRIBUTE_CURRENT_SPEED: {
          attribsf[i].setBase(phys->getSpeed());
          continue;
        }
        
        case ATTRIBUTE_UPDATE_EXTERNAL_DATA_MAX_SPEED: {
          const uint32_t index = phys->getExternalDataIndex();
          attribsf[i].setBase(externalDatas->at(index).maxSpeed);
          continue;
        }
        
        case ATTRIBUTE_UPDATE_EXTERNAL_DATA_ACCELERATION: {
          const uint32_t index = phys->getExternalDataIndex();
          attribsf[i].setBase(externalDatas->at(index).acceleration);
          continue;
        }
        
        case ATTRIBUTE_DATA_TYPE_NONE:
        case ATTRIBUTE_DATA_TYPE_COUNT:
          continue;
      }
    }
  }
}

template <>
const Attribute<FLOAT_ATTRIBUTE_TYPE>* AttributeComponent::get(const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type) {
  return AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>(fcount, attribsf).find(type);
}

template <>
const Attribute<INT_ATTRIBUTE_TYPE>* AttributeComponent::get(const AttributeType<INT_ATTRIBUTE_TYPE> &type) {
  return AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>(icount, attribsi).find(type);
}

template <>
AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> AttributeComponent::get_finder() const {
  return AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>(fcount, attribsf);
}

template <>
AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> AttributeComponent::get_finder() const {
  return AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>(icount, attribsi);
}

void AttributeComponent::change_attribute(const AttribChangeData &data) {
  datas.push_back(data);
}

size_t & AttributeComponent::internalIndex() {
  return index;
}

Container<ExternalData>* AttributeComponent::externalDatas = nullptr;

AttributeSystem::AttributeSystem(const CreateInfo &info) : pool(info.pool) {}
AttributeSystem::~AttributeSystem() {}

void AttributeSystem::update(const uint64_t &time) {
  static const auto func = [&] (const size_t &start, const size_t &count) {
    for (size_t i = start; i < start+count; ++i) {
      components[i]->update(time);
    }
  };
  
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
}

void AttributeSystem::add(AttributeComponent* comp) {
  comp->internalIndex() = components.size();
  components.push_back(comp);
}

void AttributeSystem::remove(AttributeComponent* comp) {
  components.back()->internalIndex() = comp->internalIndex();
  std::swap(components[comp->internalIndex()], components.back());
  components.pop_back();
}
