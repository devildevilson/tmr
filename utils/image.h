#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>

// размышления на тему того как лучше всего информацию о текстурке хранить
// суть в том что удобнее всего создавать пулы изображений по 256 слоев (для больших текстурок понятное дело меньше)
// так мы можем запихнуть в шейдер сразу все текстуры в игре, и рисовать с гораздо меньшим количеством 
// вызовов отрисовок, но можно пойти еще дальше: выделить статический пул в гпу памяти на определенное количество 
// текстурок разного размера, и в случае очень крупного тайтла, просто закидывать перед отрисовкой 
// необходимые текстурки в слоты, отрисовывать их, закидывать следующие, отрисовывать и тд
// так получается что для отрисовки любой сцены потребуется только определенный пул
// время тратится на копирование текстурок из памяти проца, но при этом мы не зависим от количества
// гпу памяти, причем мы можем посчитать что нужно куда скопировать заранее на гпу
// (нужен правда достаточно большой буфер), нужно два буфера: в один мы положим данные
// к отрисовке + переходы (например индексы вершин + картинка, момент когда начать копирование), 
// во второй положим данные о копировании, какой нибудь UINT32_MAX будет означать конец данных 
// отрисовки и начало копирования, и наоборот

namespace devils_engine {
  namespace render {
    struct image {
      static const uint32_t index_bits_count = UINT16_WIDTH;
      static const uint32_t layer_bits_count = UINT8_WIDTH;
      static const uint32_t sampler_bits_count = UINT8_WIDTH-2;
      static const uint32_t flip_v_bit_place = index_bits_count + layer_bits_count + sampler_bits_count;
      static const uint32_t flip_u_bit_place = index_bits_count + layer_bits_count + sampler_bits_count+1;
      static const uint8_t sampler_mask = 0b111111;
      
      uint32_t container;
      
      inline image() : container(0) {}
      inline image(const uint16_t &index, const uint8_t &layer, const uint8_t &sampler, const bool flip_u, const bool flip_v) : container(0) {
        container = container | index 
                              | (layer << index_bits_count)
                              | ((sampler & sampler_mask) << (index_bits_count+layer_bits_count))
                              | (uint32_t(flip_u) << flip_u_bit_place)
                              | (uint32_t(flip_v) << flip_v_bit_place);
      }
      
      inline uint16_t index() const {
        const uint32_t mask = 0xffff;
        return uint16_t(container & mask);
      }
      
      inline uint8_t layer() const { // слоев больше 256 делать не имеет смысла
        const uint8_t mask = 0xff;
        return uint8_t((container >> index_bits_count) & mask);
      }
      
      inline uint8_t sampler() const { // самплеров нужно не так чтобы очень много
        return uint8_t((container >> (index_bits_count+layer_bits_count)) & sampler_mask);
      }
      
      inline bool flip_u() const { // возмем два бита из самплерa
        const uint32_t mask = 1 << flip_u_bit_place;
        return (container & mask) == mask;
      }
      
      inline bool flip_v() const {
        const uint32_t mask = 1 << flip_v_bit_place;
        return (container & mask) == mask;
      }
    };
  }
}

#endif
