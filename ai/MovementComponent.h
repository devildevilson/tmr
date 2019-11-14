#ifndef MOVEMENT_COMPONENT_H
#define MOVEMENT_COMPONENT_H

class EntityAI;

// через этот компонент будем искать путь, следовать по пути, двигаться в необходимую строну, преследовать цель и прочее
class MovementComponent {
public:
  enum state {
    found,
    finding,
    not_found
  };
  
  state findPath(const EntityAI* target); // по идее мы можем указать любой объект на уровне
  void movePath(); // двигаемся по пути, что возвращать? какой то статус пройденого пути?
  void move(); // указываем направление
  
  // нужно ли делать версию с конкретной позицией? как сделать патруль? 
  // можно создать несколько псевдо объектов, и двигаться от одного к другому
  void pursue(const EntityAI* target); // скорее всего просто двигаемся к цели
  void flee(const EntityAI* target); // бежим от цели
private:
  
};

#endif
