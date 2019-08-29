#include "Interaction.h"

void Interaction::setContainers(Container<Transform>* transforms, Container<simd::mat4>* matrices, Container<RotationData>* rotationDatas) {
  Interaction::transforms = transforms;
  Interaction::matrices = matrices;
  Interaction::rotationDatas = rotationDatas;
}

Interaction::Interaction(const enum Interaction::type &t, const Type &eventType, void* userData) : eventType(eventType), userData(userData), t(t), finished(false) {}
Interaction::~Interaction() {}

enum Interaction::type Interaction::type() const { return t; }
Type Interaction::event_type() const { return eventType; }
void* Interaction::user_data() const { return userData; }
bool Interaction::isFinished() const { return finished; }

Container<Transform>* Interaction::transforms = nullptr;
Container<simd::mat4>* Interaction::matrices = nullptr;
Container<RotationData>* Interaction::rotationDatas = nullptr;