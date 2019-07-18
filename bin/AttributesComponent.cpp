#include "AttributesComponent.h"

#include "Components.h"

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
  
  for (size_t i = 0; i < fcount; ++i) {
    switch (attribsf[i].type().data_type()) {
      case ATTRIBUTE_CURRENT_SPEED: {
        if (phys != nullptr) break;
        attribsf[i].setBase(phys->getSpeed());
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
}

void AttributeComponent::init(void* userData) {
  (void)userData;
  
  phys = getEntity()->get<PhysicsComponent2>().get();
  
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
