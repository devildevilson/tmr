#include "Attributes.h"

const size_t LAST_SIZE_T_BIT = size_t(1) << (SIZE_WIDTH - 1);

//TypelessAttributeType::TypelessAttributeType(const TypelessAttributeType &type) : data(type.data) {}
TypelessAttributeType::TypelessAttributeType() noexcept : data(SIZE_MAX) {}
TypelessAttributeType::TypelessAttributeType(const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type) noexcept : data(type.id() | LAST_SIZE_T_BIT) {}
TypelessAttributeType::TypelessAttributeType(const AttributeType<INT_ATTRIBUTE_TYPE> &type) noexcept : data(type.id() & (~LAST_SIZE_T_BIT)) {}

bool TypelessAttributeType::valid() const {
  return data != SIZE_MAX;
}

bool TypelessAttributeType::float_type() const noexcept {
  return (data & LAST_SIZE_T_BIT) == LAST_SIZE_T_BIT;
}

bool TypelessAttributeType::int_type() const noexcept {
  return (data & LAST_SIZE_T_BIT) == 0;
}

AttributeType<FLOAT_ATTRIBUTE_TYPE> TypelessAttributeType::get_float_type() const noexcept {
  if (float_type()) return AttributeType<FLOAT_ATTRIBUTE_TYPE>(data & (~LAST_SIZE_T_BIT));
  return AttributeType<FLOAT_ATTRIBUTE_TYPE>();
}

AttributeType<INT_ATTRIBUTE_TYPE> TypelessAttributeType::get_int_type() const noexcept {
  if (int_type()) return AttributeType<INT_ATTRIBUTE_TYPE>(data & (~LAST_SIZE_T_BIT));
  return AttributeType<INT_ATTRIBUTE_TYPE>();
}

//TypelessAttributeType & TypelessAttributeType::operator=(const TypelessAttributeType &type) {
//  data = type.data;
//  return *this;
//}

TypelessAttributeType & TypelessAttributeType::operator=(const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type) noexcept {
  data = type.id() | LAST_SIZE_T_BIT;
  return *this;
}

TypelessAttributeType & TypelessAttributeType::operator=(const AttributeType<INT_ATTRIBUTE_TYPE> &type) noexcept {
  data = type.id() & (~LAST_SIZE_T_BIT);
  return *this;
}

bool TypelessAttributeType::operator==(const TypelessAttributeType &type) const noexcept {
  return data == type.data;
}

bool TypelessAttributeType::operator!=(const TypelessAttributeType &type) const noexcept {
  return data != type.data;
}