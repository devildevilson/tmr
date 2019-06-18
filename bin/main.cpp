#include "Helper.h"

// #include "../sound/SoundSystem.h"

int main(int argc, char** argv) {
  for (int32_t i = 0; i < argc; ++i) {
    std::cout << argv[i] << "\n";
  }

  (void)argc;
  (void)argv;

//   SoundSystem sys;
//   return 0;

  // тут мы создаем дополнительные вещи, такие как консоль, команды, переменные
//   StageContainer additionalContainer;
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
    //apathy::Path path(Global::getGameDir() + "settings.json");
    cppfs::FileHandle fh = cppfs::fs::open(Global::getGameDir() + "settings.json");

    if (fh.exists()) {
      // грузим настройки
      settings.load(fh.path());
    } else {
      throw std::runtime_error("Settings not found");
    }

    Global g;
    g.setSettings(&settings);
  }

  // у нас тут еще есть создание треад пула
  // причем неплохо было бы создавать его только тогда когда системы его используют
  // например если в системе два треда (в этом случае скорее всего тоже норм)
  // если в настройках определено не использовать треад пул
  // то нам его создавать ни к чему
  //std::cout << "std::thread::hardware_concurrency() " << std::thread::hardware_concurrency() << "\n";
  dt::thread_pool threadPool(std::max(std::thread::hardware_concurrency()-1, uint32_t(1))); // как там его правильно создать?

  initGLFW();

  // тут мы грузим с диска настройки систем (это скорее всего json)
  // обходим его и решаем какую систему лучше создать + собираем сайз
  // тип: settings.json, hidden_settings.json и прочее

  // еще где то здесь мы обрабатываем входные данные с консоли (забыл как это называется верно лол)

//   yavf::Instance instance;
  GraphicsContainer graphicsContainer;

  // теперь создаем контейнер
  const size_t &systemsSize = sizeof(VulkanRender) + sizeof(PostPhysics) + sizeof(CPUAnimationSystemParallel) + sizeof(ParticleSystem) + sizeof(SoundSystem) + sizeof(DecalSystem) + sizeof(CPUAISystem);
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
                                  sizeof(GPUContainer<TextureData>) +
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
  }
  
  const size_t updateDelta = 33333;

  // создадим физику (почти везде нужно собрать сайз для контейнера)
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
  DecalSystem* decalSystem = nullptr;
  {
    CPUAnimationSystemParallel* animSys = systemContainer.addSystem<CPUAnimationSystemParallel>(&threadPool);

    Global g;
    g.setAnimations(animSys);

//     const AnimationSystem::InputBuffers input{
//       nullptr,
//       arrays.transforms
//     };
//     animSys->setInputBuffers(input);
// 
//     const AnimationSystem::OutputBuffers output{
//       arrays.textures
//     };
//     animSys->setOutputBuffers(output);

    // что тут еще надо создать?
    // еще будет боевая система (там нужно будет придумать быстрый и эффективный способ вычислить весь (почти) урон)
    // система эффектов, хотя может быть очень родствена боевой
    // что еще? частицы посчитать?
    
    ParticleSystem* particleSystem = systemContainer.addSystem<ParticleSystem>(graphicsContainer.device());
    g.setParticles(particleSystem);
    
    DecalSystem::setContainer(arrays.matrices);
    DecalSystem::setContainer(arrays.transforms);
    
    decalSystem = systemContainer.addSystem<DecalSystem>();
    // глобальный указатель?
    
    SoundSystem* soundSystem = systemContainer.addSystem<SoundSystem>();
    g.setSound(soundSystem);
  }
  
  {
    const SoundLoadingData data{
      Global::getGameDir() + "tmrdata/sound/Omen - Kingdom Under Siege.mp3",
      SOUND_TYPE_MP3,
      false,
      false,
      true,
      false,
      false
    };
    Global::sound()->loadSound(ResourceID::get("default_sound"), data);
  }

  // создадим лоадер (лоадер бы лучше в контейнере создавать) sizeof(ParserHelper) +
  const size_t loaderContainerSize = sizeof(TextureLoader) + sizeof(HardcodedEntityLoader) + sizeof(HardcodedMapLoader) + sizeof(HardcodedAnimationLoader);
  ParserContainer loaderContainer(loaderContainerSize);
  TextureLoader* textureLoader = nullptr;
  HardcodedAnimationLoader* animationLoader = nullptr;
  HardcodedEntityLoader* entityLoader = nullptr;
  HardcodedMapLoader* mapLoader = nullptr;
  ParserHelper* parser = nullptr;
  {
    const TextureLoader::CreateInfo tInfo{
      graphicsContainer.device()
    };
    textureLoader = loaderContainer.add<TextureLoader>(tInfo);

    entityLoader = loaderContainer.add<HardcodedEntityLoader>(textureLoader);

    const HardcodedMapLoader::CreateInfo mInfo{
      graphicsContainer.device(),
      entityLoader,
      textureLoader
    };
    mapLoader = loaderContainer.add<HardcodedMapLoader>(mInfo);

    const ParserHelper::CreateInfo pInfo{
      Global::getGameDir(),
      textureLoader,
      entityLoader,
      mapLoader
    };
    parser = loaderContainer.addParserHelper(pInfo);

    const HardcodedAnimationLoader::CreateInfo aInfo{
      textureLoader
    };
    animationLoader = loaderContainer.add<HardcodedAnimationLoader>(aInfo);
  }

  nuklear_data data;
  initnk(graphicsContainer.device(), Global::window(), data);

  // здесь еще должно быть создание рендер стейджев
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

    // вот так примерно мы собираем рендер (уберем в отдельную функцию)
  }

  // загрузим стейт по умолчанию
  yacs::Entity* player = nullptr;
  CameraComponent* camera = nullptr;
  UserInputComponent* input = nullptr;
  TransformComponent* playerTransform = nullptr;
  {
    // попытаемся че нибудь загрузить
    parser->loadPlugin("tmrdata/main.json");
    // тут список модов к загрузке
    // мы их парсим с помощью parser

    // еще тут нужно передать список того чо надо загрузить
    // то есть например из мапы будут приходить данные для загрузки текстурок
    // и мы постепенно сверху вниз резолвим все ресурсы
    // как тогда загружать мапу? (мне номер какой-то нужен и что то такое)
    // ну то есть парсер хелпер должен все распарсить, данные должны оказаться в нужных лоадерах,
    // и затем мы должны вызвать что нибудь вроде лоад дефаулт левел
    if (!textureLoader->load("texture")) throw std::runtime_error("Cannot load texture");
    if (!textureLoader->load("Rough Block Wall")) throw std::runtime_error("Cannot load Rough Block Wall");
    if (!textureLoader->load("552c34f8e3469")) throw std::runtime_error("Cannot load 552c34f8e3469");
    if (!textureLoader->load("14626771132270")) throw std::runtime_error("Cannot load 14626771132270");
    if (!textureLoader->load("7037.970")) throw std::runtime_error("Cannot load 7037.970");
    if (!textureLoader->load("n")) throw std::runtime_error("Cannot load n");
    if (!textureLoader->load("ne")) throw std::runtime_error("Cannot load ne");
    if (!textureLoader->load("e")) throw std::runtime_error("Cannot load e");
    if (!textureLoader->load("se")) throw std::runtime_error("Cannot load se");
    if (!textureLoader->load("s")) throw std::runtime_error("Cannot load s");
    if (!textureLoader->load("sw")) throw std::runtime_error("Cannot load sw");
    if (!textureLoader->load("w")) throw std::runtime_error("Cannot load w");
    if (!textureLoader->load("nw")) throw std::runtime_error("Cannot load nw");

//     std::cout << "texture loading" << "\n";

    mapLoader->load("default");
    //levelLoader->load(0);

//     std::cout << "map loading" << "\n";

    textureLoader->end();
//     std::cout << "texture loading 2" << "\n";
    animationLoader->end();

//     std::cout << "texture & animation loading" << "\n";

    entityLoader->create(); // по идее этот метод будет потом переделан под нужды mapLoader

    entityLoader->end();

    mapLoader->end();

//     std::cout << "entity loading" << "\n";

    // при этом у нас еще должен быть загрузчик демок
    // и этот же механизм нам должен загрузить вещи необходимые для меню (да и вообще для ui)

    // после того как все загрузили чистим загрузчики
    parser->clear();

//     std::cout << "clearing" << "\n";

    player = entityLoader->getPlayer();
    camera = entityLoader->getCamera();
    input = entityLoader->getInput();
    playerTransform = entityLoader->getPlayerTransform();

    // понадобится ли нам указатели на текстур лоадер и прочее, кроме уровней?
    // понадобится для того чтобы обновлять, например, пайплайны
    // возможно понадобится что то еще
  }

  PostPhysics* postPhysics = nullptr;
  {
    postPhysics = systemContainer.addSystem<PostPhysics>(&threadPool, player, playerTransform);
  }

//   std::cout << "input ptr " << input << "\n";

//   initGui(device, window);

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

  KeyConfig keyConfig;
  KeyContainer keyContainer(&keyConfig);

  createReactions({&keyContainer, input, Global::window(), &menuContainer});
  setUpKeys(&keyContainer);

  for (size_t i = 0; i < dynPipe.size(); ++i) {
    dynPipe[i]->recreatePipelines(textureLoader);
  }

  Global::physics()->setGravity(simd::vec4(0.0f, -9.8f, 0.0f, 0.0f));

  Global::window()->show();
  //Global::window()->toggleVsync();
  
  // нужно сделать настраиваемым (нет, не нужно)
//   const size_t updateDelta = 33333;
//   size_t accumulator = 0;
  
  TimeMeter tm(1000000);
  while (!Global::window()->shouldClose() && !quit) {
    tm.start();
    glfwPollEvents();

    // std::cout << "\n";

    // RegionLog rl("everything", true);

    // чекаем время
    const uint64_t time = tm.getTime();

//     std::cout << "time " << time << "\n";

    // где то здесь нам нужно сделать проверки на конец уровня, начало другого (или в синке это делать)
    // еще же статистику выводить (в конце уровня я имею ввиду)

    // чекаем инпут (хотя наверное можно его чекать и в другое время)
    //if (!Global::data()->focusOnInterface)

    {
//       RegionLog rl("input + camera", true);

      {
        {
          // RegionLog rl("mouse input");
          mouseInput(input, time);
        }

        {
          // RegionLog rl("key input");
          if (menuContainer.isOpened()) menuKeysCallback(&menuContainer);
          else keysCallbacks(&keyConfig, time);
        }

        nextnkFrame(Global::window(), &data.ctx);
      }

      camera->update(time);
    }

    Global::ai()->update(time);

    {
      // RegionLog rl("physics region");
      
      if (!menuContainer.isOpened()) Global::physics()->update(time);

      postPhysics->update(time);
    }

    {
      // RegionLog rl("animation region", true);

      // мне же еще нужно изменить глобал переменную с позицией игрока
      Global::animations()->update(time);
    }

    {
      const uint32_t rayOutputCount = Global::physics()->getRayTracingSize();
      const uint32_t frustumOutputCount = Global::physics()->getFrustumTestSize();
//       const uint32_t rayOutputCount = 0;
//       const uint32_t frustumOutputCount = 0;
//
//       simd::vec4 pos = simd::vec4(0.0f);
//       simd::vec4 rot = simd::vec4(0.0f);

      const SimpleOverlayData overlayData{
        playerTransform->pos(),
//         pos,
        playerTransform->rot(),
//         rot,
        tm.getReportTime(),
        tm.getSleepTime(),
        tm.getFPS(),
        frustumOutputCount,
        rayOutputCount,
        0
      };
      nkOverlay(overlayData, &data.ctx);
    }
    
//     Global::sound()->update(time);

    // гуи
    menuContainer.draw();
    
    // дебаг
    // и прочее

    {
      // RegionLog rl("graphics region");

      graphicsContainer.update(time);

//     window.nextFrame();
//     Global::render()->update(time);
//
//     Global::render()->start();
//     window.present(); // по идее нет никакой разницы где это стоит


    }

    // здесь же у нас должна быть настройка: тип какую частоту обновления экрана использовать?
    //(Global::window()->isFullscreen() ?  : 16666)
    const size_t syncTime = Global::window()->isVsync() ? Global::window()->refreshTime() : 0; //16667
    //std::cout << "syncTime " << syncTime << "\n";
    sync(tm, syncTime);
    
//     std::cout << "instanceData.size() " << instanceData.size() << "\n";
//     for (size_t i = 0; i < std::min(instanceData.size(), uint32_t(5)); ++i) {
//       PRINT_VEC4("inst "+std::to_string(i)+" mat[0] ", instanceData[i].mat[0])
//       PRINT_VEC4("inst "+std::to_string(i)+" mat[1] ", instanceData[i].mat[1])
//       PRINT_VEC4("inst "+std::to_string(i)+" mat[2] ", instanceData[i].mat[2])
//       PRINT_VEC4("inst "+std::to_string(i)+" mat[3] ", instanceData[i].mat[3])
//       
//       std::cout << "inst " << i << " image index    " << instanceData[i].textureData.t.imageArrayIndex << "\n";
//       std::cout << "inst " << i << " image layer    " << instanceData[i].textureData.t.imageArrayLayer << "\n";
//       std::cout << "inst " << i << " image sampler  " << instanceData[i].textureData.t.samplerIndex << "\n";
//       std::cout << "inst " << i << " image move U   " << instanceData[i].textureData.movementU << "\n";
//       std::cout << "inst " << i << " image move V   " << instanceData[i].textureData.movementV << "\n";
//     }
//     
//     throw std::runtime_error("end");

    // неплохо было бы подумать на счет того чтобы отрисовывать часть вещей не дожидаясь свопчейна
    // вообще это означает что мне нужно будет сделать два таск интерфейса и дополнительный семафор
    // правильно их стартануть и закончить + собрать VkSubmitInfo с дополнительным семафором, да и все
    // для этого нам нужно пересмотреть механизм рендер таргетов и стоит отказаться от таргета в котором 3 фреймбуфера
    // каждый фреймбуфер в отделном таргете, так я смогу гораздо гибче подходить к решению проблем
    // и не придется плодить костыли. это откроет для меня возможность использования двойной буферизации
    // (рисуем в картинку, пока показываем другую, верно? у меня сейчас походу не так)
    // для этого придется опять все переписывать =(

//     throw std::runtime_error("1 frame");
  }

//   Global::data()->focusOnInterface = false;
//   nextnkFrame(Global::window(), &data.ctx);
  glfwSetInputMode(Global::window()->handle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  RegionLog rl("Shutdown", true);

//   guiShutdown(device);
  deinitnk(data);

//   destroyWindow(window);

  // тут может быть какое-нибудь удаление
//   destroyDataArrays(arraysContainer, arrays);

  deinitGLFW();

  cvar::destroy();

  return 0;
}
