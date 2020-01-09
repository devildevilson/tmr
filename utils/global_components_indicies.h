#ifndef GLOBAL_COMPONENTS_INDICIES_H
#define GLOBAL_COMPONENTS_INDICIES_H

// этот enum очень помогает не плодить миллиард указателей на компоненты
// но не решает вопрос с компонентами типо компонентов графики
// но с другой стороны мы можем задать вообще все компоненты в этом энуме
// но тогда будет большой массив компонентов заполненный вообще почти ничем
// информационный компонент нужно поставить выше
// в общем нужно отсортировать по количеству использований
// в особых случаях можно использовать обычный доступ к компонентам

// возможно нужно выделить память COMPONENTS_COUNT указателей для каждого энтити
// но это не точно

// для того чтобы это нормально работало возможно нужно сделать отложенную инициализацию у некоторых компонентов
// основная проблема с физикой наверное
// ну и проблема с тем чтобы я запомнил что нужно дополнять список

enum default_components_indices {
  INFO_COMPONENT_INDEX,
  USER_DATA_COMPONENT_INDEX,
  TRANSFORM_COMPONENT_INDEX,
  INPUT_COMPONENT_INDEX,
  PHYSICS_COMPONENT_INDEX,
  GRAPHICS_COMPONENT_INDEX,
  ANIMATION_COMPONENT_INDEX,
  SOUND_COMPONENT_INDEX,
  ATTRIBUTE_COMPONENT_INDEX,
  EFFECT_COMPONENT_INDEX,
  STATE_CONTROLLER_INDEX,
  MOVEMENT_COMPONENT_INDEX,
  INVENTORY_COMPONENT_INDEX,
  WEAPONS_COMPONENT_INDEX,
  ABILITIES_COMPONENT_INDEX,
  // тут еще добавятся
  AI_COMPONENT_INDEX,
  COMPONENTS_COUNT
};

#endif
