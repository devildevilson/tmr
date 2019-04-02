#ifndef PHYSIC_UTILS
#define PHYSIC_UTILS

#include <cstdint>
// #include "glm/glm.hpp"


#include "PhysicsTemporary.h"

#define BBOX_TYPE 0
#define SPHERE_TYPE 1
#define POLYGON_TYPE 2

struct FastAABB {
  simd::vec4 center;
  simd::vec4 extent;
};

struct RayData {
  simd::vec4 pos;
  simd::vec4 dir;
};

struct FrustumStruct {
  simd::vec4 planes[6];

  FrustumStruct();
  FrustumStruct(const simd::mat4 &matrix);

  void calcFrustum(const simd::mat4 &matrix);
};

struct OverlappingData {
  uint32_t firstIndex;
  uint32_t secondIndex;
  uint32_t hasCollision;
  uint32_t dummy; // type? (переменная которая поможет нам определить, какие из пар все еще пересекаются)
  // нам это нужно чтоб, во первых отсортировать те пары которые уже не пересекаются
  // во вторых понять какие пары нужно обработать и как
  // это не сработало

  glm::vec3 vec; // это может быть позиция
  float dist;
};

struct DataIndices {
  uint32_t indirectX;
  uint32_t indirectY;
  uint32_t indirectZ;
  uint32_t count;
  uint32_t temporaryCount;
  uint32_t powerOfTwo;
  uint32_t triggerIndicesCount;
};

struct IslandData {
  uint32_t islandIndex;
  uint32_t offset;
  uint32_t size;
  uint32_t dummy;
};

struct PhysicsType {
  uint32_t type;
  
  PhysicsType();
  PhysicsType(const bool &dynamic, const uint32_t &objType, const bool &blocking, const bool &trigger, const bool &blockRays, const bool &visible);
  
  void makeType(const bool &dynamic, const uint32_t &objType, const bool &blocking, const bool &trigger, const bool &blockRays, const bool &visible);
  bool isDynamic() const;
  uint32_t getObjType() const;
  bool isBlocking() const;
  bool isTrigger() const;
  bool canBlockRays() const;
  bool isVisible() const;
};

// в физике существует 3 типа объектов:
// трансформа (точное расположение объекта)
// данные объекта (вершины и нормали объекта, его форма)
// данные тела (физические свойства ТЕЛА, скорость, ускорение, и прочее)
// все что мне нужно так это 3 структуры и они должны явно указывать только те данные для которых их создавали
// скорее всего нужна еще одна которая объединит это все 

// struct Object2 {
//   uint32_t objectId; // это нужно будет для обратной связи с структурой для индексов
//   uint32_t proxyIndex;
//   uint32_t staticPhysicDataIndex; // константИндекс
//   uint32_t transformIndex;
//   
//   uint32_t coordinateSystemIndex;
//   uint32_t groundObjIndex;
//   //uint32_t staticPhysicDataIndex;
//   uint32_t rotationDataIndex;
//   uint32_t dummy;
// };

struct ObjectForm {
  // вот эти индексы могут вполне уйти в другую структуру
  uint32_t vertexOffset;
  uint32_t vertexCount;
  uint32_t faceCount;
  PhysicsType objType;
};

struct Object {
  // не должен обж конечно знать ни о чем таком как энтити айди
  //uint32_t entityId; // это скорее всего не нужно
  uint32_t objectId; // это нужно будет для обратной связи с структурой для индексов
  uint32_t proxyIndex;
  uint32_t staticPhysicDataIndex; // константИндекс
  uint32_t transformIndex;
  
  // вот эти индексы могут вполне уйти в другую структуру
  uint32_t vertexOffset;
  uint32_t vertexCount;
  uint32_t faceCount;
  PhysicsType objType;
  
  //uint32_t transformIndex;
  uint32_t coordinateSystemIndex;
  uint32_t groundObjIndex;
  //uint32_t staticPhysicDataIndex;
  uint32_t rotationDataIndex;
  uint32_t dummy;
  //uint32_t groundObjIndex;
};

// больше всего меня конечно занимает физдата
// максСпид и акселератион должны быть доступны вне конечно чтобы не обновлять их поуебски
// адитионалФорс тоже должен быть доступен вне (я правда пока не нашел ему применения)
// все константы все же необходимо держать в одном месте (stairHeight, groundFriction и overbounce)
// скорее всего динамические/нет можно определять по наличию скорости
struct PhysicData {
  simd::vec4 velocity;
//   uint32_t objectIndex;//isOnGround;
//   uint32_t inputIndex;
//   uint32_t groundIndex;
//   uint32_t blockingIndex;
  simd::vec4 additionalForce;
  simd::vec4 oldPos;
  
  //simd::vec4 constants;
  float maxSpeed;
  //float groundFriction;
  //float airFriction;
  float acceleration;
  float stairHeight;
  float scalar;
  
  //glm::uvec4 indexies;
  uint32_t objectIndex;
  uint32_t inputIndex;
  uint32_t groundIndex;
  uint32_t blockingIndex;
  
  //glm::uvec4 additionalData;
  //float overbounce;
  //float stairHeight;
  uint32_t transformIndex;
  uint32_t staticPhysicDataIndex;
  uint32_t isOnGround;
  uint32_t wasOnGround;
  //uint32_t dummy2;
  //uint32_t objectIndex;
};

struct ExternalData {
  simd::vec4 additionalForce;
  float maxSpeed;
  float acceleration;
  float dummy1;
  float dummy2;
};

struct Velocity {
  glm::vec3 velocity;
  float scalar;
};

struct Constants {
  float groundFriction;
  float overbounce;
  float stairHeight;
  //float dummy;
  uint32_t physDataIndex;
};

// такая физДата тоже не особо нужна статическим объектам
// в булете то что является у меня физДатой - главная структура
// 
struct PhysData2 {
  glm::vec3 velocity;
  float scalar;
  simd::vec4 oldPos;

  uint32_t objectIndex;
  uint32_t inputIndex;
  uint32_t groundIndex; // тут тип константИндекс
  uint32_t blockingIndex;

  //uint32_t velocityIndex;
  uint32_t externalDataIndex;
  uint32_t transformIndex;
  uint32_t constantsIndex;
  uint32_t onGroundBits;

  // uint32_t isOnGround;
  // uint32_t wasOnGround;
  // uint32_t dummy1;
  // uint32_t dummy2;
};

// что то подобное может сильно подсократить количество физдаты
// так как мы уберем кучу ненужных данных для статиков
// эту структуру нужно сделать главной!!!! (ну то есть из обджекта мы указываем на нее, а она уже в свою очередь указывает на данные о скоростях)
// тогда эта структура у меня будет для каждого объекта
// в принципе она меньше чем предыдущая физ дата, нужно только понять чем ее наполнить
// добавить эту инфу в обжект? (тип мне нужно то только фриктион и овербоунс)
struct StaticPhysicData {
  // float groundFriction;
  // float overbounce;
  // float dummy1;
  // float dummy2;
  // uint32_t objectIndex;
  // uint32_t physicDataIndex;
  // uint32_t groundIndex;
  // uint32_t blockingIndex;
  float groundFriction;
  float overbounce;
  uint32_t objectIndex;
  uint32_t physicDataIndex;
};

// для того чтобы сделать двери похожие на хексеновские мне нужно:
// 1. уникальная матрица поворота (может быть какие то матрицы можно использовать нескольким объектам)
// 2. направление якоря по умолчанию (то есть нормализованный вектор указывающий направление ОТ якоря (то есть сначала вычесть потом прибавить))
// 3. расстояние до якоря (тут понятно)
// таким образом для любого объекта использующего такую систему мы должны
// а) из позиций вершин вычесть направление*расстояние
// б) перемножить позиции с матрицей поворота и направление тоже
// в) к новым позициям прибавить новое направление*расстояние
// по идее тот же эффект мы должны получить просто перемножив матрицы в правильном порядке
// тип по умолчанию у нас есть форма двери, нам нужно провести верные манипуляции, чтобы получить открывающуюся дверь
// для этого матрица поворота должна быть аддиктивной (то есть угол поворота мы должны собирать) 
// выглядеть это в итоге будет:
// а) прибавляем позицию якоря (перевычисляем из дефолтной позиции?)
// б) перемножить позиции с матрицей поворота и направление тоже
// в) к новым позициям прибавить новое направление*расстояние
// это будут вершины для вычисления боксов вокруг них, как добавить это в шейдеры?
// мне еще пригодится максимальный/минимальный угол поворота (?), скорость поворота, еще наверное время между шагом поворота
// + нам бы не помешала возможность делать двери из одного шейпа в разных положениях (это еще одна матрица пихается куда то)
// как это учесть? матрица по умолчанию?
// еще конечно нужно проконтролировать КОГДА происходят все эти повороты

// struct RotationData {
//   glm::vec3 anchorDir;
//   float anchorDist;
//   // или тут все же позиция якоря должна быть?

//   float currentAngle;
//   float maxAngle; // нужен ли минимальный угол? или он всегда 0?
//   float rotationSpeed;
//   uint32_t stepTime; // нужно ли?

//   simd::mat4 matrix;
// };

// struct Transform {
//   simd::vec4 pos;
//   simd::mat4 orn;
// };

// struct InputData {
//   simd::vec4 right;
//   simd::vec4 up;
//   simd::vec4 front;
//   simd::vec4 moves;
// };

struct Gravity {
  simd::vec4 gravity;
  simd::vec4 gravityNormal;
  
  float length;
  float length2;
  float dummy1;
  float dummy2;
  
  uint32_t objCount;
  uint32_t time;
  uint32_t dummy3;
  uint32_t dummy4;
};

bool testAABB(const FastAABB &first, const FastAABB &second);
// second contain first
bool AABBcontain(const FastAABB &first, const FastAABB &second);

simd::vec4 getVertex(const simd::vec4 &pos, const simd::vec4 &ext, const simd::mat4 &orn, const uint32_t &index);

#endif
