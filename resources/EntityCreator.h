#ifndef ENTITY_CREATOR_H
#define ENTITY_CREATOR_H

namespace yacs {
  class entity;
  class world;
}

// для стен и абилок создание будет очень похоже, для энтити нужно будет подгрузить данные из json
// типов загрузчиков будет видимо крайне мало: стены, абилка, энтити, только энтити скорее всего будет обладать большим количеством настроек
// абилка должна как то поставляться в энтити не разделяя их 
class EntityCreator {
public:
  virtual ~EntityCreator() {}
  
  virtual yacs::entity* create(yacs::entity* parent, const void* data) const = 0; // тут мы создаем все от начала до конца, в data хранится необходимые данные для конкретного типа энтити
};

#endif
