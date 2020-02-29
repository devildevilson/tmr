#include "graphics_component.h"

#include "EntityComponentSystem.h"
#include "Globals.h"
#include "GPUOptimizers.h"
#include "global_components_indicies.h"
#include "TransformComponent.h"
#include "states_component.h"
#include "shared_mathematical_constant.h"
#include "Physics.h"

#include "game_funcs.h"
#include "core_funcs.h"

class states_loader;

namespace devils_engine {
  namespace components {
    void sprite_graphics::draw() {
      if (core::deleted_state(ent)) return;
      
      auto opt = Global::get<MonsterGPUOptimizer>();
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
      
      const MonsterGPUOptimizer::GraphicsIndices idx{
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
      
      auto opt = Global::get<GeometryGPUOptimizer>();
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      auto states = ent->at<components::states>(game::entity::states);
      
      if (states->current == nullptr) return;
      
      //std::cout << "sprite graphics state " << states->current->id.name() << "\n";
      
      // может ли тут быть ситуация
      // для стен врядли, хотя можно сделать интересные эффекты
      ASSERT(states->current->frame.images_count < 2);
      
      const uint32_t texture_index = states->current->frame.texture_offset;
      
      const GeometryGPUOptimizer::GraphicsIndices idx{
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
      
      // модификатор к позиции мы должны задавать извне
      // света не может быть у стен я так понимаю
      // у стен только параметр освещенности который мы задаем в карте
      
    }
    
    void player_sprite::draw(const size_t &time) {
      // нужно интерполировать смещение
      // интерполяция будет происходить исходя из константы
      // то есть константа - это 1.0f
    }
    
    void player_interface::draw(const size_t &time) {
      // нужно ли время?
      // здесь должны рисоваться такие вещи как: хп, мана, лицо перса, и проч
      // для этого скорее всего необходимо ввести функцию
      // рендер сообщений скорее всего нужно зафорсить
    }
    
    void player_interface::message(const std::string &string) {
      strings.push_back(string);
    }
  }
}
