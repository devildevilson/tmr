#include "Helper.h"

class LogCounter {
public:
  LogCounter(const std::string &log) {
    std::cout << log << " #" << counter << "\n";
    ++counter;
  }
private:
  static size_t counter;
};

size_t LogCounter::counter = 0;

enum class game_state {
  menu, // причем неплохо было бы сделать проигрывание демки в главном меню
  loading,
  game
};

int main(int argc, char** argv) {
  TimeLogDestructor appTime("Shutdown");
  
  //std::locale::global(std::locale("ru_RU.UTF-8"));
  std::locale::global(std::locale("en_US.UTF-8"));

  // тут мы создаем дополнительные вещи, такие как консоль, команды, переменные
  Global::init();
  Global::commands()->addCommand("set", cvar::set_f);
  Global::commands()->addCommand("get", cvar::get_f);

  cvar debugDraw("debugDraw", float(0), VAR_ARCHIVE);

  // нам нужно еще создать settings
  {
    Global g;
    g.setGameDir(FileTools::getAppDir());

    //std::cout << Global::getGameDir() << "\n";
  }

  Settings settings;
  {
    Global g;
    g.setSettings(&settings);
    
    cppfs::FileHandle fh = cppfs::fs::open(Global::getGameDir() + "settings.json");

    if (fh.exists()) {
      // грузим настройки
      settings.load("game", fh.path());
    } else {
      throw std::runtime_error("Settings not found");
    }
  }

  // у нас тут еще есть создание треад пула
  // причем неплохо было бы создавать его только тогда когда системы его используют
  // например если в системе два треда (в этом случае скорее всего тоже норм)
  // если в настройках определено не использовать треад пул
  // то нам его создавать ни к чему
  // чаще всего пул необходим, не могу честно говоря представить когда он не нужен в современных условиях
  //std::cout << "std::thread::hardware_concurrency() " << std::thread::hardware_concurrency() << "\n";
  dt::thread_pool threadPool(std::max(std::thread::hardware_concurrency()-1, uint32_t(1)));

  initGLFW();

  // тут мы грузим с диска настройки систем (это скорее всего json)
  // обходим его и решаем какую систему лучше создать + собираем сайз
  // тип: settings.json, hidden_settings.json и прочее

  // еще где то здесь мы обрабатываем входные данные с консоли (забыл как это называется верно лол)

  GraphicsContainer graphicsContainer;

  // теперь создаем контейнер sizeof(ParticleSystem) + sizeof(DecalSystem) +
  const size_t &systemsSize = sizeof(VulkanRender) + sizeof(PostPhysics) + sizeof(CPUAnimationSystemParallel) + sizeof(SoundSystem) + sizeof(CPUAISystem) + sizeof(StateControllerSystem) + sizeof(InteractionSystem) + sizeof(EffectSystem) + sizeof(AttributeSystem);
  GameSystemContainer systemContainer(systemsSize);

  {
    const size_t stageContainerSize = sizeof(BeginTaskStage) + sizeof(EndTaskStage) + sizeof(MonsterGPUOptimizer) + sizeof(GeometryGPUOptimizer) + 
                                      sizeof(GBufferStage) + sizeof(DefferedLightStage) + sizeof(ToneMappingStage) + sizeof(CopyStage) + sizeof(PostRenderStage);

    GraphicsContainer::CreateInfo info{
      stageContainerSize,
      &systemContainer
    };
    graphicsContainer.construct(info);
  }

  // так же нам нужно будет загрузить какие-нибудь ресурсы
  // загрузчик ресурсов поди будет и их менеджером
  // (ну точнее загрузчик и менеджер будут все же разными, но обитать очень близко друг к другу)
  // мы в этот же загрузчик наверное поместим загрузку хардкодных данных
  // как тогда это должно выглядеть?
  // наряду с системами типа консоли, должна быть система загрузки, но должна она быть
  // собираема из разных загрузчиков
  // для этих загрузчиков (менеджеров) могут потребоваться кое какие вещи из систем (например рендер девайс)
  // следовательно системы и менеджеры должны уметь взаимодействовать друг с другом
  // (может не напрямую, но тем не менее)

  // здесь грузим какой-нибудь стейт по умолчанию
  // это скорее всего будет менюшка
  // кстати в менюшке должно быть переключение на демку
  // то есть рендер то поди один,
  // только нужен какой нибудь метод для отрисовки единичной текстуры на весь экран
  // см. ранее
  // скорее всего для разных состояний приложения у нас разные контейнеры с ресурсами
  // типо для менюшки (для гуи) и общий контейнер, по идее никаких особых проблем с этим у нас не будет
  // таким образом не нужно будет менять рендер, скорее всего мне нужно будет чутка изменить текстур лоадер

  // как создать буфферы с данными (трансформа, ротация, матрица и прочее)?
  // создать их здесь - будет куча лишних указателей (или не лишних??)
  // удаление я могу упрятать в отдельный контейнер, когда окончательно пойму сколько массивов мне нужно будет
  DataArrays arrays;
  ArrayContainers arraysContainer(sizeof(GPUContainer<ExternalData>) + //CPUContainer
                                  sizeof(GPUContainer<InputData>) +
                                  sizeof(GPUContainer<simd::mat4>) +
                                  sizeof(GPUBuffer<uint32_t>) +
                                  sizeof(GPUContainer<RotationData>) +
                                  sizeof(GPUContainer<Transform>) +
                                  sizeof(GPUContainer<Texture>) +
                                  sizeof(GPUContainer<AnimationState>));
                                  //sizeof(GPUArray<BroadphasePair>));
  createDataArrays(graphicsContainer.device(), arraysContainer, arrays);

  // тут по идее мы должны создать оптимизеры
  OptimiserContainer optContainer(sizeof(MonsterOptimizer) + sizeof(GeometryOptimizer) + sizeof(LightOptimizer) + sizeof(MonsterDebugOptimizer) + sizeof(GeometryDebugOptimizer));
  MonsterOptimizer* mon = nullptr;
  GeometryOptimizer* geo = nullptr;
  LightOptimizer* lights = nullptr;
  MonsterDebugOptimizer* monDebug = nullptr;
  GeometryDebugOptimizer* geoDebug = nullptr;
  {
    // тут мы задаем инпут буфферы
    // аутпут буферы по идее лежат в стейджах
//     mon = optContainer.add<MonsterOptimizer>();
//     geo = optContainer.add<GeometryOptimizer>();
    lights = optContainer.add<LightOptimizer>();
    monDebug = optContainer.add<MonsterDebugOptimizer>();
    geoDebug = optContainer.add<GeometryDebugOptimizer>();

//     mon->setInputBuffers({arrays.transforms, arrays.matrices, arrays.rotationCountBuffer, arrays.rotations, arrays.textures});
//     geo->setInputBuffers({arrays.transforms, arrays.matrices, arrays.rotationCountBuffer, arrays.rotations, arrays.textures});
    lights->setInputBuffers({arrays.transforms});
    monDebug->setInputBuffers({arrays.transforms});

//     GraphicComponent::setOptimizer(mon);
//     GraphicComponentIndexes::setOptimizer(geo);
    Light::setOptimizer(lights);
    GraphicComponent::setDebugOptimizer(monDebug);
    GraphicComponentIndexes::setDebugOptimizer(geoDebug);

    Global::render()->addOptimizerToClear(lights);
    Global::render()->addOptimizerToClear(monDebug);
    Global::render()->addOptimizerToClear(geoDebug);
  }
  
  const size_t updateDelta = DELTA_TIME_CONSTANT;

  // создадим физику (почти везде нужно собрать сайз для контейнера)
//   const size_t physSize = sizeof(CPUOctreeBroadphaseParallel) + sizeof(CPUNarrowphaseParallel) + sizeof(CPUSolverParallel) + sizeof(CPUPhysicsSorter) + sizeof(CPUPhysicsParallel);
//   std::cout << "physSize " << physSize << "\n";
  PhysicsContainer physicsContainer(sizeof(CPUOctreeBroadphaseParallel) + sizeof(CPUNarrowphaseParallel) + sizeof(CPUSolverParallel) + sizeof(CPUPhysicsSorter) + sizeof(CPUPhysicsParallel));
  {
    PhysicsEngine* phys;
    createPhysics(&threadPool, arrays, updateDelta, physicsContainer, &phys);
    //Global::phys2 = phys;
    Global g;
    g.setPhysics(phys);
  }

  // создадим систему ии
  createAI(&threadPool, updateDelta, systemContainer);
  createBehaviourTrees();

  // создадим еще какое то количество систем
  // и заодно парсеры для них
  // затем мы все это дело добавим в контейнер
//  DecalSystem* decalSystem = nullptr;
  StateControllerSystem* statesSys = nullptr;
  {
    CPUAnimationSystemParallel* animSys = systemContainer.addSystem<CPUAnimationSystemParallel>(&threadPool);

    Global g;
    g.setAnimations(animSys);
    
    statesSys = systemContainer.addSystem<StateControllerSystem>(StateControllerSystem::CreateInfo{&threadPool});

    // что тут еще надо создать?
    // еще будет боевая система (там нужно будет придумать быстрый и эффективный способ вычислить весь (почти) урон)
    // система эффектов, хотя может быть очень родствена боевой
    // что еще? частицы посчитать?
    // у меня будет скорее не боевая система, а система которая будет обрабатывать большое количество локальных эвентов параллельно
    // то есть в нее будут входить много разных подсистем которые будут так или иначе запускать эвенты
    // от ии будет отличаться тем что вызов эвентов может пересекать друг друга
    
//    ParticleSystem* particleSystem = systemContainer.addSystem<ParticleSystem>(graphicsContainer.device());
//    g.setParticles(particleSystem);
//
//    DecalSystem::setContainer(arrays.matrices);
//    DecalSystem::setContainer(arrays.transforms);
//
//    decalSystem = systemContainer.addSystem<DecalSystem>();
    // глобальный указатель?
  }

  // создадим лоадер (лоадер бы лучше в контейнере создавать) sizeof(ParserHelper) +
  const size_t loaderContainerSize = sizeof(ImageLoader) + sizeof(SoundLoader) + sizeof(EntityLoader) + sizeof(HardcodedMapLoader) + sizeof(AnimationLoader) + sizeof(AttributesLoader) + sizeof(EffectsLoader) + sizeof(AbilityTypeLoader) + sizeof(ItemTypeLoader) + sizeof(ModificationContainer);
  ParserContainer loaderContainer(loaderContainerSize);
  ImageLoader* textureLoader = nullptr;
  SoundLoader* soundLoader = nullptr;
  AnimationLoader* animationLoader = nullptr;
  EntityLoader* entityLoader = nullptr;
  ItemTypeLoader* itemLoader = nullptr;
  AbilityTypeLoader* abilityTypeLoader = nullptr;
  AttributesLoader* attributesLoader = nullptr;
  EffectsLoader* effectsLoader = nullptr;
  HardcodedMapLoader* mapLoader = nullptr;
//   HardcodedAnimationLoader* animationLoader = nullptr;
//   HardcodedEntityLoader* entityLoader = nullptr;
//  ParserHelper* parser = nullptr;
  ModificationContainer* mods = nullptr;
  {
    std::unordered_map<std::string, AttributeType<FLOAT_ATTRIBUTE_TYPE>::FuncType> floatComputeFuncs;
    std::unordered_map<std::string, AttributeType<INT_ATTRIBUTE_TYPE>::FuncType> intComputeFuncs;
    
    floatComputeFuncs["default"] = [] (const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, const FLOAT_ATTRIBUTE_TYPE &base, const FLOAT_ATTRIBUTE_TYPE &rawAdd, const FLOAT_ATTRIBUTE_TYPE &rawMul, const FLOAT_ATTRIBUTE_TYPE &finalAdd, const FLOAT_ATTRIBUTE_TYPE &finalMul) {
      (void)float_finder;
      (void)int_finder;
      
      FLOAT_ATTRIBUTE_TYPE value = base;
      
      value *= rawMul;
      value += rawAdd;
      
      value *= finalMul;
      value += finalAdd;
      
      return value;
    };
    
    intComputeFuncs["default"] = [] (const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, const INT_ATTRIBUTE_TYPE &base, const INT_ATTRIBUTE_TYPE &rawAdd, const FLOAT_ATTRIBUTE_TYPE &rawMul, const INT_ATTRIBUTE_TYPE &finalAdd, const FLOAT_ATTRIBUTE_TYPE &finalMul) {
      (void)float_finder;
      (void)int_finder;
      
      INT_ATTRIBUTE_TYPE value = base;
      
      value *= rawMul;
      value += rawAdd;
      
      value *= finalMul;
      value += finalAdd;
      
      return value;
    };
    
    std::unordered_map<std::string, Effect::FuncType> hardcodedComputeFunc;
    std::unordered_map<std::string, Effect::FuncType> hardcodedResistFunc;
    
    std::unordered_map<std::string, AbilityType::ComputeTransformFunction> transFuncs;
    std::unordered_map<std::string, AbilityType::ComputeAttributesFunction> attribsFuncs;
    
    const ImageLoader::CreateInfo tInfo{
      graphicsContainer.device()
    };
    textureLoader = loaderContainer.add<ImageLoader>(tInfo);

    soundLoader = loaderContainer.add<SoundLoader>();
    
    const AnimationLoader::CreateInfo aInfo{
      textureLoader
    };
    animationLoader = loaderContainer.add<AnimationLoader>(aInfo);
    
    const AttributesLoader::CreateInfo attribsInfo{
      floatComputeFuncs,
      intComputeFuncs
    };
    attributesLoader = loaderContainer.add<AttributesLoader>(attribsInfo);
    
    const EffectsLoader::CreateInfo effectsInfo{
      attributesLoader,
      hardcodedComputeFunc,
      hardcodedResistFunc
    };
    effectsLoader = loaderContainer.add<EffectsLoader>(effectsInfo);
    
    const AbilityTypeLoader::CreateInfo abilityInfo{
      attributesLoader,
      effectsLoader,
      transFuncs,
      attribsFuncs
    };
    abilityTypeLoader = loaderContainer.add<AbilityTypeLoader>(abilityInfo);
    
    const ItemTypeLoader::CreateInfo itemInfo{
      abilityTypeLoader,
      effectsLoader
    };
    itemLoader = loaderContainer.add<ItemTypeLoader>(itemInfo);

    const EntityLoader::CreateInfo entInfo{
      textureLoader,
      soundLoader,
      animationLoader,
      attributesLoader,
      effectsLoader,
      abilityTypeLoader,
      itemLoader
    };
    entityLoader = loaderContainer.add<EntityLoader>(entInfo);

    const HardcodedMapLoader::CreateInfo mInfo{
      graphicsContainer.device(),
      entityLoader,
      textureLoader
    };
    mapLoader = loaderContainer.add<HardcodedMapLoader>(mInfo);

    const ModificationContainer::CreateInfo pInfo{
      0
    };
    mods = loaderContainer.addModParser<ModificationContainer>(pInfo);

    mods->addParser(textureLoader);
    mods->addParser(soundLoader);
    mods->addParser(animationLoader);
    mods->addParser(attributesLoader);
    mods->addParser(effectsLoader);
    mods->addParser(abilityTypeLoader);
    mods->addParser(itemLoader);
    mods->addParser(entityLoader);
    mods->addParser(mapLoader);
    
    // я хочу из энда перенести загрузку всего в метод load
    // единственное что мне мешает это сделать сейчас это текстурлоадер
    // если там угадывать количество изображений и какие изображения могут потребоваться
    // то действительно можно перенести всю загрузку из энда в лоад
    // а в энде оставить только непосредственную загрузку текстур
    // на основе этого можно будет сделать стриминговую загрузку данных
    // (ну то есть я надеюсь на то что мне удастся сделать правильно и быстро ротацию текстур)
    // (в пуле, помимо текстур подгрузить 100% нужно будет данные карты и возможно звуки)
    // (все остальные вещи скорее всего будут занимать очень мало места)
    // (постройки тоже поди нужно будет стримить)
    
    // стриминг видимо будет предполагать что мы должны загрузить данные покрайней мере > 256.0f (дальность видимости) расстояния от персонажа
    // вместе с этим придется решить несколько проблем связанных с текущей физикой, нужно будет перескакивать из локации в локацию
    // тут может быть два решения - либо держать несколько экземляров физики одновременно (-оператива), либо писать другую броадфазу
    // нужно посмотреть, сколько занимает физика в текущем виде
  }

  DelayedWorkSystem delaySoundWork(DelayedWorkSystem::CreateInfo{&threadPool});

  {
    TimeLogDestructor soundSystemLog("Sound system initialization");
    // где создать лоадер?

    const SoundSystem::CreateInfo sInfo {
      &threadPool,
      soundLoader,
      &delaySoundWork
    };
    auto* soundSystem = systemContainer.addSystem<SoundSystem>(sInfo);
    Global g;
    g.setSound(soundSystem);

    const SoundLoader::LoadData sound{
      "default_sound",
      Global::getGameDir() + "tmrdata/sounds/Curio feat. Lucy - Ten Feet (Daxten Remix).mp3",
      false,
      false
    };
    soundLoader->load(sound);

//    const SoundLoadingData data{
//      Global::getGameDir() + "tmrdata/sound/Curio feat. Lucy - Ten Feet (Daxten Remix).mp3",
//      SOUND_TYPE_MP3,
//      false,
//      false,
//      true,
//      false,
//      false
//    };
//    Global::sound()->loadSound(ResourceID::get("default_sound"), data);
  }

  nuklear_data data;
  initnk(graphicsContainer.device(), Global::window(), data);

  std::vector<DynamicPipelineStage*> dynPipe;
  {
    const RenderConstructData renderData{
      graphicsContainer.device(),
      &graphicsContainer,
      Global::render(),
      Global::window(),
      mapLoader,
      &arrays,
      mon,
      geo,
      lights,
      &data,
      monDebug,
      geoDebug
    };
    createRenderStages(renderData, dynPipe);
    // переделать рендер, частицы вызовут проблемы у меня сейчас
  }

  // загрузим стейт по умолчанию
  yacs::entity* player = nullptr;
  CameraComponent* camera = nullptr;
  UserInputComponent* input = nullptr;
  TransformComponent* playerTransform = nullptr;
  EntityAI* entityWithBrain = nullptr;
  {
    // теперь мы сначало грузим какие нибудь данные мода
    auto mod = mods->loadModData(Global::getGameDir() + "tmrdata/main.json");

    PRINT_VAR("mod name", mod->name())
    PRINT_VAR("mod description", mod->description())
    PRINT_VAR("mod author", mod->author())
    PRINT_VAR("mod path", mod->path())

    // теперь парсим мод
    mods->parseModification(mod);

    PRINT_VAR("resource count", mod->resources().size())

    // теперь сгружаем это все дело в лоадер
    for (size_t i = 0; i < mod->resources().size(); ++i) {
      textureLoader->load(mods, mod->resources()[i]);
    }

    // попытаемся че нибудь загрузить
//    parser->loadPlugin("tmrdata/main.json");
    // тут список модов к загрузке
    // мы их парсим с помощью parser

    // еще тут нужно передать список того чо надо загрузить
    // то есть например из мапы будут приходить данные для загрузки текстурок
    // и мы постепенно сверху вниз резолвим все ресурсы
    // как тогда загружать мапу? (мне номер какой-то нужен и что то такое)
    // ну то есть парсер хелпер должен все распарсить, данные должны оказаться в нужных лоадерах,
    // и затем мы должны вызвать что нибудь вроде лоад дефаулт левел
//    if (!textureLoader->load("texture")) throw std::runtime_error("Cannot load texture");
//    if (!textureLoader->load("Rough Block Wall")) throw std::runtime_error("Cannot load Rough Block Wall");
//    if (!textureLoader->load("552c34f8e3469")) throw std::runtime_error("Cannot load 552c34f8e3469");
//    if (!textureLoader->load("14626771132270")) throw std::runtime_error("Cannot load 14626771132270");
//    if (!textureLoader->load("7037.970")) throw std::runtime_error("Cannot load 7037.970");
//    if (!textureLoader->load("n")) throw std::runtime_error("Cannot load n");
//    if (!textureLoader->load("ne")) throw std::runtime_error("Cannot load ne");
//    if (!textureLoader->load("e")) throw std::runtime_error("Cannot load e");
//    if (!textureLoader->load("se")) throw std::runtime_error("Cannot load se");
//    if (!textureLoader->load("s")) throw std::runtime_error("Cannot load s");
//    if (!textureLoader->load("sw")) throw std::runtime_error("Cannot load sw");
//    if (!textureLoader->load("w")) throw std::runtime_error("Cannot load w");
//    if (!textureLoader->load("nw")) throw std::runtime_error("Cannot load nw");

//     std::cout << "texture loading" << "\n";
    
    // карта по умолчанию - главное меню, несколько текстур, интерфейс, одна плоскость
//    mapLoader->load("default");
    //levelLoader->load(0);

//     std::cout << "map loading" << "\n";

    textureLoader->end();
//     std::cout << "texture loading 2" << "\n";
    animationLoader->end();

//     std::cout << "texture & animation loading" << "\n";

    //entityLoader->create(); // по идее этот метод будет потом переделан под нужды mapLoader

    entityLoader->end();

    mapLoader->end();

//     std::cout << "entity loading" << "\n";

    // при этом у нас еще должен быть загрузчик демок
    // и этот же механизм нам должен загрузить вещи необходимые для меню (да и вообще для ui)

    // после того как все загрузили чистим загрузчики
//    parser->clear();

    textureLoader->clear();

//     std::cout << "clearing" << "\n";

    player = mapLoader->getPlayer();
    camera = mapLoader->getCamera();
    input = mapLoader->getInput();
    playerTransform = mapLoader->getPlayerTransform();
    entityWithBrain = mapLoader->getEntityBrain();

    // понадобится ли нам указатели на текстур лоадер и прочее, кроме уровней?
    // понадобится для того чтобы обновлять, например, пайплайны
    // возможно понадобится что то еще
    
    throw std::runtime_error("no more");
  }

  PostPhysics* postPhysics = nullptr;
  {
    postPhysics = systemContainer.addSystem<PostPhysics>(&threadPool, player, playerTransform);
  }

//   bool drawMenu = false;
  bool quit = false;
  MenuStateMachine menuContainer(sizeof(MainMenu) + sizeof(ButtonItem) + sizeof(QuitGame));
  {
    MainMenu* menu = menuContainer.addMenuItem<MainMenu>(&data);
    QuitGame* quitGame = menuContainer.addMenuItem<QuitGame>(&data, &quit, menu);
    ButtonItem* button = menuContainer.addMenuItem<ButtonItem>(&data, "Quit game", quitGame, Extent{30, 200});
    menu->addItem(button);
    menuContainer.setDefaultItem(menu);
  }

  KeyContainer keyContainer;

  createReactions({&keyContainer, input, Global::window(), &menuContainer, entityWithBrain});
  setUpKeys(&keyContainer);
  
  // мы може создать дополнительный KeyContainer специально для меню

  for (size_t i = 0; i < dynPipe.size(); ++i) {
    dynPipe[i]->recreatePipelines(textureLoader);
  }

  Global::physics()->setGravity(simd::vec4(0.0f, -9.8f, 0.0f, 0.0f));

  Global::window()->show();
  
  TimeMeter tm(ONE_SECOND);
  while (!Global::window()->shouldClose() && !quit) {
    tm.start();
    glfwPollEvents();

    // чекаем время
    const size_t time = tm.time();

    // где то здесь нам нужно сделать проверки на конец уровня, начало другого (или в синке это делать)
    // еще же статистику выводить (в конце уровня я имею ввиду)
    
    mouseInput(input, time);
    if (menuContainer.isOpened()) menuKeysCallback(&menuContainer);
    else keysCallbacks(&keyContainer, time);
    nextnkFrame(Global::window(), &data.ctx);
    
//     if (Global::data()->keys[GLFW_KEY_M]) std::cout << "M pressed" << "\n";

    camera->update();

    // ии тоже нужно ограничить при открытии менюхи
    Global::ai()->update(time);
    
    if (!menuContainer.isOpened()) Global::physics()->update(time);

    {
      Global g;
      g.setPlayerPos(playerTransform->pos());
    }
    
    // где должны обновляться анимации?
    Global::animations()->update(time);
    statesSys->update(time);

    postPhysics->update(time);

    {
      // пересчитал значения из TimeMeter, получилось что кадр занимает около 4 мс =(
      
      const uint32_t rayOutputCount = Global::physics()->getRayTracingSize();
      const uint32_t frustumOutputCount = Global::physics()->getFrustumTestSize();
      const SimpleOverlayData overlayData{
        playerTransform->pos(),
        playerTransform->rot(),
        tm.accumulatedFrameTime(),
        tm.accumulatedSleepTime(),
        tm.lastIntervalFrameTime(),
        tm.lastIntervalSleepTime(),
        tm.framesCount(),
        tm.fps(),
        frustumOutputCount,
        rayOutputCount,
        0
      };
      nkOverlay(overlayData, &data.ctx);
    }

    // гуи
    menuContainer.draw();
    
    // дебаг
    // и прочее

    graphicsContainer.update(time);
    
    // звук идет после запуска графики
    // сюда же мы можем засунуть и другие вычисления
    Global::animations()->update(time); // ???
    statesSys->update(time);
    
    //     Global::sound()->update(time);
    delaySoundWork.detach_work();

    // здесь же у нас должна быть настройка: тип какую частоту обновления экрана использовать?
    const size_t syncTime = Global::window()->isVsync() ? Global::window()->refreshTime() : 0; //16667
    sync(tm, syncTime);

//    const size_t entityCount = 5000;
//    for (size_t i = entityCount; i < arrays.textures->size(); ++i) {
//      PRINT_VAR("array index", i)
//      PRINT_VAR("image index", arrays.textures->at(i).t.imageArrayIndex)
//      PRINT_VAR("image layer", arrays.textures->at(i).t.imageArrayLayer)
//    }
//
//    throw std::runtime_error("dvdsvlasdvbmk;lbs");

    // неплохо было бы подумать на счет того чтобы отрисовывать часть вещей не дожидаясь свопчейна
    // вообще это означает что мне нужно будет сделать два таск интерфейса и дополнительный семафор
    // правильно их стартануть и закончить + собрать VkSubmitInfo с дополнительным семафором, да и все
    // для этого нам нужно пересмотреть механизм рендер таргетов и стоит отказаться от таргета в котором 3 фреймбуфера
    // каждый фреймбуфер в отделном таргете, так я смогу гораздо гибче подходить к решению проблем
    // и не придется плодить костыли. это откроет для меня возможность использования двойной буферизации
    // (рисуем в картинку, пока показываем другую, верно? у меня сейчас походу не так)
    // для этого придется опять все переписывать =(
    // для этого нужно несколько командных буферов
  }
  
  appTime.updateTime();
  
  glfwSetInputMode(Global::window()->handle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  
  deinitnk(data);
  deinitGLFW();

  cvar::destroy();

  return 0;
}
