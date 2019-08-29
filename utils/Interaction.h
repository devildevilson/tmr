#ifndef INTERACTION_H
#define INTERACTION_H

#include "Utility.h"
#include "PhysicsTemporary.h"
#include "ArrayInterface.h"
#include "Type.h"

struct UserDataComponent;

class Interaction {
public:
  static void setContainers(Container<Transform>* transforms, Container<simd::mat4>* matrices, Container<RotationData>* rotationDatas);

  enum class type {
    target,
    ray,
    physics,
    projectile,
    aura,
    collision
  };

  Interaction(const type &t, const Type &eventType, void* userData);
  virtual ~Interaction();

  struct NewData {
    simd::vec4 pos;
    simd::vec4 dir;
  };
  virtual void update_data(const NewData &data) = 0;
  virtual void update(const size_t &time) = 0;
  virtual void cancel() = 0;

  // в большинстве случаев, объектов для которых нужно вызвать эвент будет 1
  // и вообще можно сделать примерно также как мы делали для EntityAI то есть
  // выдавать указатель на PhysicsIndexContainer (спорно) пока не nullptr
  // вообще можно выдвавать сразу юзер дату, но может ли нам что нибудь еще пригодиться?
  virtual UserDataComponent* get_next() = 0;

  enum type type() const;
  Type event_type() const;
  void* user_data() const;
  bool isFinished() const;
private:
  Type eventType;
  void* userData;
  enum type t;

protected:
  bool finished;

  // нам тут нужен доступ к данным о матрицах, поворотах и позиции объекта
  static Container<Transform>* transforms;
  static Container<simd::mat4>* matrices;
  static Container<RotationData>* rotationDatas;
};

#endif //INTERACTION_H
