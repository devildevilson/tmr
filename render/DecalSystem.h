#ifndef DECAL_SYSTEM_H
#define DECAL_SYSTEM_H

#include "Engine.h"
#include "Utility.h"
#include "RenderStructures.h"
#include "ArrayInterface.h"
#include "PhysicsTemporary.h"

#include <vector>

struct DecalPhysContainer;
class DecalComponent;

namespace yavf {
  class Buffer;
}

namespace yacs {
  class entity;
}

// нам нужно найти все стены пересекающие объект
// далее, мы должны посчитать для каждой стены кусочек декали
// это значит расчитать несколько точек
// и выдать им верные текстурные координаты
// нужно проджектить все точки бокса на плоскость, а затем
// выбрать самые максимальные точки входящие в плоскость
// либо ближайшие точки к прецированным, вот только
// мне нужно взять всего максимум 4 точки (нет, может быть больше)
// минимум 3 точки при любых условиях

// нашел статью наконец https://vk.com/@tproger-dekali-games
// там говориться что нужно перевести вершины треугольника в базис декали
// базис расчитывается по нормали (у них он как-то странно расчитывается, нужно самому посчитать)
// затем клипаем все вершины плоскости 

// декаль - квадрат в мировых координатах (примерно как монстры, только не поворачивается за игроком)
// scale * координаты чтоб получить то что нужно

// как декали распространить на двери? если декали не делать самостоятельными
// а добавить их к объектам, то тогда можно сделать так чтобы декаль перемещалась с дверью
// но тогда она теряет "самостоятельность" (я хотел бы чтобы кровь еще умела стекать, размазываться как-нибудь и прочее)
// стекание крови и подобные эффекты будут очень редки
// вот что действительно не помешало бы сделать так это стекающую водичку
// но это не имеет отношение к декалям
// думаю что стекание мне не пригодится

// значит для декали не нужно создавать энтити
// кусочек декали мы добавим в непосредственно объект
// так мы сможем гарантировать что кусочек декали будет двигаться вместе со стеной
// + отрисовывать будем в зависимости от видимости стены

// нужно будет создать физиксКомпонент для того чтобы найти все пересечения со стенами

// для декали еще было бы неплохо сделать возможность анимации
// это означает что нужно передать не саму текстур дату, а индекс
// но мне еще нужно будет компонент анимации сделать
// это все означает что мне скорее всего нужно сделать энтити на каждый отдельный кусочек декали
// это все затрудняет удаление декали

// тут нам нужно указать позицию, размер и ориентацию декали
// с помощью этого мы вычислим пересечение декали с объектами
struct PendingDecalData {
  simd::vec4 pos;
  simd::mat4 orn; // по идее если все правильно, то orn[2] - нормаль
  float scale; // отношение ширины к высоте (или наоборот, но это скейл для того чтобы правильно показать не квадратную картинку)
  // высота/ширина, лучше всего на мой взгляд
  float size; // на это мы умножим конечную координату
  
  TextureData texture; // должна ли быть у декали анимация?
};

// как перемещать декали? для этого нам нужно запомнить где то ориентацию
// для того чтобы ее можно было выделить  нужно сделать физикс компонент

// нужно ли удалять декаль? может пригодиться
// "старые" или много декалей одного типа может потребоваться почистить
class DecalSystem : public Engine {
public:
  static void setContainer(Container<simd::mat4>* matrixContainer);
  static void setContainer(Container<Transform>* transformContainer);
  
  DecalSystem();
  ~DecalSystem();
  
  void update(const uint64_t &time) override;
  
  yacs::entity* add(const PendingDecalData &data);
  void remove(yacs::entity* ent);
  void removeOld();
  void changePlace(yacs::entity* ent, const PendingDecalData &data);
private:
  struct ComputedDecals {
    yacs::entity* ent;
    DecalComponent* comp;
    uint32_t matrixIndex;
  };
  
  // pending decals
//   std::vector<PendingDecalData> pendingDecals;
  std::vector<DecalPhysContainer*> pendingPhysics;

  // computed decals
  // вычисленные декали состоят из вершин и текстур, видимо придется еще заполнить вершинные индексы
  // это достаточно для декали, еще наверное нужна нормаль для того чтобы чуть чуть пододвинуть декаль
//   std::vector<Vertex> vertices;
//   std::vector<uint32_t> indices;
//   std::vector<TextureData> textures;
  
  std::vector<ComputedDecals> decals;
  
  // потом введем гпу буфер
  // как запомнить декаль? вернуть энтити?
  
  void clip(const std::vector<simd::vec4> &inVerts, std::vector<simd::vec4> &outVerts, const simd::vec4 &normal, const float &dist);
  
  static Container<simd::mat4>* matrix;
  static Container<Transform>* transforms;
  // тут нужен еще ротатион
};

// вершины лучше всего передавать массивами в компонент
// так мы легко сможем удалить декаль, так чтобы при этом не остались дырки
// проблема такого подхода прежде всего заключается в том что у нас вершины не будут храниться в одном месте
// но с другой стороны потом в оптимизере мы складываем все эти вершины в один буфер
// таким же образом можно сделать и стены (в смысле стены на карте)
// по всей видимости это самый нормальный вариант
// тогда для удаления нам нужно хранить какие-нибудь указатели на данные декали + на декаль в компоненте
// возможно нужно создать энтити отдельный для декали, в котором мы будем хранить все места где декаль находится
// + позицию и ориентацию, так будет проще всего удалять, 
// для смены положения нужно будет просто сделать еще один метод

#endif
