#ifndef COMPONENT_CREATOR_H
#define COMPONENT_CREATOR_H

// в таких функциях мы будем создавать компоненты какого то типа
// функции должны быть определены в компонентах типа? или же отдельно, в энтити лоадере например?
// предпочтительно чтобы в компонентах типа это было определено, тогда мы можем сократить немного вычислений 

#include "Type.h"

#include <unordered_map>
#include <vector>

namespace yacs {
  class entity;
}

// class ComponentCreator {
// public:
//   virtual ~ComponentCreator() {}
//   
//   virtual void create(const yacs::entity* type, yacs::entity* parent, yacs::entity* ent) const = 0;
// };

// попытка как то решить проблему означанную в самом низу 
// в чем проблема? мы можем указать больше данных чем нужно энтити, но не наоборот
// нам так или иначе нужно указать что то, но теперь мы можем в ифы все пихнуть
// либо задать данные в иных местах а потом просто передать контейнер с одного места на другое
// копирование контейнера? на всякий случай можно сделать

struct DataIdentifier {
  union {
    size_t num;
    struct {
      char str[sizeof(size_t)];
    };
  };
  
  DataIdentifier(const char* name);
  DataIdentifier(const DataIdentifier &id);
  
  bool operator==(const DataIdentifier &another) const;
  DataIdentifier & operator=(const DataIdentifier &id);
};

class UniversalDataContainer {
public:
  struct DataInfo {
    DataIdentifier id;
    size_t size;
    void* data;
  };
  
  struct CreateInfo {
    std::vector<DataInfo> datas;
  };
  UniversalDataContainer(const CreateInfo &info);
  UniversalDataContainer(const UniversalDataContainer &cont);
  ~UniversalDataContainer();
  
  char* get_data(const DataIdentifier &id) const;
  size_t get_data_size(const DataIdentifier &id) const;
  size_t size() const;
  
  UniversalDataContainer & operator=(const UniversalDataContainer &cont);
private:
  struct DataPlacement {
    DataIdentifier id;
    size_t offset;
    size_t size;
  };
  
  size_t dataSize;
  char* data;
  std::vector<DataPlacement> placements;
};

class CreateComponent {
public:
  virtual ~CreateComponent() {}
  
  virtual void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const = 0;
};

class CreatorComponent {
public:
  struct CreateInfo {
    std::vector<CreateComponent*> creators;
  };
  CreatorComponent(const CreateInfo &info);
  
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const;
private:
  std::vector<CreateComponent*> creators; // последовательно обходим все и создаем
};

// class EntityCreator {
// public:
//   
//   // некоторые создатели будут сделаны заранее, например создатели стен, для них же можно сделать отдельную функцию
//   
//   // как передать данные для создания? возможно стоит это делать позже
//   // передавать данные после создания? кажется это единственный более менее нормальный выход
//   yacs::entity* create(const Type &type, yacs::entity* parent);
// private:
//   std::unordered_map<Type, const yacs::entity*> types;
//   
//   // запомнить созданные энтити? не сработает, энтити по идее должны самоудаляться
//   // + список энтити есть в world
//   //std::vector<yacs::entity*> created;
// };

// как быть с энтити которые не абилка? у них должен быть способ задать данные из карты (или из сохранения)
// эти данные мы указываем при загрузке карты, то есть создаем энтити, парсим карту, получаем данные, смотрим что куда мы добавляем

// для того чтобы это хорошо работало, мне нужно у многих компонентов определять метод инит, что мне очень не нравится
// можно ли как то избежать этого? то есть мне нужно чтобы данные о создании были известны заранее и были переданы в CreatorComponent
// это почти сразу означает хардкодинг, есть еще вариант подавать данные для стен, энтити и проч в CreatorComponent
// но это означает что нужно создать какой-то контейнер с данными, причем универсальный

#endif
