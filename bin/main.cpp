#include "Helper.h"

#include <random>
#include "whereami.h"
#include <cxxopts.hpp>

// из крупняка осталось сделать загрузку/сохранение, загрузку карт, двери, ???

// как делается загрузка/сохранение?
// нам необходимо полностью востановить стейт мира: все энтити загружаются правильного типа, все позиции сохранены и проч
// это значит: нужно сохранить все загруженные моды, состояние рандома (?), и каждого энтити на карте
// состояние энтити у меня определяется состоянием всех его компонентов
// состояние компонента определяется: указатель на энтити, внутренний стейт, время
// указатель на энтити - можно сохранить индекс, внутренний стейт у многих компонентов опредялется типом энтити, время - число
// я должен отталкиваться от компонентов, конкретные индексы должны быть неважны, у каждого энтити должно быть определено две функции
// сериалайз + десериалайз (не обязательно делать их виртуальными), сохранение это обход каждого компонента энтити, не у всех компонентов меняется стейт
// (например у физики по идее не меняется (у физики меняется скорость), у графики, их нужно пересобирать заново: всю информацию можно получить из данных модов)
// нужен еще скриншот

// загрузка карты: точки окружения + набор энтити ид + позиции/направления/тэги для энтити
// + возможность обработать вход из блендера
// + настройка физики под карту

// двери: двигающиеся сложные объекты, должны каким-либо образом перекрывать доступ между вершинами
// два режима меня интересует: статика поднимается опускается и двигающиеся под действием гравитации объекты
// еще это могут быть как и двери так и платформы 
// (как завершинить платформы? в этом плане сложно, нужно при изменении состояния платформы изменять видимо состояние ребра)
// видимо у таких объектов будет только один стейт

const size_t updateDelta = DELTA_TIME_CONSTANT;

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
  demo_playing, // причем неплохо было бы сделать проигрывание демки в главном меню
  loading,
  game
};

int main(int argc, char** argv) {
  TimeLogDestructor appTime("Shutdown");
  
  // парсить входные жанные будем так
  //cxxopts::Options options("the madness returns", "game");
//   options.add_options()
//     ("d,debug", "Enable debugging") // a bool parameter
//     ("i,integer", "Int param", cxxopts::value<int>())
//     ("f,file", "File name", cxxopts::value<std::string>())
//     ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
//   ;
  //auto result = options.parse(argc, argv);
  //result["opt"].as<type>()
  
  uint32_t seed = 0;
  {
    std::random_device rd;
    seed = rd();
  }
  
  //std::locale::global(std::locale("ru_RU.UTF-8"));
  std::locale::global(std::locale("en_US.UTF-8"));

  // тут мы создаем дополнительные вещи, такие как консоль, команды, переменные
  Global::init();
  Global::commands()->addCommand("set", utils::cvar::set_f);
  Global::commands()->addCommand("get", utils::cvar::get_f);

  //cvar debugDraw("debugDraw", float(0), VAR_ARCHIVE);
  utils::cvar::create(utils::id::get("debug_draw"), 0.0);

  // нам нужно еще создать settings
  { 
    int dirname;
    uint32_t length = wai_getExecutablePath(NULL, 0, NULL);
    std::vector<char> str(length+1);
    wai_getExecutablePath(str.data(), length, &dirname);
    str[length] = '\0';
    const std::string dir(str.data(), dirname+1);
    Global g;
    g.setGameDir(dir);

    //std::cout << Global::getGameDir() << "\n";
//     std::cout << dir << "\n";
  }

  utils::settings settings;
  {
    Global::get<utils::settings>(&settings);
    ASSERT(Global::get<utils::const_settings>() == nullptr);
    ASSERT(Global::get<utils::settings>());
    
    cppfs::FileHandle fh = cppfs::fs::open(Global::getGameDir() + "settings.json");

    if (fh.exists()) {
      // грузим настройки
      settings.load(fh.path());
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
  systems::collision_interaction inter(systems::collision_interaction::create_info{&threadPool});
  utils::delayed_work_system works(utils::delayed_work_system::create_info{&threadPool});
  utils::random rand(seed);
  input::data input_data;
  Global::get(&works);
  Global::get(&rand);
  Global::get(&input_data);
  
  initGLFW();

  // еще где то здесь мы обрабатываем входные данные с консоли (забыл как это называется верно лол)

  // теперь создаем контейнер sizeof(ParticleSystem) + sizeof(DecalSystem) +
  system_container systems;
  create_graphics(systems);

  // как создать буфферы с данными (трансформа, ротация, матрица и прочее)?
  // создать их здесь - будет куча лишних указателей (или не лишних??)
  // удаление я могу упрятать в отдельный контейнер, когда окончательно пойму сколько массивов мне нужно будет
  DataArrays arrays;
  createDataArrays(systems.graphics_container->device(), arrays);

  // тут по идее мы должны создать оптимизеры
  create_optimizers(systems, arrays);

  // создадим физику (почти везде нужно собрать сайз для контейнера)
  create_physics(systems, &threadPool, arrays, updateDelta);

  // создадим систему ии
  create_ai(systems, &threadPool);
  
  // звук
  create_sound_system(systems);
  
  // частицы?
  
  resources_ptr resources(systems.graphics_container->device());

  // создадим лоадер (лоадер бы лучше в контейнере создавать) 
  // лоадеры неплохо было бы создавать перед непосредственной загрузкой, пока так создадим
  yacs::world world;
  Global::get(&world);
  const size_t loaderContainerSize = sizeof(resources::image_loader) + sizeof(resources::sound_loader) + sizeof(resources::entity_loader) + sizeof(resources::map_loader) + sizeof(resources::attributes_loader) + sizeof(resources::effects_loader) + sizeof(resources::abilities_loader) + sizeof(resources::state_loader);
  resources::map_loader* map_loader = nullptr;
  resources::modification_container mods(loaderContainerSize);
  render::image_container images(render::image_container::create_info{systems.graphics_container->device()});
  {
    createLoaders(mods, systems.graphics_container, arrays, &images, resources, &map_loader);
    
    // теперь мне нужно загружать сразу все ресурсы, кроме всех карт
  }

  nuklear_data data;
  initnk(systems.graphics_container->device(), Global::window(), data);

  std::vector<DynamicPipelineStage*> dynPipe;
  {
    const RenderConstructData renderData{
      &systems,
      &arrays,
      &data,
      Global::get<MonsterDebugOptimizer>(),
      Global::get<GeometryDebugOptimizer>()
    };
    createRenderStages(renderData, dynPipe);
    // переделать рендер, частицы вызовут проблемы у меня сейчас
  }

  // загрузим стейт по умолчанию
  yacs::entity* player = nullptr;
//   CameraComponent* camera = nullptr;
//   UserInputComponent* input = nullptr;
//   TransformComponent* playerTransform = nullptr;
//   yacs::entity* entityWithBrain = nullptr;
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
    mods.end();
    map_loader->load_obj(Global::getGameDir() + "models/box4.obj");
    map_loader->end();

//     std::cout << "clearing" << "\n";

    player = map_loader->player();
    //entityWithBrain = mapLoader->getEntityBrain();

    // понадобится ли нам указатели на текстур лоадер и прочее, кроме уровней?
    // понадобится для того чтобы обновлять, например, пайплайны
    // возможно понадобится что то еще
    
    //throw std::runtime_error("no more");
  }

  bool quit = false;
  interface::container menu_container(sizeof(interface::main_menu) + sizeof(interface::quit_game) + sizeof(interface::settings));
  {
    auto menu = menu_container.add_page<interface::main_menu>(interface::main_menu::create_info{"Main menu", &data, nullptr, nullptr, nullptr, {300, 500}});
    auto quitGame = menu_container.add_page<interface::quit_game>(interface::quit_game::create_info{"Quit game", &data, menu, &quit, {300, 500}});
    // мы должны указать положение и видимо использовать не обычное окно, а композитную область для отрисовки всего этого
    menu->set_pointers(quitGame, nullptr, nullptr);
    menu_container.set_default_page(menu);
  }

//   KeyContainer keyContainer;
//   createReactions({&keyContainer, Global::window(), &menu_container, entityWithBrain});
//   setUpKeys(&keyContainer);
  setUpKeys(nullptr);
  
  settings.dump(Global::getGameDir() + "settings.json");
  
  // мы може создать дополнительный KeyContainer специально для меню
  
  // обновлять textureLoader и пайплайны нужно будет динамически при изменении состояния textureLoader
  for (size_t i = 0; i < dynPipe.size(); ++i) {
    //ImageLoader* textureLoader = static_cast<ImageLoader*>(loaders[0]);
    dynPipe[i]->recreatePipelines(resources.image_res);
  }

  Global::get<PhysicsEngine>()->setGravity(simd::vec4(0.0f, -9.8f, 0.0f, 0.0f));
  
  size_t current_time = 0;
  TimeMeter tm(ONE_SECOND);
  Global::window()->show();
  game_state currentState = game_state::loading;
  interface::overlay overlay(interface::overlay::create_info{&data, player, &tm});
  while (!Global::window()->shouldClose() && !quit) {
    tm.start();
    
    const size_t time = tm.time();
    ASSERT(time != 0);
    input::update_time(time);
    {
      Global g;
      g.increase_level_time(time);
    }
    
    glfwPollEvents(); // здесь вызываются коллбеки, верно?
    
    switch (currentState) {
      case game_state::loading: {
        // здесь должна быть работа ModificationContainer + небольшой VulkanRender (какое то изображение + статус загрузки)
        // можно ли использовать один VulkanRender или нужно сделать отдельный? 
        // в принципе все что нужно сделать это нарисовать текстуру с координатами, координаты статичные на весь экран
        // + нужно скорректировать матрицу. возможно нужно просто сделать еще один рендер стейдж
        // перед загрузкой неплохо было бы еще вычищать полностью textureLoader и загружать туда только картинку загрузки
        // вычищать нужно для того чтобы там не остались какие то проблемные участки с предыдущего уровня или из меню
        
        // создаем лоадеры
        // парсим моды (нам при смене уровня по идее нужно парсить только инфу об уровне)
        // грузим уровень в отдельном потоке
        // во время этого рисуем интерфейс 
        // нужно ли делать думовское сползание экрана?
        // требуется запомнить картинку перед началом загрузки
        
        break;
      }
      
      case game_state::demo_playing: {
        // по идее меню на фоне демки
        
        break;
      }
      
      case game_state::game: {
        // обычный стейт
        
        break;
      }
    }

    // где то здесь нам нужно сделать проверки на конец уровня, начало другого (или в синке это делать)
    // еще же статистику выводить (в конце уровня я имею ввиду)
    // нужно определить функцию некст левел, которая просто будет выставлять флажок следующего уровня
    // а здесь мы будем проверять и соответственно начинать загружаться
    // при загрузке уровня меняем стейт
    
    // инпут игрока нужно чуть чуть переделать 
    // (необходимо сделать какой то буфер нажатия клавиш и возможность использовать его в разных местах)
    mouse_input(player, time);
    keys_callback(player, &menu_container);
    nextnkFrame(Global::window(), &data.ctx);

    camera::first_person(player);

    // ии тоже нужно ограничить при открытии менюхи
    if (!menu_container.is_opened()) {
      Global::get<PhysicsEngine>()->update(time);
    }

    // обход видимых объектов 
    //Global::get<PostPhysics>()->update(time);
    post_physics::update(&threadPool, player);
    Global::get<systems::sound>()->update_listener(player);

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
    threadPool.submitbase([time] () {
      Global::get<GraphicsContainer>()->update(time); // обновление графики пересекается с телепортацией
    });
    
    threadPool.submitbase([time] () {
      Global::get<systems::sound>()->update(time); // звуки то поди должны быть разделены на звуки меню и обычные?
    });
    
    current_time += time;
    if (current_time < updateDelta || time >= 20000) {
      // во время исполнения этих работ могут добавится еще несколько, но их необходимо обработать уже в следующий раз
      // если здесь добавляются работы которые удаляют какой то объект, то необходимо правильно обновить tree_ai и func_ai
      // это значит что tree_ai и func_ai должны обновляться каждый кадр или detach_works должно работать иначе
      const size_t count = works.works_count();
      works.detach_works(count);
    }
    
    threadPool.wait();
    
    if (current_time >= updateDelta) {
      current_time -= updateDelta;
      if (!menu_container.is_opened()) {
        utils::update<components::states>(&threadPool, updateDelta);
        utils::update<components::attributes>(&threadPool, updateDelta); // тут будет вызываться core::remove
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

//   cvar::destroy();
  
  //delete mapLoader;

  return 0;
}
