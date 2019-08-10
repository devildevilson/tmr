#include "InfoComponent.h"

InfoComponent::InfoComponent(const CreateInfo &info) : type(info.type), userData(info.userData) {}
InfoComponent::~InfoComponent() {}

void InfoComponent::edit() {

}

std::string InfoComponent::getType() const {
  return type.getName();
}
