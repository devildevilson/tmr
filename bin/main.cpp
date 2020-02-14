#include "Helper.h"

// по хорошему нам бы конечно держать все компоненты в одном месте чтобы избежать этих костылей
#include "GetSpeedTemFunc.h"

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

// проблема такого подхода в том, что нам нужно меню открывать и в самой игре
// причем, рисовать нужно фрейм полностью, и потом дорисовывать сверху меню
// при этом не обновляя некоторые системы (почти все)
// возможно стоит сделать больше стейтов чтобы учесть почти все
enum class game_state {
  main_menu, // причем неплохо было бы сделать проигрывание демки в главном меню
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
  utils::settings settings1;
  {
    Global g;
    g.setSettings(&settings);
    Global::get<const Settings>(&settings);
    Global::get<utils::const_settings>(&settings1);
    ASSERT(Global::get<utils::const_settings>());
    ASSERT(Global::get<utils::settings>() == nullptr);
    
    cppfs::FileHandle fh = cppfs::fs::open(Global::getGameDir() + "settings.json");

    if (fh.exists()) {
      // грузим настройки
      settings.load("game", fh.path());
    } else {
      throw std::runtime_error("Settings not found");
    }
  }
  
//   GetSpeedFunc speed_func_temp_container{
//     [] (yacs::entity* ent) -> float {
//       auto phys = ent->at<PhysicsComponent>(PHYSICS_COMPONENT_INDEX);
//       if (phys.valid()) return phys->getSpeed();
//       return 0.0f;
//     }
//   };
//   Global::get(&speed_func_temp_container);

  // у нас тут еще есть создание треад пула
  // причем неплохо было бы создавать его только тогда когда системы его используют
  // например если в системе два треда (в этом случае скорее всего тоже норм)
  // если в настройках определено не использовать треад пул
  // то нам его создавать ни к чему
  // чаще всего пул необходим, не могу честно говоря представить когда он не нужен в современных условиях
  //std::cout << "std::thread::hardware_concurrency() " << std::thread::hardware_concurrency() << "\n";
  dt::thread_pool threadPool(std::max(std::thread::hardware_concurrency()-1, uint32_t(1)));
  systems::collision_interaction inter(systems::collision_interaction::create_info{&threadPool});

  initGLFW();

  // тут мы грузим с диска настройки систем (это скорее всего json)
  // обходим его и решаем какую систему лучше создать + собираем сайз
  // тип: settings.json, hidden_settings.json и прочее

  // еще где то здесь мы обрабатываем входные данные с консоли (забыл как это называется верно лол)

  // теперь создаем контейнер sizeof(ParticleSystem) + sizeof(DecalSystem) +
  const size_t &systemsSize = sizeof(GraphicsContainer) + sizeof(PostPhysics) + sizeof(systems::sound);
  GameSystemContainer systemContainer(systemsSize);
  GraphicsContainer* graphicsContainer;
  {
    const size_t stageContainerSize = sizeof(BeginTaskStage) + sizeof(EndTaskStage) + sizeof(MonsterGPUOptimizer) + sizeof(GeometryGPUOptimizer) + 
                                      sizeof(GBufferStage) + sizeof(DefferedLightStage) + sizeof(ToneMappingStage) + sizeof(CopyStage) + sizeof(PostRenderStage);

    GraphicsContainer::CreateInfo info{
      stageContainerSize
      //&systemContainer
    };
    graphicsContainer = systemContainer.addSystem<GraphicsContainer>();
    graphicsContainer->construct(info);
    Global::get(graphicsContainer);
  }

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
                                  sizeof(GPUContainer<Texture>));
                                  //sizeof(GPUArray<BroadphasePair>));
  createDataArrays(graphicsContainer->device(), arraysContainer, arrays);

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
//     Light::setOptimizer(lights);
//     GraphicComponent::setDebugOptimizer(monDebug);
//     GraphicComponentIndexes::setDebugOptimizer(geoDebug);

    Global::render()->addOptimizerToClear(lights);
    Global::render()->addOptimizerToClear(monDebug);
    Global::render()->addOptimizerToClear(geoDebug);
    
    Global::get(lights);
    Global::get(monDebug);
    Global::get(geoDebug);
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
    Global::get(phys);
    Global g;
    g.setPhysics(phys);
  }

  // создадим систему ии
  createAI(&threadPool, updateDelta, systemContainer);
  createBehaviourTrees();

  // частицы?
  
  resources_ptr resources(graphicsContainer->device());

  // создадим лоадер (лоадер бы лучше в контейнере создавать) 
  // лоадеры неплохо было бы создавать перед непосредственной загрузкой, пока так создадим
  const size_t loaderContainerSize = sizeof(resources::image_loader) + sizeof(resources::sound_loader) + sizeof(resources::entity_loader) + sizeof(HardcodedMapLoader) + sizeof(resources::attributes_loader) + sizeof(resources::effects_loader) + sizeof(resources::abilities_loader) + sizeof(resources::state_loader); //  + sizeof(ModificationContainer)
  HardcodedMapLoader* mapLoader = nullptr;
  resources::modification_container mods(loaderContainerSize);
  render::image_container images(render::image_container::create_info{graphicsContainer->device()});
  {
    createLoaders(mods, graphicsContainer, &images, resources, &mapLoader);
    
    // теперь мне нужно загружать сразу все ресурсы, кроме всех карт
  }

  //DelayedWorkSystem delaySoundWork(DelayedWorkSystem::CreateInfo{&threadPool});
  utils::delayed_work_system works(utils::delayed_work_system::create_info{&threadPool});
  Global::get(&works);

  createSoundSystem(&threadPool, systemContainer);

  nuklear_data data;
  initnk(graphicsContainer->device(), Global::window(), data);

  std::vector<DynamicPipelineStage*> dynPipe;
  {
    const RenderConstructData renderData{
      graphicsContainer->device(),
      graphicsContainer,
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
  yacs::entity* entityWithBrain = nullptr;
  {
    // теперь мы сначало грузим какие нибудь данные мода
    auto mod = mods.load_mod(Global::getGameDir() + "tmrdata/main.json");

    PRINT_VAR("mod name", mod->name())
    PRINT_VAR("mod description", mod->description())
    PRINT_VAR("mod author", mod->author())
    PRINT_VAR("mod path", mod->path())

    // теперь парсим мод
    mods.parse_mod(mod);

    PRINT_VAR("resource count", mod->size())

    // все ресурсы в лоадеры?
//     for (size_t i = 0; i < mod->resources().size(); ++i) {
//       //textureLoader->load(mods, mod->resources()[i]);
//       loaders[0]->load(mods, mod->resources()[i]);
//     }
// 
//     for (auto loader : loaders) {
//       loader->end();
//     }
//     
//     for (auto loader : loaders) {
//       loader->clear();
//     }
    
    mods.validate();
    mods.load_data();
    mapLoader->end();

//     std::cout << "clearing" << "\n";

    player = mapLoader->getPlayer();
    camera = mapLoader->getCamera();
    input = mapLoader->getInput();
    playerTransform = mapLoader->getPlayerTransform();
    entityWithBrain = mapLoader->getEntityBrain();

    // понадобится ли нам указатели на текстур лоадер и прочее, кроме уровней?
    // понадобится для того чтобы обновлять, например, пайплайны
    // возможно понадобится что то еще
    
    //throw std::runtime_error("no more");
  }

  PostPhysics* postPhysics = nullptr;
  {
    postPhysics = systemContainer.addSystem<PostPhysics>(&threadPool, player, playerTransform);
    Global::get(postPhysics);
  }

//   bool drawMenu = false;
  bool quit = false;
  interface::container menu_container(sizeof(interface::main_menu) + sizeof(interface::quit_game) + sizeof(interface::settings));
  {
    auto menu = menu_container.add_page<interface::main_menu>(interface::main_menu::create_info{"Main menu", &data, nullptr, nullptr, nullptr, {300, 500}});
    auto quitGame = menu_container.add_page<interface::quit_game>(interface::quit_game::create_info{"Quit game", &data, menu, &quit, {300, 500}});
    // мы должны указать положение и видимо использовать не обычное окно, а композитную область для отрисовки всего этого
    menu->set_pointers(quitGame, nullptr, nullptr);
    menu_container.set_default_page(menu);
  }

  KeyContainer keyContainer;

  createReactions({&keyContainer, input, Global::window(), &menu_container, entityWithBrain});
  setUpKeys(&keyContainer);
  
  // мы може создать дополнительный KeyContainer специально для меню
  
  // обновлять textureLoader и пайплайны нужно будет динамически при изменении состояния textureLoader
  for (size_t i = 0; i < dynPipe.size(); ++i) {
    //ImageLoader* textureLoader = static_cast<ImageLoader*>(loaders[0]);
    dynPipe[i]->recreatePipelines(resources.image_res);
  }

  Global::physics()->setGravity(simd::vec4(0.0f, -9.8f, 0.0f, 0.0f));
  
  size_t current_time = 0;
  TimeMeter tm(ONE_SECOND);
  Global::window()->show();
  game_state currentState = game_state::loading;
  interface::overlay overlay(interface::overlay::create_info{&data, player, &tm});
  while (!Global::window()->shouldClose() && !quit) {
    tm.start();
    glfwPollEvents();
    
    switch (currentState) {
      case game_state::loading: {
        // здесь должна быть работа ModificationContainer + небольшой VulkanRender (какое то изображение + статус загрузки)
        // можно ли использовать один VulkanRender или нужно сделать отдельный? 
        // в принципе все что нужно сделать это нарисовать текстуру с координатами, координаты статичные на весь экран
        // + нужно скорректировать матрицу. возможно нужно просто сделать еще один рендер стейдж
        // перед загрузкой неплохо было бы еще вычищать полностью textureLoader и загружать туда только картинку загрузки
        // вычищать нужно для того чтобы там не остались какие то проблемные участки с предыдущего уровня или из меню
        
        break;
      }
      
      case game_state::main_menu: {
        // тут по идее мы должны загрузить все, но никакого уровня загружено быть не должно
        // наверное даже вот как: должен быть загружен уровень меню, состоящий из плоской картинки,
        // камеры и меню, через какое то время загрузка демо
        // вполне возможно что этого стейта просто не будет
        // в думе не этот конкретно стейт использовался, а стейт демо плеинг
        // и просто по умолчанию включалось меню
        
        break;
      }
      
      case game_state::game: {
        // обычный стейт
        
        break;
      }
    }

    // чекаем время
    const size_t time = tm.time();
    ASSERT(time != 0);

    // где то здесь нам нужно сделать проверки на конец уровня, начало другого (или в синке это делать)
    // еще же статистику выводить (в конце уровня я имею ввиду)
    // нужно определить функцию некст левел, которая просто будет выставлять флажок следующего уровня
    // а здесь мы будем проверять и соответственно начинать загружаться
    
    // инпут игрока нужно чуть чуть переделать 
    // (необходимо сделать какой то буфер нажатия клавиш и возможность использовать его в разных местах)
    mouseInput(input, time);
    if (menu_container.is_opened()) menuKeysCallback(&menu_container);
    else keysCallbacks(&keyContainer, time);
    nextnkFrame(Global::window(), &data.ctx);
    
//     if (Global::data()->keys[GLFW_KEY_M]) std::cout << "M pressed" << "\n";

    camera->update();

    // ии тоже нужно ограничить при открытии менюхи
    if (!menu_container.is_opened()) {
      Global::get<PhysicsEngine>()->update(time);
      {
        Global g;
        g.setPlayerPos(playerTransform->pos());
      }
    }

    // обход видимых объектов 
    Global::get<PostPhysics>()->update(time);

    // гуи
    const interface::data::extent screenSize{
      static_cast<float>(Global::window()->size().extent.width), // почему float?
      static_cast<float>(Global::window()->size().extent.height)
    };
    overlay.draw(screenSize); // аттрибуты, иконки, текст, все что не меню (иконка аттрибута?)
    menu_container.proccess(screenSize);
    
    // дебаг
    // и прочее

    // это и звук вполне могут быть сделаны паралельно
    // и вместе с этом я запущу работу в ворксистеме
    // где то рядом нужно обойти всю коллизию 
    Global::get<GraphicsContainer>()->update(time);
    Global::get<systems::sound>()->update_listener(player);
    Global::get<systems::sound>()->update(time);
    
    current_time += time;
    if (current_time < updateDelta || time >= 20000) {
      // во время исполнения этих работ могут добавится еще несколько, но их необходимо обработать уже в следующий раз
      // если здесь добавляются работы которые удаляют какой то объект, то необходимо правильно обновить tree_ai и func_ai
      // это значит что tree_ai и func_ai должны обновляться каждый кадр или detach_works должно работать иначе
      const size_t count = works.works_count();
      works.detach_works(count);
    }
    
    if (current_time >= updateDelta) {
      current_time -= updateDelta;
      if (!menu_container.is_opened()) {
        utils::update<components::states>(&threadPool, updateDelta);
        utils::update<components::attributes>(&threadPool, updateDelta); // здесь скорее всего будем умирать чаще всего
        utils::update<components::effects>(&threadPool, updateDelta);
        utils::update<components::tree_ai>(&threadPool, updateDelta); 
        utils::update<components::func_ai>(&threadPool, updateDelta);
        // функции ниже добавляют работу в works
        // функции коллизии
        inter.update(updateDelta); // не должны ли и они быть синхронизированы по времени? судя по моей физике - да
        // интеракции
        utils::update<core::slashing_interaction>(&threadPool, updateDelta);
        utils::update<core::stabbing_interaction>(&threadPool, updateDelta);
      }
    }

    // здесь же у нас должна быть настройка: тип какую частоту обновления экрана использовать?
    const size_t syncTime = Global::window()->isVsync() ? Global::window()->refreshTime() : 0; //16667
    sync(tm, syncTime);
  }
  
  appTime.updateTime();
  
  glfwSetInputMode(Global::window()->handle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  
  deinitnk(data);
  deinitGLFW();

  cvar::destroy();

  return 0;
}
