#include "InfoComponent.h"

InfoComponent::InfoComponent(const CreateInfo &info) : m_type(info.type), m_ent(info.ent) {}
InfoComponent::~InfoComponent() {}

void InfoComponent::edit() {

}

Type InfoComponent::type() const {
  return m_type;
}
