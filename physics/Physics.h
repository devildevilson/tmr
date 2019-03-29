#ifndef PHYSICS_H
#define PHYSICS_H

#include <vector>
#include <string>

#include "glm/glm.hpp"

#include "Engine.h"
#include "PhysicsUtils.h"
#include "ArrayInterface.h"

struct PhysicsIndexContainer {
  uint32_t objectDataIndex;
  uint32_t physicDataIndex;
  uint32_t transformIndex;
  uint32_t inputIndex;
  uint32_t rotationIndex;
  uint32_t internalIndex;

  void* userData;
};

struct PhysicsObjectCreateInfo {
  bool rotation;

  PhysicsType type;
  uint32_t collisionGroup;
  uint32_t collisionFilter;

  float stairHeight;
  //float acceleration;
  float overbounce;
  float groundFricion;
  
  float radius;
  //glm::vec4 pos;

  uint32_t inputIndex;
  uint32_t transformIndex;
  uint32_t externalDataIndex;
  uint32_t matrixIndex;
  uint32_t rotationIndex;
  
  //PhysicsIndexContainer* comp;
  // std::vector<glm::vec4> points;
  // std::vector<glm::vec4> faces;
  std::string shapeName;
};

struct RegisterNewShapeInfo {
  std::vector<glm::vec4> points;
  std::vector<glm::vec4> faces;
  // нужно ли тут что то еще?
};

struct ShapeInfo {
  uint32_t shapeType;
  uint32_t offset;
  uint32_t pointCount;
  uint32_t faceCount;
};

// возможно в будущем я практически все буфферы буду создавать вне
// не уверен что из этого правда верное решение
// возможно лучше если будет какой то объект со всеми данными (вроде текущего контейнера, но не с индексами а с конкретными физ значениями)
struct PhysicsExternalBuffers {
  uint32_t defaultStaticMatrixIndex;
  uint32_t defaultDynamicMatrixIndex;

  ArrayInterface<InputData>* inputs;
  ArrayInterface<Transform>* transforms;
  ArrayInterface<glm::mat4>* matrices;
  ArrayInterface<uint32_t>* rotationDatasCount;
  ArrayInterface<RotationData>* rotationDatas;
  ArrayInterface<ExternalData>* externalDatas;
};

struct PhysicsOutputBuffers {
  ArrayInterface<OverlappingData>* overlappingData;
  ArrayInterface<OverlappingData>* rayTracingData;
  ArrayInterface<BroadphasePair>* frustumPairs;
};

// короч со всеми этими внешними массивами с данными возникает проблема
// усложняется создание дескрипторов и прочего связанного с пайплайном
// выход из этого как обычно это воспользоваться новой абстракцией
// например класс в котором создаются все массивы, это будет внешний класс который контролирует весь процесс выделения памяти
// также он будет контролировать создание дескрипторов (но тут возникают проблемы с тем как контролировать количество сетов в пайплайне)
// (тип придется скакать от класса к классу чтоб что-нибудь поменять, мне бы не хотелось этого)
// с другой стороны проблема заключается еще в том что некоторые буферы могут потребоваться в других местах 
// в совершенно иной конфигурации, следовательно дескрипторы нужно создавать непосредственно в физике (ну точнее там где создаются пайплайны)
// что делать? создавать дескрипторы для массивов отдельно в каждой системе? 
// (возможно это предпочтительный вариант, но тут возникает куча проблем с передачей разных вулкан объектов)
// в булете если и делано что все данные одного типа в одном массиве, то как то странно
// создание объектов там разделено на несколько этапов: создание шейпа, определение свойств, создание объекта
// и все в не в скоупе буллета, а где угодно может это произойти
// с другой стороны на основании чего другие классы должны знать как там че должно храниться в физике?
// хотя у меня наверное это будет походить скорее на то что все классы будут придерживаться единого стандарта
// с другой стороны мне и правда ненужно ВСЕ давнные физики выносить из нее
// достаточно лишь того что будет использоваться в разных местах, например позиции, инпуты и тд
// впрочем в самом буллете по всей видимости эти данные копируются по сотне раз
// не хочу копировать =(
// создавать отдельные дескрипторы? тогда либо тащить дохурилион всего по всем шейдерам,
// либо создавать для каждой ситуации дескриптор и менять его миллиард раз (на первый взгляд это муторно, но скорее всего так и есть)
// либо для вытащенных вне массивов использовать некий стандарт и использовать эти дескрипторы, но что если их станет слишком много?
// копиДескрипторСет использовать? геморно копировать каждый раз, а копировать придется часто
// проще конечно сделать уникальные дескрипторы для каждого внешнего массива, а внутри подумать какие массивы можно пихнуть вместе
// в общем проблема решается просто добавлением уникального дескриптора матрицам, но тогда у меня становится критически много дескрипторСетов в шейдере
// а ведь максимум на линухе 8, булщит какой-то
// вообще мне бы надо бы подумать о том как реорганизовать вообще все данные в физике, так как у меня сейчас смешано все подряд
// первое что приходит в голову дак это растащить все по отдельным буферам и тащить все эти будеры по всем шейдерам физики
// зато один дескриптор, врочем при разделении в более мелкие части скорее всего этого никак не избежать (у меня и так впритык все)
// с другой стороны правильное растаскивание по разным местам свойств - это более верное решение

// сейчас я вычисляю физику 1 раз за кадр, вычисляя скорость от изменяющегося времени
// но также имеет смысл попробовать константный шаг (например, тех же 10 раз (точнее 1/60), по которым у меня вычисляется солвер)
// возможно мне удастся переделать тогда солвер на более приличный
// впрочем солвер скорее всего все равно переделывать
class PhysicsEngine : public Engine {
public:
  virtual ~PhysicsEngine();
  
  static glm::vec4 getGravity();
  static glm::vec4 getGravityNorm();
  static float getGravLenght();
  static float getGravLenght2();
  
  static glm::vec4 abscissa();
  static glm::vec4 ordinate();
  static glm::vec4 applicat();
  static glm::mat3 getTransform();
  static glm::mat4 getOrientation();

  virtual void setBuffers(const PhysicsExternalBuffers &buffers) = 0;
  
  virtual void registerShape(const std::string &name, const uint32_t shapeType, const RegisterNewShapeInfo &info) = 0;

  virtual void add(const PhysicsObjectCreateInfo &info, PhysicsIndexContainer* container) = 0;
  virtual void remove(PhysicsIndexContainer* comp) = 0;

  // нужно наверное также переделать класс с сортировками
  // тип обозвать его как-нибудь физиксУтилс, и он будет сортировать разные массивы
  // точнее наверное один (два) тип разных массивов
  virtual uint32_t add(const RayData &ray) = 0; // лучи и фрустумы нужно передобавлять каждый кадр
  virtual uint32_t add(const glm::mat4 &frustum, const glm::vec4 &pos = glm::vec4(10000.0f)) = 0; // так добавить фрустум, или вычислить его вне?
  
  virtual Object & getObjectData(const uint32_t &index) = 0;
  virtual const Object & getObjectData(const uint32_t &index) const = 0;
  
  virtual PhysData2 & getPhysicData(const uint32_t &index) = 0;
  virtual const PhysData2 & getPhysicData(const uint32_t &index) const = 0;
  
  // virtual Transform & getTransform(const uint32_t &index) = 0;
  // virtual const Transform & getTransform(const uint32_t &index) const = 0;

  // virtual RotationData & getRotationData(const uint32_t &index) = 0;
  // virtual const RotationData & getRotationData(const uint32_t &index) const = 0;
  
  virtual void* getUserData(const uint32_t &objIndex) const = 0;
  
  virtual void setGravity(const glm::vec4 &g) = 0;
  
  virtual void updateMaxSpeed(const uint32_t &physicDataIndex, const float &maxSpeed) = 0;
  //virtual void setInput(const uint32_t &index, const InputData &input) = 0;
  virtual uint32_t setShapePointsAndFaces(const uint32_t &objectDataIndex, const std::vector<glm::vec4> &points, const std::vector<glm::vec4> &faces) = 0;

  // мы еще должны здесь получать данные пересечия, причем пересечения на предыдущем кадре тоже должны быть проверены
  // и в итоге должен быть массив, в котором поддерживаются результаты пересечения
  // (короче у меня сейчас некоторые объекты у которых скорость 0 могут выпасть из некоторых проверок, что нежелательно)
  // (и мне нужен массив с дистом, мтв и прочим, с возможностью его сортировать и чекать уникальность)
  // ! требуемый результат можно получить если в каждом кадре проверять все динамические объекты !
  // я пытаюсь всеми силами этого избежать
  virtual ArrayInterface<OverlappingData>* getOverlappingPairsData() = 0;
  virtual const ArrayInterface<OverlappingData>* getOverlappingPairsData() const = 0;
  
  virtual ArrayInterface<uint32_t>* getTriggerPairsIndices() = 0;
  virtual const ArrayInterface<uint32_t>* getTriggerPairsIndices() const = 0;

  // тут скорее всего достаточно просто "стандартной" пары (тип: индекс, индекс, расстояние, индекс батча/острова/сортировки/прочее)
  // но мне может потребоваться более точное вычисление лучей (точнее скорее всего нужно, а для этого нужно еще к расстоянию добавить точку пересечения)
  virtual ArrayInterface<OverlappingData>* getRayTracingData() = 0;
  virtual const ArrayInterface<OverlappingData>* getRayTracingData() const = 0;

  // тут дополнительно ничего особо не нужно (нужно будет массив отсортировать)
  virtual ArrayInterface<BroadphasePair>* getFrustumPairs() = 0;
  virtual const ArrayInterface<BroadphasePair>* getFrustumPairs() const = 0;

  uint32_t getOverlappingDataSize() const;
  uint32_t getTriggerPairsIndicesSize() const;
  uint32_t getRayTracingSize() const;
  uint32_t getFrustumTestSize() const;

  virtual void printStats() = 0;
protected:
  static glm::vec4 gravity;
  static glm::vec4 gravityNorm;
  static float gravLength2;
  static float gravLength;
  
  static glm::mat4 orientation;

  uint32_t overlappingDataSize = 0;
  uint32_t triggerPairsIndicesSize = 0;
  uint32_t rayTracingSize = 0;
  uint32_t frustumTestSize = 0;
};

#endif
