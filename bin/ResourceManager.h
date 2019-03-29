#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Manager.h"

// как я понял, мне нужен такой плагин парсер который я после парсинга могу удалить
// и вообще ненужные данные мне после нужно удалять
// но тем не менее должно что то оставаться
// прежде всего это какие то общие данные модов, для того чтобы их выводить после загрузки
// затем мне же еще нужно будет при перезагрузке снова обрабатывать каждый мод

class PluginParserImpl : public PluginParser {
public:
  PluginParserImpl();
  ~PluginParserImpl();
  
  void add(ResourceParser* parser) override;
  void set(const std::vector<ResourceParser*> &parsers) override;
  
  void parseJSON(const nlohmann::json &data) override;
  void unloadJSON(const std::string &modName) override;
  
  //virtual void begin() = 0; // этого наверное не будет
  //virtual void end() = 0; // здесь у нас постзагрузочные действия призводятся
  
  std::vector<Resource*> getResources() const override;
  size_t getOverallSize() const override;
  size_t getHostSize() const override;
  
  std::vector<Resource*> getPluginResources(const std::string &name) const override;
  
  std::vector<Conflict*> conflicts() const override;
  std::vector<Plugin> plugins() const override; // это наверное уйдет отсюда
private:
  std::vector<ResourceParser*> parsers;
  
  std::unordered_map<std::string, Plugin> pluginsMap;
//   LoadingStateData loading;
//   LoadedStateData loaded;
  
  // по идее больше ничего не нужно
  // на данном этапе parseJSON должен работать и с какими то отдельными данными
  // нужно ли будет это после? может и нужно, но только эта возможность наверное уйдет в конкретные лоадеры
  
  // тут еще должна быть возможность загрузить какие-то вещи нужные для 
  // правильного отображения плагина
  
  // но мне все равно нужно как то удалять парсер чтоб не держать все json'ы
  // + хранить выбранные решения
  
  // тут парсеры должны четко задаваться
  // хотя может не тут четко а в parserHelper'е
};

class TextureLoader;
class HardcodedEntityLoader;
class HardcodedMapLoader;

struct PluginCommonData {
  std::string name;
  std::string path;
  std::string desc;
  
  // версия, автор и проч
  // некоторые данные пойдут в сейвфайл для того чтобы
  // загрузить мод при загрузке сейва
//   std::vector<Conflict*> conflicts;
};

struct TemporalData {
  std::unordered_map<std::string, nlohmann::json> modData;
  std::unordered_map<std::string, std::unordered_map<std::string, Conflict*>> conflicts;
  
  std::vector<WarningDesc> warnings;
  std::vector<ErrorDesc> errors;
};

class ParserHelper {
public:
  struct CreateInfo {
    std::string prefix;
    
    TextureLoader* textureLoader;
    HardcodedEntityLoader* entityLoader;
    HardcodedMapLoader* mapLoader;
    // и другие
  };
  
  ParserHelper(const CreateInfo &info);
  ~ParserHelper();
  
  // или тут не путь?
  // мне вообще придется с диска подгрузить как можно больше модов 
  // и тут соответственно можно какие то уже подгруженные данные передавать
  void loadPlugin(const std::string &path); // загружаем интересующие нас плагины
  void loadPlugin(const PluginCommonData &path);
  
  // может быть как раз здесь прогружать все с диска для гуи
  // и так выбирать
  // и так и сяк придется все моды грузить с диска для удобного отображения
  // тут у нас наверное будет сборник уже выбранных модов
  // а где то в другом месте будем обрабатывать все моды
  
  // короч тут наверное тоже будут временные данные, но не будет PluginParserImpl
  // и тип будет метод запарсить все моды, где мы обходим папку с модами и все их чекаем
  // игрок выбирает нужный мод, мы его как-нибудь отмечаем и добавляем к загрузке
  // могут ли у нас быть моды которые изменяют демки? скорее да чем нет
  // но тогда нам нужен какой-нибудь механизм, вроде предзагрузки определенных модов
  void parsePlugins(); // парсим все плагины в папке
  
  void clear();
  
  std::unordered_map<std::string, PluginCommonData> getPluginData() const;
  TemporalData* getTemporalData() const;
private:
  // это у нас будет удаляться после загрузки
  TemporalData* temporal;
//   PluginParserImpl* man;
  
  std::string prefix;
  
  // все же какие то данные нужны
  //std::vector<PluginCommonData> pluginsPaths;
  std::unordered_map<std::string, PluginCommonData> plugins;
//   std::unordered_map<std::string, size_t> resolvedConflicts;
  
  std::vector<ResourceParser*> loaders;
  
  // тут нужно указать конкретные лоадеры, для сбора разных данных
  TextureLoader* textureLoader;
  HardcodedEntityLoader* entityLoader;
  HardcodedMapLoader* mapLoader;
};

// class PluginParser {
// public:
//   
//   void loadPlugin();
//   
//   
// private:
//   ResourceManager* man;
//   
//   std::vector<Loader*> loaders; // тип их мы добавим в парсер ресурсов
//   std::unordered_map<std::string, Plugin> plugins;
//   
//   // тут еще должна быть возможность загрузить какие-то вещи нужные для 
//   // правильного отображения плагина
//   // у нас в менюшке еще должны проигрываться демки, тут ничего сложного по идее нет
//   // демка это тип на фоне рендерится игра + рендерится меню
//   // ну в общем так или иначе у игры должны быть состояния по которым поведение чуть чуть меняется
//   
//   // так стоп, а где тогда брать конфликты????????????
// };

// тут будет еще один парсер
// который должен распарсить только данные о моде 
// + загрузить какие-нибудь иконки мода и ресурсы в этом духе
// основной парсер же должен распарсить вообще все и верно загрузить
// и по идее мне нужно будет его потом удалить, так как он будет занимать много лишней памяти
// может быть мне как-то нужно соединить эти две мысли
// то есть парсер1 парсит данные о модах и сохраняет их у себя на все время игры
// парсер2 при загрузке непосредственно игрового уровня парсит вообще все, но его можно будет потом удалить

// парсер плагинов и парсер ресурсов
// в общем это буллщит все должно быть в одном месте

#endif
