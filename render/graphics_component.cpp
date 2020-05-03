#include "graphics_component.h"

#include "EntityComponentSystem.h"
#include "Globals.h"
// #include "GPUOptimizers.h"
#include "stages.h"
#include "global_components_indicies.h"
#include "TransformComponent.h"
#include "states_component.h"
#include "shared_mathematical_constant.h"
#include "Physics.h"
// #include "interface_context.h"

#include "game_funcs.h"
#include "core_funcs.h"

namespace devils_engine {
  struct lower_panel_sizes {
    float width;
    float height;
    float panel_offset_x;
    float panel_offset_y;
    
    struct nk_rect face_panel;
    struct nk_rect face_image;
    
    struct nk_rect skills_panel;
    
    struct nk_rect attrib_panel;
    struct nk_rect armor_panel;
    
    struct nk_rect ammo_panel;
    struct nk_rect inventory_panel;
    
    lower_panel_sizes(const float &panel_width, const float &panel_height, const float &offset_x, const float &offset_y);
  };
  
  struct attrib_panel_sizes {
    struct nk_rect attrib_value;
    struct nk_rect attrib_value_shadow;
    struct nk_rect attrib_name;
    struct nk_rect attrib_name_shadow;
    
    attrib_panel_sizes(const struct nk_rect &attrib_panel, const float &border);
  };
  
  namespace components {
    void sprite_graphics::draw() {
      if (core::deleted_state(ent)) return;
      
//       auto opt = Global::get<MonsterGPUOptimizer>();
      auto opt = Global::get<render::monster_optimizer>();
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      auto states = ent->at<components::states>(game::entity::states);
      
      //auto loader = Global::get<states_loader>();
      
      // нужно добавить как то индексы текстуры
      // вообще я думал хранить изображение в каком нибудь массиве как это было в анимациях
      // и в этом случае отправлять сразу индексы изображения
      // откуда мы берем стейты? скорее всего из стейт лоадера
      // оттуда же мы должны брать текущую текстурку
      
      if (states->current == nullptr) return;
      
      const uint32_t frame_size = states->current->frame.images_count;
      int index = 0;
      if (frame_size > 1) {
        // вычислим сторону 
        //const simd::vec4 playerPos = Global::getPlayerPos();
        const simd::vec4 playerPos = core::player_pos();

        simd::vec4 dir = playerPos - trans->pos();

        const simd::vec4 dirOnGround = projectVectorOnPlane(-PhysicsEngine::getGravityNorm(), trans->pos(), dir);

        dir = simd::normalize(dirOnGround);

        float angle2 = glm::acos(simd::dot(trans->rot(), dir));
        // проверим сторону
        const bool side = sideOf(trans->pos(), trans->pos()+trans->rot(), playerPos, -PhysicsEngine::getGravityNorm()) > 0.0f;
        angle2 = side ? -angle2 : angle2;

  #define PI_FRAME_SIZE (PI_2 / frame_size)
  #define PI_HALF_FRAME_SIZE (PI_FRAME_SIZE / 2)
        // поправка на 22.5 градусов (так как 0 принадлежит [-22.5, 22.5))
        angle2 -= PI_HALF_FRAME_SIZE;

        angle2 = angle2 < 0.0f ? angle2 + PI_2 : angle2;
        angle2 = angle2 > PI_2 ? angle2 - PI_2 : angle2;
        index = glm::floor(angle2 / PI_FRAME_SIZE);

        // я не понимаю почему (5 при 8 сторонах)
        index = (index + (frame_size/2 + 1)) % frame_size;
      }
      
      const uint32_t final_image_index = states->current->frame.texture_offset + index;
      // получаем изображение и засовываем его в буфер
      // с другой стороны можем ли мы использовать буфер из которого мы берем собственно изображение
      // по идее да
      
      float arr[4];
      trans->scale().storeu(arr);
      
//       const MonsterGPUOptimizer::GraphicsIndices idx{
//         trans->index(),
//         UINT32_MAX,
//         UINT32_MAX,
//         final_image_index,
//         arr[1]
//       };
      const render::monster_optimizer::object_indices idx{
        trans->index(),
        UINT32_MAX,
        UINT32_MAX,
        final_image_index,
        arr[1]
      };
      opt->add(idx);
    }
    
    void sprite_graphics::debug_draw() {
      
    }
    
    void indexed_graphics::draw() {
      if (core::deleted_state(ent)) return;
      
      //auto opt = Global::get<GeometryGPUOptimizer>();
      auto opt = Global::get<render::geometry_optimizer>();
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      auto states = ent->at<components::states>(game::entity::states);
      
      if (states->current == nullptr) return;
      
      //std::cout << "sprite graphics state " << states->current->id.name() << "\n";
      
      // может ли тут быть ситуация
      // для стен врядли, хотя можно сделать интересные эффекты
      ASSERT(states->current->frame.images_count < 2);
      
      const uint32_t texture_index = states->current->frame.texture_offset;
      
//       const GeometryGPUOptimizer::GraphicsIndices idx{
//         trans.valid() ? trans->index() : UINT32_MAX,
//         UINT32_MAX,
//         UINT32_MAX,
//         texture_index,
//         offset,
//         count,
//         index,
//         0
//       };
      const render::geometry_optimizer::object_indices idx{
        trans.valid() ? trans->index() : UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        texture_index,
        offset,
        count,
        index,
        0
      };
      opt->add(idx);
    }
    
    void indexed_graphics::debug_draw() {
      // нужно отрисовать то же самое только с цветом
    }
    
    void point_light::draw() {
      if (core::deleted_state(ent)) return;
      if (glm::abs(radius * brightness) < EPSILON) return;
      
      // модификатор к позиции мы должны задавать извне
      // света не может быть у стен я так понимаю
      // у стен только параметр освещенности который мы задаем в карте
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      
      const render::lights_optimizer::light_data data{
        {
          glm::vec4(0.0f, 0.0f, 0.0f, brightness),     // как позицию брать из частицы? нам придется подключать еще один буфер в шейдер
          glm::vec4(color.x, color.y, color.z, radius)
        },
        trans.valid() ? trans->index() : UINT32_MAX
      };
      Global::get<render::lights_optimizer>()->add(data);
    }
    
    void player_sprite::draw(const size_t &time) {
      // нужно интерполировать смещение
      // интерполяция будет происходить исходя из константы
      // то есть константа - это 1.0f
    }
    
    player_interface::player_interface(yacs::entity* ent) : ent(ent), face_state(nullptr), current_time_info(0), current_time_game(0) {}
    void player_interface::draw(const size_t &time, const interface::data::extent &screen_size) {
      current_time_info += time;
      current_time_game += time;
      auto ctx = &Global::get<interface::context>()->ctx;
      auto interface_ctx = Global::get<interface::context>();
      
      // рисуем окно на всю величину фрейм буфера
      // слева вверху у нас сообщения
      // по центру у нас другие сообщения (сам текст должен быть по центру)
      // снизу, справа снизу, слева? - собственно интерфейс с игровой инфой
      // он может различаться от перса к персу
      // но вот в чем свойство - в таком интерфейсе выводится только информация
      // нужно дать просто несколько возможностей нарисовать инфу в интерфейсе
      // этого будет достаточно
      
      // относительные размеры нижней панели? некая константа
      {
        interface::style_background_color s1(ctx, nk_rgba(0, 0, 0, 0));
//       nk_style* s = &ctx->style;
//       nk_color* oldColor = &s->window.background;
//       nk_style_item* oldStyleItem = &s->window.fixed_background;
//       nk_style_push_color(ctx, oldColor, nk_rgba(0, 0, 0, 0));
//       nk_style_push_style_item(ctx, oldStyleItem, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
      
        if (nk_begin(ctx, "info_text", nk_rect(400, 10, screen_size.width-20, 300), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
          nk_style_set_font(ctx, &interface_ctx->fonts[fonts::interface]->handle);
          nk_layout_row_static(ctx, 0.0f, screen_size.width-20, 1);
          //nk_layout_row_dynamic(ctx, 0.0f, 2);
  //         const float ratio[] = {0.01f, 1.0f};
  //         nk_layout_row(ctx, NK_DYNAMIC, 13, 2, ratio);
          for (size_t i = 0; i < strings.size(); ++i) {
  //           nk_image(ctx, image_to_nk_image(Image{0, 1}));
            // таким образом можно текстурировать текст
            //nk_label_colored(ctx, strings[i].c_str(), NK_TEXT_ALIGN_LEFT, nk_color{0,1,0,1});
            nk_label_colored(ctx, strings[i].c_str(), NK_TEXT_ALIGN_LEFT, nk_color{255,0,0,255});
            //nk_button_image(ctx, image_to_nk_image(Image{0, 1}));
            //nk_button_label(ctx, "kaqksfka");
          }
        }
        nk_end(ctx);
        
        if (current_time_info >= info_message_time) {
          strings.pop_front();
          current_time_info -= info_message_time;
        }
        
        if (!game_messages.empty()) {
          const float size_w = screen_size.width * 0.5f;
          const float size_h = screen_size.height * 0.5f;
          const auto rect = nk_rect(screen_size.width*0.5f-size_w*0.5f, screen_size.height*0.5f-size_h*0.5f, size_w, size_h);
          if (nk_begin(ctx, "message_text", rect, NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
            nk_layout_row_static(ctx, 13.0f, size_w, 1);
            nk_label_colored(ctx, game_messages.front().c_str(), NK_TEXT_ALIGN_CENTERED, nk_color{255,0,0,255});
          }
          nk_end(ctx);
          
          if (current_time_game >= game_message_time) {
            game_messages.pop();
            current_time_game -= game_message_time;
          }
        }
      
//       nk_style_pop_color(ctx);
//       nk_style_pop_style_item(ctx);
      }
      
      //const char str[] = "HEALTH";
      
      {
        interface::style_borders style(ctx, 4, 4);
        //auto attr = ent->at<components::effects>(game::player::effects);
        const float space_offset_x = 3;
        const float space_offset_y = -2;
//         const float borders = space_offset_x*2 + space_offset_x*2 + space_offset_x*2 + space_offset_x*2; // space_offset_x
        const float width = screen_size.width;
        const float height = screen_size.height/10.0f;
        const auto rect = nk_rect(-2, screen_size.height-height, width+4, height+4);
        if (nk_begin(ctx, "lower_panel", rect, NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
          nk_style_set_font(ctx, &interface_ctx->fonts[fonts::interface]->handle);
          nk_layout_space_begin(ctx, NK_STATIC, height, INT_MAX); 
          const auto bounds = nk_layout_space_bounds(ctx);
          const float space_height = bounds.h-10;
          
          const float group_border = 4;
          interface::style_borders style(ctx, 4, group_border);
//           std::cout << "space bounds x " << bounds.x << " y " << bounds.y << " w " << bounds.w << " h " << bounds.h << "\n";
//           std::cout << "window rect  x " << rect.x << " y " << rect.y << " w " << rect.w << " h " << rect.h << "\n";
  //         nk_layout_space_push(ctx, rect);
          
  //         nk_style* s = &ctx->style;
  //         nk_color* oldColor = &s->window.background;
  //         nk_style_item* oldStyleItem = &s->window.fixed_background;
  //         nk_color* old_border_color = &s->window.group_border_color;
  // //         nk_style_push_color(ctx, oldColor, nk_rgba(120, 120, 120, 255));
  // //         nk_style_push_style_item(ctx, oldStyleItem, nk_style_item_color(nk_rgba(120, 120, 120, 255)));
  //         nk_style_push_color(ctx, old_border_color, nk_rgba(120, 120, 120, 255));
  //         if (nk_group_begin(ctx, "background", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NO_INPUT)) {
  //           // ???
  //           nk_layout_row_static(ctx, height, width, 1);
  //           nk_image(ctx, nk_subimage_handle(nk_handle_image(Image{0,0}), width, height, nk_rect(0, 0, 1, 1)));
  //           nk_group_end(ctx);
  //         }
  //         
  // //         nk_style_pop_color(ctx);
  // //         nk_style_pop_style_item(ctx);
  //         nk_style_pop_color(ctx);
          
          // скорее всего нужно будет разделить нижнюю часть на 5 отделов, 
          // каждый будет управляться nk_group_begin 
          // 1) аттрибуты (хп, броня) (было бы неплохо в одном nk_layout использовать несколько фонтов), 
          // 2) скиллы (или оружее, тут можно оформить выбор конкретного оружия), 
          // 3) лицо, 
          // 4) список патронов с пометкой какие патроны сейчас используются (выделения достаточно),
          // 5) инвентарь (в широкоформатные экраны поди поместится 5 айтемов)
          // про идее нужно сделать несколько слоев: задний слой группы, слой панельки, непосредственная информация
          // так получится богаче нижняя панелька
          // тень текста можно сделать написав текст два раза
          // текст рисуется по цвету текстуры, делается это несложно, вместо цвета нужно указать номер текстурки
          // тень текста - это тот же текст, чуть чуть смещенный вправо вниз
          // как это сделать средствами наклира?
          // один nk_layout_space_begin, два nk_layout_space_push, один от другого отличается на пару пикселей
          // там мы указываем nk_rect в координатах nk_layout_space_begin (?)
          // существуют функции переводящие координаты из/в nk_layout_space координаты
          // в демке используются nk_layout_space и nk_group попеременно
          // спейс действительно позволяет наложить одну группу на другую
          
//           float offset = 0;
//           float offset_y = space_offset_y;
          const float elem_width = bounds.w/5.0f-space_offset_x-2; // -borders
          lower_panel_sizes lower_sizes(bounds.w, space_height, space_offset_x, space_offset_y);
          
          {
            const auto attr_rect = lower_sizes.attrib_panel;
            nk_layout_space_push(ctx, attr_rect);
            if (nk_group_begin(ctx, "attribs", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
              interface::style_borders style(ctx, 0, 0);
              interface::style_background_color s1(ctx, nk_rgba(0, 0, 0, 0));
              nk_layout_space_begin(ctx, NK_STATIC, space_height, 10);
              const auto attribs_bounds = nk_layout_space_bounds(ctx);
              attrib_panel_sizes sizes(attribs_bounds, group_border);
              
              //std::cout << "space bounds x " << attribs_bounds.x << " y " << attribs_bounds.y << " w " << attribs_bounds.w << " h " << attribs_bounds.h << "\n";
              nk_style_set_font(ctx, &interface_ctx->fonts[fonts::interface]->handle);
              
              // тень
              nk_layout_space_push(ctx, sizes.attrib_value_shadow);
              if (nk_group_begin(ctx, "attribs1_shadow", NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
                nk_layout_row_dynamic(ctx, 25.0f, 1); // width
//                 nk_label_colored(ctx, "HEALTH", NK_TEXT_ALIGN_LEFT, nk_color{255,0,0,255});
                nk_label_colored(ctx, "100/100", NK_TEXT_ALIGN_CENTERED, nk_color{0,0,0,255});
//                 nk_label(ctx, "ARMOR", NK_TEXT_ALIGN_LEFT);
//                 nk_label(ctx, "100/100", NK_TEXT_ALIGN_RIGHT);
                nk_group_end(ctx);
              }
              
              // данные аттрибута
              nk_layout_space_push(ctx, sizes.attrib_value);
              if (nk_group_begin(ctx, "attribs1", NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
                nk_layout_row_dynamic(ctx, 25.0f, 1); // width
//                 nk_label_colored(ctx, "HEALTH", NK_TEXT_ALIGN_LEFT, nk_color{255,0,0,255});
                nk_label_colored(ctx, "100/100", NK_TEXT_ALIGN_CENTERED, nk_color{255,0,0,255});
//                 nk_label(ctx, "ARMOR", NK_TEXT_ALIGN_LEFT);
//                 nk_label(ctx, "100/100", NK_TEXT_ALIGN_RIGHT);
                nk_group_end(ctx);
              }
              
              nk_style_set_font(ctx, &interface_ctx->fonts[fonts::technical]->handle);
              nk_layout_space_push(ctx, sizes.attrib_name_shadow);
              if (nk_group_begin(ctx, "name1_shadow", NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
                nk_layout_row_dynamic(ctx, 20.0f, 1);
                //nk_label(ctx, "attributes", NK_TEXT_ALIGN_CENTERED);
                nk_label_colored(ctx, "HEALTH", NK_TEXT_ALIGN_CENTERED, nk_color{0,0,0,255});
                nk_group_end(ctx);
              }
              
              nk_layout_space_push(ctx, sizes.attrib_name);
              if (nk_group_begin(ctx, "name1", NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
                nk_layout_row_dynamic(ctx, 20.0f, 1);
                //nk_label(ctx, "attributes", NK_TEXT_ALIGN_CENTERED);
                nk_label(ctx, "HEALTH", NK_TEXT_ALIGN_CENTERED);
                nk_group_end(ctx);
              }
              
              nk_layout_space_end(ctx);
              nk_group_end(ctx);
            }
          }
          
          {
            const auto attr_rect = lower_sizes.armor_panel;
            nk_layout_space_push(ctx, attr_rect);
            if (nk_group_begin(ctx, "attribs", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
              interface::style_borders style(ctx, 0, 0);
              interface::style_background_color s1(ctx, nk_rgba(0, 0, 0, 0));
              nk_layout_space_begin(ctx, NK_STATIC, space_height, 10);
              const auto attribs_bounds = nk_layout_space_bounds(ctx);
              attrib_panel_sizes sizes(attribs_bounds, group_border);
              
              //std::cout << "space bounds x " << attribs_bounds.x << " y " << attribs_bounds.y << " w " << attribs_bounds.w << " h " << attribs_bounds.h << "\n";
              nk_style_set_font(ctx, &interface_ctx->fonts[fonts::interface]->handle);
              
              // тень
              nk_layout_space_push(ctx, sizes.attrib_value_shadow);
              if (nk_group_begin(ctx, "attribs2_shadow", NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
                nk_layout_row_dynamic(ctx, 25.0f, 1); // width
//                 nk_label_colored(ctx, "HEALTH", NK_TEXT_ALIGN_LEFT, nk_color{255,0,0,255});
                nk_label_colored(ctx, "100/100", NK_TEXT_ALIGN_CENTERED, nk_color{0,0,0,255});
//                 nk_label(ctx, "ARMOR", NK_TEXT_ALIGN_LEFT);
//                 nk_label(ctx, "100/100", NK_TEXT_ALIGN_RIGHT);
                nk_group_end(ctx);
              }
              
              // данные аттрибута
              nk_layout_space_push(ctx, sizes.attrib_value);
              if (nk_group_begin(ctx, "attribs2", NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
                nk_layout_row_dynamic(ctx, 25.0f, 1); // width
//                 nk_label_colored(ctx, "HEALTH", NK_TEXT_ALIGN_LEFT, nk_color{255,0,0,255});
                nk_label_colored(ctx, "100/100", NK_TEXT_ALIGN_CENTERED, nk_color{255,0,0,255});
//                 nk_label(ctx, "ARMOR", NK_TEXT_ALIGN_LEFT);
//                 nk_label(ctx, "100/100", NK_TEXT_ALIGN_RIGHT);
                nk_group_end(ctx);
              }
              
              nk_style_set_font(ctx, &interface_ctx->fonts[fonts::technical]->handle);
              nk_layout_space_push(ctx, sizes.attrib_name_shadow);
              if (nk_group_begin(ctx, "name2_shadow", NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
                nk_layout_row_dynamic(ctx, 20.0f, 1);
                //nk_label(ctx, "attributes", NK_TEXT_ALIGN_CENTERED);
                nk_label_colored(ctx, "ARMOR", NK_TEXT_ALIGN_CENTERED, nk_color{0,0,0,255});
                nk_group_end(ctx);
              }
              
              nk_layout_space_push(ctx, sizes.attrib_name);
              if (nk_group_begin(ctx, "name2", NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
                nk_layout_row_dynamic(ctx, 20.0f, 1);
                //nk_label(ctx, "attributes", NK_TEXT_ALIGN_CENTERED);
                nk_label(ctx, "ARMOR", NK_TEXT_ALIGN_CENTERED);
                nk_group_end(ctx);
              }
              
              nk_layout_space_end(ctx);
              nk_group_end(ctx);
            }
          }
          
          nk_style_set_font(ctx, &interface_ctx->fonts[fonts::interface]->handle);
          const auto skills_rect = lower_sizes.skills_panel;
          nk_layout_space_push(ctx, skills_rect);
          if (nk_group_begin(ctx, "skills", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
            // тут на самом деле 6 картинок должно быть наверное (если скиллы)
            nk_layout_row_static(ctx, space_height*0.75f/3.0f, elem_width, 1);
            nk_label(ctx, "100/100", NK_TEXT_ALIGN_CENTERED);
            nk_label(ctx, "100/100", NK_TEXT_ALIGN_CENTERED);
            nk_layout_row_static(ctx, space_height*0.25f, elem_width, 1);
            nk_label(ctx, "skills", NK_TEXT_ALIGN_CENTERED);
            nk_group_end(ctx);
          }
          
          {
            interface::style_background_color s1(ctx, nk_rgba(0, 0, 0, 255));
            //nuklear_style_borders style(ctx, 0, 0);
            const auto face_rect1 = lower_sizes.face_panel;
            nk_layout_space_push(ctx, face_rect1);
            if (nk_group_begin(ctx, "face", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
              nk_group_end(ctx);
            }
            const auto face_rect2 = lower_sizes.face_image;
            nk_layout_space_push(ctx, face_rect2);
            nk_image(ctx, image_to_nk_image(render::create_image(0, 1, 0, false, false)));
          }
          
          const auto ammo_rect = lower_sizes.ammo_panel;
          nk_layout_space_push(ctx, ammo_rect);
          if (nk_group_begin(ctx, "ammo", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_static(ctx, space_height*0.75f/3.0f, elem_width, 1);
            nk_label(ctx, "blue mana", NK_TEXT_ALIGN_CENTERED);
            nk_label(ctx, "red mana", NK_TEXT_ALIGN_CENTERED);
            nk_label(ctx, "green mana", NK_TEXT_ALIGN_CENTERED);
            nk_layout_row_static(ctx, space_height*0.25f, elem_width, 1);
            nk_label(ctx, "ammo", NK_TEXT_ALIGN_CENTERED);
            nk_group_end(ctx);
          }
          
          const auto inventory_rect = lower_sizes.inventory_panel;
          nk_layout_space_push(ctx, inventory_rect);
          if (nk_group_begin(ctx, "inventory", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_static(ctx, space_height*0.75f/2.0f, elem_width, 1);
            nk_label(ctx, "inventory", NK_TEXT_ALIGN_CENTERED);
            nk_layout_row_static(ctx, space_height*0.25f, elem_width, 1);
            nk_label(ctx, "skills", NK_TEXT_ALIGN_CENTERED);
            nk_group_end(ctx);
          }
          
          nk_layout_space_end(ctx);
        }
        nk_end(ctx);
      }
      
      
//       nk_style_pop_color(ctx);
//       nk_style_pop_style_item(ctx);
//       
//       const struct nk_input *in = &ctx->input;
//       if (nk_begin(ctx, "effects_panel", nk_rect(10, 10, 300, 300), 0)) {
//         // обходим все эффекты, рисуем только изображения
//         nk_layout_row_dynamic(ctx, 0.0f, 5);
//         nk_image(ctx, nk_image_handle(nk_handle_image(Image{1, 1})));
//         //nk_style_set_font(ctx, &f->font->handle);
//         const auto bounds = nk_widget_bounds(ctx);
//         if (nk_input_is_mouse_hovering_rect(in, bounds)) {
//           if (nk_tooltip_begin(ctx, 0)) {
//             nk_layout_row_dynamic(ctx, 0.0f, 1);
//             nk_label(ctx, "Frost", NK_TEXT_ALIGN_LEFT);
//             nk_label(ctx, "Frost desc", NK_TEXT_ALIGN_LEFT);
//             nk_layout_row_static(ctx, 0.0f, 300, 2);
//             nk_label(ctx, "attr: ", NK_TEXT_ALIGN_LEFT);
//             nk_label_colored(ctx, "-10%", NK_TEXT_ALIGN_RIGHT, nk_color{255,0,0,255});
//           }
//         }
//       }
//       nk_end(ctx);
      
      // еще инвентарь
    }
    
    void player_interface::info_message(const std::string &string) {
      if (strings.empty()) current_time_info = 0;
      strings.push_back(string);
    }
    
    void player_interface::game_message(const std::string &string) {
      if (game_messages.empty()) current_time_game = 0;
      game_messages.push(string);
    }
    
    void complex_indices_graphics::draw() {
      if (core::deleted_state(ent)) return;
      
      //auto opt = Global::get<GeometryGPUOptimizer>();
      auto opt = Global::get<render::geometry_optimizer>();
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      auto states = ent->at<components::states>(game::entity::states);
      
      if (states->current == nullptr) return;
      
      // по идее этого достаточно
      // теперь когда у меня текстурка упакована в 32 бита, я могу свободно ее передавать
      // осталось придумать хороший контейнер, чтобы не сильно усложнял стейт
      for (const auto &face : model_faces) {
        const uint32_t texture_index = face.state->frame.texture_offset;
        ASSERT(face.state->frame.images_count == 1);
        
        const render::geometry_optimizer::object_indices idx{
          trans.valid() ? trans->index() : UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          texture_index,
          face.offset,
          face.count,
          face.index,
          0
        };
        opt->add(idx);
      }
    }
    
    void complex_indices_graphics::debug_draw() {}
  }
  
  lower_panel_sizes::lower_panel_sizes(const float &panel_width, const float &panel_height, const float &offset_x, const float &offset_y) : 
    width(panel_width), 
    height(panel_height), 
    panel_offset_x(offset_x), 
    panel_offset_y(offset_y) 
  {
    {
      const float face_height = height-2;
      const float face_width = face_height; //height;
      const float face_offset_x = width/2.0f-face_width/2.0f;
      face_panel = nk_rect(face_offset_x, panel_offset_y, face_width, face_height);
      face_image = nk_rect(face_panel.x+2, face_panel.y+2, face_panel.w-4, face_panel.h-4);
    }
    
    const float left_width = face_panel.x-panel_offset_x;
    const float left_panel_width = left_width/2.0f-panel_offset_x*2;
    
    const float left_panel_ratio[] = {0.25f, 0.25f, 0.5f};
    
    {
      attrib_panel = nk_rect(panel_offset_x, panel_offset_y, left_width*left_panel_ratio[0]-panel_offset_x*2, height);
    }
    
    {
      armor_panel = nk_rect(attrib_panel.x+attrib_panel.w+panel_offset_x*2, panel_offset_y, left_width*left_panel_ratio[1]-panel_offset_x*2, height);
    }
    
    {
      skills_panel = nk_rect(armor_panel.x+armor_panel.w+panel_offset_x*2, panel_offset_y, left_width*left_panel_ratio[2]-panel_offset_x*2, height);
    }
    
    {
      ammo_panel = nk_rect(face_panel.x+face_panel.w+panel_offset_x*2, panel_offset_y, left_panel_width, height);
    }
    
    {
      inventory_panel = nk_rect(ammo_panel.x+ammo_panel.w+panel_offset_x*2, panel_offset_y, left_panel_width, height);
    }
  }
  
  attrib_panel_sizes::attrib_panel_sizes(const struct nk_rect &attrib_panel, const float &border) {
    const float name_size = 31;
    
    {
      const float y = (attrib_panel.h-name_size-border)/2.0f-36;
      attrib_value = nk_rect(-5, y, attrib_panel.w+5, attrib_panel.h);
    }
    
    {
      attrib_value_shadow = nk_rect(attrib_value.x+5, attrib_value.y+3, attrib_value.w, attrib_value.h);
    }
    
    {
      const float y = attrib_panel.h-name_size-border;
      attrib_name = nk_rect(0, y, attrib_panel.w, attrib_panel.h-y);
    }
    
    {
      attrib_name_shadow = nk_rect(attrib_name.x+3, attrib_name.y+2, attrib_name.w, attrib_name.h);
    }
  }
}
