#include "overlay.h" 

#include <fmt/format.h>

#include "EntityComponentSystem.h"
#include "nuklear_header.h"
#include "TimeMeter.h"

// #include "InventoryComponent.h"
// #include "AttributesComponent.h"
// #include "EffectComponent.h"
#include "TransformComponent.h"

#include "Physics.h"
#include "Globals.h"

namespace devils_engine {
  namespace interface {
    overlay::overlay(const create_info &info) : nuklear(info.nuklear), player(info.player), tm(info.tm) {
//       inventory = player->get<InventoryComponent>().get();
//       attribs = player->get<AttributeComponent>().get();
//       effects = player->get<EffectComponent>().get();
      transform = player->get<TransformComponent>().get();
    }
    
    // пересчитал значения из TimeMeter, получилось что кадр занимает около 4 мс =(
    // как мы будем делать оверлэй? скорее всего подойдет композитор паттерн
    // вообще я так полагаю нужно сделать отдельные композиторы для основного оверлея и для меню
    
    void overlay::draw(const data::extent &screen_size) const {
      (void)screen_size;
      
      auto ctx = &nuklear->ctx;
      nk_style* s = &ctx->style;
      nk_color* oldColor = &s->window.background;
      nk_style_item* oldStyleItem = &s->window.fixed_background;
      nk_style_push_color(ctx, oldColor, nk_rgba(oldColor->r, oldColor->g, oldColor->b, int(0.5f*255)));
      nk_style_push_style_item(ctx, oldStyleItem, nk_style_item_color(nk_rgba(oldStyleItem->data.color.r, oldStyleItem->data.color.g, oldStyleItem->data.color.b, int(0.5f*255))));

      if (nk_begin(ctx, "Basic window", nk_rect(10, 10, 300, 240), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
        nk_style_set_font(ctx, &Global::get<interface::context>()->fonts[fonts::technical]->handle);
        {
          const simd::vec4 &pos = transform->pos();
          float arr[4];
          pos.store(arr);

          const auto &str = fmt::sprintf("Camera pos: (%.2f,%.2f,%.2f,%.2f)", arr[0], arr[1], arr[2], arr[3]);

          nk_layout_row_static(ctx, 10.0f, 300, 1); // ряд высотой 30, каждый элемент шириной 300, 1 столбец
          nk_label(ctx, str.c_str(), NK_TEXT_LEFT); // nk_layout_row_static скорее всего нужно указывать каждый раз
        }

        {
          const simd::vec4 &dir = transform->rot();
          float arr[4];
          dir.store(arr);

          const auto &str = fmt::sprintf("Camera dir: (%.2f,%.2f,%.2f)", arr[0], arr[1], arr[2]);

    //       nk_layout_row_static(ctx, 30.0f, 300, 1);
          nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
        }
        
        {
          const size_t average_frame_time = float(tm->accumulatedFrameTime()) / float(tm->framesCount());
          const auto &str = fmt::sprintf("Frame rendered in %lu mcs (%.2f fps)", average_frame_time, float(tm->framesCount() * TIME_PRECISION) / float(tm->accumulatedFrameTime()));
          // в скором сремени так уже будет нельзя считать фпс, во время отрисовки добавятся вычисления
          // последний интервал тоже очень сильно изменился лол, что не так?
          // это может быть связано с тем что у меня добавились вычисления ии, но чтоб на 1-2 мс странно
          //  last interval frame time %lu mcs data.lastFrameComputeTime

    //       nk_layout_row_static(ctx, 30.0f, 300, 1);
          nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
        }

        {
          const size_t average_sleep_time = float(tm->accumulatedSleepTime()) / float(tm->framesCount());
          const auto &str = fmt::sprintf("Sleep between frames equals %lu mcs", average_sleep_time);

    //       nk_layout_row_static(ctx, 30.0f, 300, 1);
          nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
        }

        {
          const auto &str = fmt::sprintf("Final fps is %.2f", tm->fps());

    //       nk_layout_row_static(ctx, 30.0f, 300, 1);
          nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
        }
        
        const uint32_t rayOutputCount = Global::get<PhysicsEngine>()->getRayTracingSize();
        const uint32_t frustumOutputCount = Global::get<PhysicsEngine>()->getFrustumTestSize();
        {
          const auto &str = fmt::sprintf("In frustum %zu objects", frustumOutputCount);

    //       nk_layout_row_static(ctx, 30.0f, 300, 1);
          nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
        }

        {
          const auto &str = fmt::sprintf("Ray collide %zu objects", rayOutputCount);

    //       nk_layout_row_static(ctx, 30.0f, 300, 1);
          nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
        }

        {
          const auto &str = fmt::sprintf("Player see %zu objects", 0);

    //       nk_layout_row_static(ctx, 30.0f, 300, 1);
          nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
        }
      }
      nk_end(ctx);

      nk_style_pop_color(ctx);
      nk_style_pop_style_item(ctx);
    }
  }
}
