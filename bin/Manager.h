#ifndef MANAGER_H
#define MANAGER_H

#include <vector>
#include <string>
#include <list>
#include <unordered_map>
#include <iostream>

#include "nlohmann/json.hpp"
#include "MemoryPool.h"

// как выглядит конфликт
// это несколько разных ресурсов из разных модов претендующие на одно место
// следовательно должны быть указаны эти ресурсы

// загрузку в динамике мы можем реализовать междусобойчиком внутри менеджера
// то есть в нем хранить че загружено сейчас, при загрузке нового уровня чекать что ненужно
// и в соответствии с этим предпринимать какие-то действия
// основная сложность это обновлять текстурки, так как просто указатель сделать не выйдет

// короч мы сначало должны распарсить все моды, получить списки ресурсов и прочего
// а затем начать загружать уровень, соствляя из депенданси список объектов необходимых к загрузке
// этот список мы используем чтобы проверить что есть а чего нет

// после загрузки ни структуры с ресурсами ни структуры с конфликтами нам будут больше не нужны
// и мы по идее легко можем от них избавиться
// нам нужно будет только сохранить какое то совсем малое количество данных
// по идее только имя ресурса для того чтобы его анлоадить
// усложнит ли это хоть чтото? по сути тогда не нужно будет хранить информацию в лоадерах
// хотя мне наверное и так и сяк так поступать придется
// поэтому наверное будем удалять
// как тогда он будет загружать следующий уровень? при загрузке просто придется опять распарсить 
// данные об уровне
// опять получается какая то хрень, мы должны распарсить моды и из них получить последовательность уровней
// эта последовательность уже может создать конфликт

// ну и да, чтобы избежать проблем с доступом к ресурсам из луа, например, нужно все же давать абсолютные имена
// с другой стороны это чуть чуть усложнит жизнь мододелам 

// так, подойти к вопросу с другой стороны
// дествительно заиметь парсер, который превратит все json'ы в удобные штуки
// и по этим удобным штукам и начинать загружать все ресурсы
// а потом этот парсер можно просто удалить
// то есть я сначало парсю все это дело...
// ну в общем это ничем не отличается от того что у меня уже есть
// парсер у меня тоже можно удалить... где хранить уже загруженные вещи?
// как определить что можно удалить?

// так короче
// нужно сделать здесь ТОЛЬКО парсер, который передает кусочки для парсинга в лоадеры
// вернуть мне нужно только имя ресурса (или даже это мне ненужно)
// лоадер сам уже разберется как ему удобнее всего загрузить
// + лоадеры потом будут составлять иерархию, которая при загрузке должна постепенно разворачиваться
// таким образом карта основывается на объектах в в ней, объекты в свою очередь основываются на текстурах и прочих данных
// и вот так идет загрузка уровня
// вынесем метод лоад левел за пределы парсера
// после загрузки мы должны почистить лоадеры
// и видимо при загрузке нового уровня мы будем все заново парсить
// то есть нужно хранить список загруженных модов 

#define DEFAULT_CONFLICT_COUNT 100
#define DEFAULT_RESOURCE_COUNT 1000

enum ErrorType {
  // gamedata errors
  DUBLICATE_ERROR = 0,
  DUBLICATE_PREFIX,
  COULD_NOT_LOAD_FILE,
  // texture errors
  MISSED_TEXTURE_PATH,
  TOO_MUCH_TEXTURE_COUNT,
  MISSED_TEXTURE_SIZE,
  BAD_TEXTURE_TYPE,
  NUM_ERRORS
};

enum WarningType {
  MISSED_TEXTURE_NAME = 0,
  REDUNDANT_DATA,
  NUM_WARNINGS
};

struct ErrorDesc {
  ErrorDesc() {}
  ErrorDesc(const ErrorType &type, const std::string &desc) : type(type), description(desc) {}
  
  ErrorType type;
  std::string description;
};

struct WarningDesc {
  WarningDesc() {}
  WarningDesc(const WarningType &type, const std::string &desc) : type(type), description(desc) {}
  
  WarningType type;
  std::string description;
};

class Conflict;

struct Resource {
  std::string name; // (или лучше путь? лучше наверное и то и другое)
  std::string path;
  // группа?
  //std::string group;
  size_t size; // в байтах 
  // тут возможно нужно будет еще прописать тип ресурса
  
  // тут еще потребуются данные json'а, потому что нужно 
  // передать их в лоадер
  // вот только json нам нужен только на этапе загрузки
  // потом же он будет только место занимать
  nlohmann::json data;
  
  Conflict* conflict;
  
  // пока не уверен как это заполнить
  std::vector<std::string> dependencies;
};

struct Plugin {
  std::string name;
  std::string desc;
  // юзер данные 
  
  // тут наверное все же должны быть имена
  //std::vector<Conflict*> relatedResources;
  //std::vector<std::string> relatedResources;
  std::vector<Resource*> relatedResources;
};

class Conflict {
public:
  Conflict(Resource* defaultRes) : choosenVar(SIZE_MAX), nameStr(defaultRes->name) {
    variants.push_back(defaultRes);
  }
  
  Conflict(const std::string &name) : choosenVar(SIZE_MAX), nameStr(name) {
    variants.push_back(nullptr);
  }
  
  bool exist() const { return variants.size() > 1; }
  bool mainData() const { return variants[0] != nullptr; }
  bool solved() const { return choosenVar != SIZE_MAX; }
  
  // нужно еще как-нибудь вывести инфу о конфликте
  void print(const std::string &indent) const {
    std::cout << indent << nameStr << "\n";
    
    const size_t index = getChoosenIndex();
    for (size_t i = 0; i < variants.size(); ++i) {
      if (index == i) std::cout << indent + indent << variants[i]->name << " " << "<---" << "\n";
      else std::cout << indent + indent << variants[i]->name << "\n";
    }
  }
  
  void setMainData(Resource* res) {
    variants[0] = res;
  }
  
  void choose(const size_t &index) {
    if (index >= variants.size()) return;
    
    choosenVar = index;
  }
  
  const std::vector<Resource*> & getVariants() const { return variants; }
  
  void addVariant(Resource* res) {
    variants.push_back(res);
  }
  
  void removeVariant(Resource* res) {
    for (auto itr = variants.begin(); itr != variants.end(); ++itr) {
      if ((*itr) == res) {
        variants.erase(itr);
        break;
      }
    }
  }
  
  size_t getChoosenIndex() const {
    return choosenVar == SIZE_MAX ? variants.size()-1 : choosenVar;
  }
  
  Resource* getChoosenResource() { 
    const size_t idx = getChoosenIndex();
    
    return variants[idx];
  }
  
  std::string name() const {
    return nameStr;
  }
private:
  //Resource* mainResource;
  size_t choosenVar;
  std::string nameStr;
  std::vector<Resource*> variants;
};

struct LoadingStateData {
  MemoryPool<Resource, sizeof(Resource)*DEFAULT_RESOURCE_COUNT> resources;
  MemoryPool<Conflict, sizeof(Conflict)*DEFAULT_CONFLICT_COUNT> conflicts;
  std::vector<Resource*> resourceArray;
  std::vector<Conflict*> conflictsArray;
  
  // тут еще добавить анордеред мап наверное нужно
  std::unordered_map<std::string, Resource*> resourceNames;
  std::unordered_map<std::string, Conflict*> conflictsNames;
};

struct LoadedStateData {
  std::vector<std::string> names;
};

class PluginParser;

// тут наверное потребуется еще как то выводить сообщения об ошибках
// точнее хотя бы их собрать, не все ошибки критические 
// (например, если у мода отсутствует файл, но мы можем не выкидывать игрока из меню)
// ко всему прочему неплохо было бы четко раскидать ресурс к моду
// вообще у нас будет список в самих модах
// в общем нужны дополнительные данные
// конфликты и ресурсы поди нужно возвращать по именам

class ResourceParser {
public:
  virtual ~ResourceParser() {}
  
  virtual bool parse(const std::string &pathPrefix, 
                     const std::string &forcedNamePrefix, 
                     const nlohmann::json &data, 
                     std::vector<Resource*> &resource, 
                     std::vector<ErrorDesc> &errors, 
                     std::vector<WarningDesc> &warnings) = 0;
  virtual bool forget(const std::string &name) = 0;
  
  //virtual bool load(Conflict* res) = 0;
  //virtual bool unload(Conflict* res) = 0;
  //virtual bool unload(const std::string &name) = 0;
  //virtual void unloadAll() = 0;
  //virtual void end() = 0; // фиксируем изменения
  
  //virtual void clear() = 0;
  
  //virtual std::vector<std::string> getLoadedResourceNames() = 0;
//   virtual std::vector<Resource*> getLoadedResource() = 0;
//   virtual std::vector<Conflict*> getConflicts() = 0;
  
  virtual std::unordered_map<std::string, Resource*> getLoadedResource() = 0;
  virtual std::unordered_map<std::string, Conflict*> getConflicts() = 0;
  
  //virtual size_t overallState() const = 0;
  //virtual size_t loadingState() const = 0;
  //virtual std::string hint() const = 0;
};

// class Parser {
// public:
//   virtual ~Parser() {}
//   
//   virtual Resource* parse(const nlohmann::json &data) = 0;
// };

class PluginParser {
public:
  virtual ~PluginParser() {}
  
  //virtual void add(Manager* manager) = 0;
  virtual void add(ResourceParser* parser) = 0;
  virtual void set(const std::vector<ResourceParser*> &parsers) = 0;
  
  //virtual Resource* createResource() = 0;

  // загружаем какую-то часть + обходим все дочерние менеджеры
  // возможно тут будем возвращать какой нибудь ресурсХендл
  // хотя скорее всего нет
  // неплохо было бы еще как нибудь отслеживать состояние загрузки (float)
  //virtual bool load() = 0; // тут никаких особых выходных данных
  // может быть еще потребуется метод unload или что то такое
  // который должен выгружать ресурс по требованию
  // вот только как его адекватно сделать?
  // это в основном потребуется только тогда когда у нас есть какая то динамическая прогрузка мира
  // да и то в этом случае должен быть механизм распознавания какой ресурс больше не используется
  // динамическая прогрузка, ктстати, плохо коррелирует с тем 
  // что я хочу чтобы ресурсы также использовались и в гпу
  
  // для начала мы должны распарсить json, во первых для того чтобы составить список того что мы загрузим
  // и посчитать размер и доп данные, а во воторых найти потенциальные конфликты
  // в третьих я должен сказать какие ресурсы оставить, какие загрузить, какие изменить
  virtual void parseJSON(const nlohmann::json &data) = 0;
  // что тут? тут нужно удалить какой-нибудь мод и не использовать его ресурсы, как это сделать?
  // по имени мода? (выглядит логично)
  virtual void unloadJSON(const std::string &modName) = 0;
  
  // если по каким-то причинам лоад должен запуститься несколько раз за одну загрузочную сессию
  // то эти методы точно должны запуститься единожды для каждой загрузочной сессии
  // теперь с parseJSON это может и не потребоваться (нет, наверное все же потребуется)
  //virtual void begin() = 0; // этого наверное не будет
  //virtual void end() = 0; // здесь у нас постзагрузочные действия призводятся
  
  // также должны быть методы возвращающие какие-то общие данные
  // + гуи
  // прасер json должен выдавать кучу дополнительных данных, а именно:
  //   размер, имя, где находится в памяти, тип (?), тэг, (что-то еще) загружаемого материала
  //   общий размер загрузки, нужно еще прикинуть размер всех объектов в памяти (ну то есть количество монтров * на размер их в памяти)
  virtual std::vector<Resource*> getResources() const = 0; // лист?
  virtual size_t getOverallSize() const = 0;
  virtual size_t getHostSize() const = 0;
  
  virtual std::vector<Resource*> getPluginResources(const std::string &name) const = 0;
  
  // также менеджер должен возвращать конфликты, так чтобы с ними мог разобраться человек
  // это наверное в гуи будет упаковано
  virtual std::vector<Conflict*> conflicts() const = 0;
  
  // также нам нужно вернуть данные о текущих загруженных модах
  virtual std::vector<Plugin> plugins() const = 0;
  
  // мы должны загружать ресурсы нужные только уровню!!! остальные просто держать в какой-нибудь таблице
  // короч у нас есть не очень сложная иерархия, где на вершине стоит карта, а от нее мы узнаем какие вещи нам нужно загрузить в данный момент
  // у нас должна быть возможность загрузить конкретный уровень и в нем будут указаны ресурсы которые мы должны загрузить
  //virtual bool loadLevel(const size_t &index) = 0; // что-то вроде такого
  
  // как сделать полосу загрузки при этом? для полосы загрузки достаточно
  // одного float значения, которое обозначет степень завершения
  // как его получить? я подумал о том что нужно держать два числа
  // условный объем загружаемых вещей, и объем того что уже загружено
  // как это посчитать? задать какие то значения по умолчанию?
  // прежде чем загрузить, разпарсить json? ну кстати это здравая идея
  // так как мне нужно будет еще составлять список конфликтов
  // короч видимо будет так:
  //   парсим json
  //   считаем число
  //   делаем сопутствующую работу
  //   когда приходит время непосредственной загрузки вычисляем степень готовности по этому числу
  //virtual size_t overallState() const = 0;
  //virtual size_t loadingState() const = 0;
  // возможно также сделать какой-нибудь тут текстовый хинт
  //virtual std::string hint() const = 0;
  
  // также нужно подумать о том как верно передать изменения в системы
  // все пересобирать может и вариант, но только тогда когда мало ресурсов требуется
  // что делать в случае когда требуется динамчески подгружать?
  // хранить массивы данных в менеджерах и в них все изменения проводить?
  // проблема возникает например с массивами сформированными после системы анимации
  // другое дело, должно быть некое конечное место ресурса, 
  // которое используется системой и идет дальше по движку
  // то есть при загрузке чего бы то ни было менеджер
  // обновляет систему (системы) и эта система получает верные и обновленные данные
  // это более менее похоже на правду, непонятно правда пока что как это будет выглядеть на практике
  
  // как должен быть организован доступ в ресурсы мода со стороны пользователя?
  // 
};

#endif
