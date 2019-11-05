#include "Attributes.h"

//const size_t LAST_SIZE_T_BIT = size_t(1) << (SIZE_WIDTH - 1);

//TypelessAttributeType::TypelessAttributeType(const TypelessAttributeType &type) : data(type.data) {}
TypelessAttributeType::TypelessAttributeType() noexcept : type(false), data(nullptr) {}
TypelessAttributeType::TypelessAttributeType(const AttributeType<FLOAT_ATTRIBUTE_TYPE>* type) noexcept : type(true), data(type) {}
TypelessAttributeType::TypelessAttributeType(const AttributeType<INT_ATTRIBUTE_TYPE>* type) noexcept : type(false), data(type) {}

bool TypelessAttributeType::valid() const {
  return data != nullptr;
}

bool TypelessAttributeType::float_type() const noexcept {
  return type;
}

bool TypelessAttributeType::int_type() const noexcept {
  return !type;
}

const AttributeType<FLOAT_ATTRIBUTE_TYPE>* TypelessAttributeType::get_float_type() const noexcept {
  if (float_type()) return reinterpret_cast<const AttributeType<FLOAT_ATTRIBUTE_TYPE>*>(data);
  return nullptr;
}

const AttributeType<INT_ATTRIBUTE_TYPE>* TypelessAttributeType::get_int_type() const noexcept {
  if (int_type()) return reinterpret_cast<const AttributeType<INT_ATTRIBUTE_TYPE>*>(data);
  return nullptr;
}

//TypelessAttributeType & TypelessAttributeType::operator=(const TypelessAttributeType &type) {
//  data = type.data;
//  return *this;
//}

TypelessAttributeType & TypelessAttributeType::operator=(const AttributeType<FLOAT_ATTRIBUTE_TYPE>* type) noexcept {
  this->type = true;
  this->data = type;
  return *this;
}

TypelessAttributeType & TypelessAttributeType::operator=(const AttributeType<INT_ATTRIBUTE_TYPE>* type) noexcept {
  this->type = false;
  this->data = type;
  return *this;
}

bool TypelessAttributeType::operator==(const TypelessAttributeType &type) const noexcept {
  return data == type.data;
}

bool TypelessAttributeType::operator!=(const TypelessAttributeType &type) const noexcept {
  return data != type.data;
}
