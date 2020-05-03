#include "Helper.h"

#include <random>
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

const size_t update_delta = DELTA_TIME_CONSTANT;

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
  cxxopts::Options options("the madness returns", "game");
  options.add_options()
//     ("d,debug", "Enable debugging") // a bool parameter
//     ("i,integer", "Int param", cxxopts::value<int>())
//     ("f,file", "File name", cxxopts::value<std::string>())
    ("h,help", "Help", cxxopts::value<bool>()->default_value("false"), "Print help")
    ("v,version", "Version", cxxopts::value<bool>()->default_value("false"), "Print engine and application version")
  ;
  auto result = options.parse(argc, argv);
  if (result["v"].as<bool>()) {
    std::cout << "The madness returns game" << "\n";
    std::cout << "Engine " << ENGINE_NAME << " version " << ENGINE_VERSION_STR << "\n";
    std::cout << "Application version " << APP_VERSION_STR << "\n";
    return 0;
  }
  
  if (result["h"].as<bool>()) {
    std::cout << "The madness returns game" << "\n";
    std::cout << "-h, --help" << "\t" << "Print this message" << "\n";
    std::cout << "-v, --version" << "\t" << "Print engine and application version" << "\n";
    return 0;
  }
  
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

  {     
    Global g;
    g.setGameDir(get_app_dir());

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
    }
  }

  // у нас тут еще есть создание треад пула
  // причем неплохо было бы создавать его только тогда когда системы его используют
  // например если в системе два треда (в этом случае скорее всего тоже норм)
  // если в настройках определено не использовать треад пул
  // то нам его создавать ни к чему
  // чаще всего пул необходим, не могу честно говоря представить когда он не нужен в современных условиях
  //std::cout << "std::thread::hardware_concurrency() " << std::thread::hardware_concurrency() << "\n";
  dt::thread_pool thread_pool(std::max(std::thread::hardware_concurrency()-1, uint32_t(1)));
  systems::collision_interaction inter(systems::collision_interaction::create_info{&thread_pool});
  utils::delayed_work_system works(utils::delayed_work_system::create_info{&thread_pool});
  utils::random rand(seed);
  input::data input_data;
  Global::get(&works);
  Global::get(&rand);
  Global::get(&input_data);
  
  //initGLFW();
  glfw_init _glfw;

  // еще где то здесь мы обрабатываем входные данные с консоли (забыл как это называется верно лол)

  // теперь создаем контейнер sizeof(ParticleSystem) + sizeof(DecalSystem) +
  system_container systems;
  create_graphics(systems);
  screenshot_container scr(systems.graphics_container->device);

  // как создать буфферы с данными (трансформа, ротация, матрица и прочее)?
  // создать их здесь - будет куча лишних указателей (или не лишних??)
  // удаление я могу упрятать в отдельный контейнер, когда окончательно пойму сколько массивов мне нужно будет
  DataArrays arrays;
  createDataArrays(systems.graphics_container->device, arrays);

  // тут по идее мы должны создать оптимизеры
  create_optimizers(systems, arrays);

  // создадим физику (почти везде нужно собрать сайз для контейнера)
  create_physics(systems, &thread_pool, arrays, update_delta);

  // создадим систему ии
  create_ai(systems, &thread_pool);
  
  // звук
  create_sound_system(systems);
  
  // частицы?
  
  resources_ptr resources(systems.graphics_container->device);

  // создадим лоадер (лоадер бы лучше в контейнере создавать) 
  // лоадеры неплохо было бы создавать перед непосредственной загрузкой, пока так создадим
  yacs::world world;
  Global::get(&world);
  const size_t loaderContainerSize = sizeof(resources::image_loader) + sizeof(resources::sound_loader) + sizeof(resources::entity_loader) + sizeof(resources::map_loader) + sizeof(resources::attributes_loader) + sizeof(resources::effects_loader) + sizeof(resources::abilities_loader) + sizeof(resources::state_loader);
  resources::map_loader* map_loader = nullptr;
  resources::modification_container mods(loaderContainerSize);
  render::image_container images(render::image_container::create_info{systems.graphics_container->device});
  {
    createLoaders(mods, systems.graphics_container, arrays, &images, resources, &map_loader);
    
    // теперь мне нужно загружать сразу все ресурсы, кроме всех карт
  }

//   nuklear_data data;
//   initnk(systems.graphics_container->device(), Global::window(), data);
  interface::context context(systems.graphics_container->device, Global::get<render::window>());
  Global::get(&context);

  std::vector<render::pipeline_stage*> dynPipe;
  {
    const RenderConstructData renderData{
      &systems,
      &arrays,
      &context,
      map_loader,
      &scr
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
  interface::container menu_container(sizeof(interface::main_menu) + sizeof(interface::quit_game) + sizeof(interface::settings) + sizeof(interface::graphics));
  {
    auto menu = menu_container.add_page<interface::main_menu>(interface::main_menu::create_info{&context, nullptr, nullptr, nullptr});
    auto quitGame = menu_container.add_page<interface::quit_game>(interface::quit_game::create_info{&context, menu, &quit});
    auto settings = menu_container.add_page<interface::settings>(interface::settings::create_info{&context, menu});
    auto graphics_settings = menu_container.add_page<interface::graphics>(interface::graphics::create_info{&context, settings});
    // мы должны указать положение и видимо использовать не обычное окно, а композитную область для отрисовки всего этого
    menu->set_pointers(quitGame, settings, nullptr);
    settings->set_pointers(menu, graphics_settings, nullptr, nullptr, nullptr);
    menu_container.set_default_page(menu);
  }

//   KeyContainer keyContainer;
//   createReactions({&keyContainer, Global::window(), &menu_container, entityWithBrain});
//   setUpKeys(&keyContainer);
//   setUpKeys(nullptr);
  setup_keys();
  
  settings.dump(Global::getGameDir() + "settings.json");
  
  // мы може создать дополнительный KeyContainer специально для меню
  
  // обновлять textureLoader и пайплайны нужно будет динамически при изменении состояния textureLoader
  for (size_t i = 0; i < dynPipe.size(); ++i) {
    //ImageLoader* textureLoader = static_cast<ImageLoader*>(loaders[0]);
    dynPipe[i]->recreate_pipelines(resources.image_res);
  }

  Global::get<PhysicsEngine>()->setGravity(simd::vec4(0.0f, -9.8f, 0.0f, 0.0f));
  
  size_t current_time = 0;
  TimeMeter tm(ONE_SECOND);
//   frame_time frame_time1;
  game_state currentState = game_state::loading;
  interface::overlay overlay(interface::overlay::create_info{&context, player, &tm});
  Global::get<render::window>()->show();
  while (!Global::get<render::window>()->close() && !quit) {
    tm.start();
//     frame_time1.next_frame();
    
    const size_t time = tm.time();
//     const size_t time = frame_time1.time;
//     PRINT_VAR("time", time)
    ASSERT(time != 0);
    input::update_time(time);
    {
      Global g;
      g.increase_level_time(time);
    }
    
    poll_events(); // здесь вызываются коллбеки, верно?
    
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
    keys_callback(player, &menu_container, &scr, time);
    nextnkFrame(Global::get<render::window>(), &context.ctx);

    camera::first_person(player, menu_container.is_opened());

    // ии тоже нужно ограничить при открытии менюхи
    if (!menu_container.is_opened()) {
      Global::get<PhysicsEngine>()->update(time);
    }

    // обход видимых объектов 
    //Global::get<PostPhysics>()->update(time);
    post_physics::update(&thread_pool); // player
    Global::get<systems::sound>()->update_listener(player);

    // гуи
    const interface::data::extent screenSize{
      static_cast<float>(Global::get<render::window>()->surface.extent.width), // почему float?
      static_cast<float>(Global::get<render::window>()->surface.extent.height)
    };
    overlay.draw(screenSize); // аттрибуты, иконки, текст, все что не меню (иконка аттрибута?)
    menu_container.proccess(screenSize, time);
    Global::get<render::particles>()->update(Global::get<render::buffers>()->get_view_proj(), Global::get<PhysicsEngine>()->getGravity(), time);
    
    {
      auto interface = player->at<components::player_interface>(game::player::interface);
      interface->draw(time, screenSize);
      auto particles = Global::get<render::particles>();
      static bool first = true;
      if (first) {
        for (size_t i = 0; i < 5; ++i) {
          particles->add(render::particle(simd::vec4(-2.0f+i, 1.0f, 0.0f, 1.0f), simd::vec4(0.0f, 5.0f, 1.0f, 0.0f), render::create_image(0, 1), ONE_SECOND*10));
        }
        
        auto data = reinterpret_cast<render::particles_data*>(particles->data_buffer.ptr());
        if (data->particles_count > 25) first = false;
      }
    }
    
    // дебаг
    // и прочее

    // это и звук вполне могут быть сделаны паралельно
    // и вместе с этом я запущу работу в ворксистеме
    // где то рядом нужно обойти всю коллизию 
    thread_pool.submitbase([] () {
      Global::get<systems::render>()->update(Global::get<render::container>());
//       Global::get<GraphicsContainer>()->update(time); // обновление графики пересекается с телепортацией
    });
    
    thread_pool.submitbase([time] () {
      Global::get<systems::sound>()->update(time); // звуки то поди должны быть разделены на звуки меню и обычные?
    });
    
    // еще отдельно можно запустить обработчика декалей
    // кажется он не пересекается ни со звуком ни с графикой
    if (!menu_container.is_opened()) {
      thread_pool.submitbase([time] () {
        Global::get<systems::decals>()->update(time, update_delta);
      });
    }
    
    // в работах может быть как телепорт, так и вызов звука
    // значит что вызывать параллельно графике и звукам не имеет смысла
    thread_pool.wait();
    
    {
      static bool first = true;
      if (first) {
        auto state = Global::get<game::states_container>()->get(utils::id::get("tmr_decor1_state"));
        const systems::decals::pending_data data{
          basic_vec4(0.0f, 0.0f, 3.5f, 1.0f),
          basic_mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
          ),
          1,
          1,
          state
        };
        Global::get<systems::decals>()->add(data);
        
        const systems::decals::pending_data data1{
          basic_vec4(2.0f, 0.0f, 3.5f, 1.0f),
          basic_mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
          ),
          1,
          1,
          state
        };
        Global::get<systems::decals>()->add(data1);
        first = false;
      }
    }
    
    current_time += time;
    if ((current_time < update_delta || time >= 20000) && !menu_container.is_opened()) {
      // во время исполнения этих работ могут добавится еще несколько, но их необходимо обработать уже в следующий раз
      // если здесь добавляются работы которые удаляют какой то объект, то необходимо правильно обновить tree_ai и func_ai
      // это значит что tree_ai и func_ai должны обновляться каждый кадр или detach_works должно работать иначе
      const size_t count = works.works_count();
      works.detach_works(count);
    }
    
    thread_pool.wait();
    
    if (current_time >= update_delta) {
      current_time -= update_delta;
      if (!menu_container.is_opened()) {
        utils::update<components::states>(&thread_pool, update_delta);     // может вызваться core::remove
        utils::update<components::attributes>(&thread_pool, update_delta); // может вызваться core::remove
        utils::update<components::effects>(&thread_pool, update_delta);
        utils::update<components::tree_ai>(&thread_pool, update_delta); 
        utils::update<components::func_ai>(&thread_pool, update_delta);
        // функции ниже добавляют работу в works
        // функции коллизии
        inter.update(update_delta); // не должны ли и они быть синхронизированы по времени? судя по моей физике - да
        // интеракции
        utils::update<core::slashing_interaction>(&thread_pool, update_delta);
        utils::update<core::stabbing_interaction>(&thread_pool, update_delta);
      }
    }
    
//     {
//       auto particles = Global::get<render::particles>();
//       auto data = reinterpret_cast<render::particles_data*>(particles->data_buffer.ptr());
//       std::cout << "particles count " << data->particles_count << "\n";
//       std::cout << "old particles count " << data->old_count << "\n";
//       for (size_t i = 0; i < data->particles_count; ++i) {
//         PRINT_VEC4("pos", particles->array[i].pos.get_simd())
//         PRINT_VEC4("vel", particles->array[i].vel.get_simd())
//         PRINT_VAR("life_time",particles->array[i].life_time)
//         PRINT_VAR("current_time",particles->array[i].current_time)
//         std::cout << "\n";
//       }
//       
//       static size_t counter = 0;
//       ++counter;
// //       if (counter == 5) throw std::runtime_error("no more");
//     }

    // здесь же у нас должна быть настройка: тип какую частоту обновления экрана использовать?
    const size_t syncTime = Global::get<render::window>()->flags.vsync() ? Global::get<render::window>()->refresh_rate_mcs() : 0; //16667
    sync(tm, syncTime);
  }
  
  appTime.updateTime();
  return_cursor();

  return 0;
}
