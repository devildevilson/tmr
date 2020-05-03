#ifndef SCENE_DATA_H
#define SCENE_DATA_H

// здесь будут всякие константные объекты

// #include "RenderStructures.h"
#include "shared_structures.h"

#define countOf(x) (sizeof(x) / sizeof((x)[0]))

namespace devils_engine {
  namespace render {
    const vertex monster_default_vertices[] = {
      {{-0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{ 0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
      {{ 0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
      {{-0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };

    const size_t monster_default_vertices_count = countOf(monster_default_vertices);
    const size_t monster_default_vertices_size = monster_default_vertices_count * sizeof(vertex);

    const vertex cube_strip_vertices[] = {
      {{-0.5f,  0.5f,  0.5f, 1.0f}, { 0.0f, 0.0f,  1.0f, 0.0f}, {0.0f, 0.0f}}, // Front-top-left
      {{ 0.5f,  0.5f,  0.5f, 1.0f}, { 0.0f, 0.0f,  1.0f, 0.0f}, {1.0f, 0.0f}}, // Front-top-right
      {{-0.5f, -0.5f,  0.5f, 1.0f}, { 0.0f, 0.0f,  1.0f, 0.0f}, {0.0f, 1.0f}}, // Front-bottom-left
      {{ 0.5f, -0.5f,  0.5f, 1.0f}, { 0.0f, 0.0f,  1.0f, 0.0f}, {1.0f, 1.0f}}, // Front-bottom-right
      {{ 0.5f, -0.5f, -0.5f, 1.0f}, { 1.0f, 0.0f,  0.0f, 0.0f}, {1.0f, 1.0f}}, // Back-bottom-right
      {{ 0.5f,  0.5f,  0.5f, 1.0f}, { 1.0f, 0.0f,  0.0f, 0.0f}, {0.0f, 0.0f}}, // Front-top-right
      {{ 0.5f,  0.5f, -0.5f, 1.0f}, { 1.0f, 0.0f,  0.0f, 0.0f}, {1.0f, 0.0f}}, // Back-top-right
      {{-0.5f,  0.5f,  0.5f, 1.0f}, { 0.0f, 1.0f,  0.0f, 0.0f}, {0.0f, 0.0f}}, // Front-top-left
      {{-0.5f,  0.5f, -0.5f, 1.0f}, { 0.0f, 1.0f,  0.0f, 0.0f}, {1.0f, 0.0f}}, // Back-top-left
      {{-0.5f, -0.5f,  0.5f, 1.0f}, {-1.0f, 0.0f,  0.0f, 0.0f}, {1.0f, 1.0f}}, // Front-bottom-left
      {{-0.5f, -0.5f, -0.5f, 1.0f}, {-1.0f, 0.0f,  0.0f, 0.0f}, {0.0f, 1.0f}}, // Back-bottom-left
      {{ 0.5f, -0.5f, -0.5f, 1.0f}, { 0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // Back-bottom-right
      {{-0.5f,  0.5f, -0.5f, 1.0f}, { 0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // Back-top-left
      {{ 0.5f,  0.5f, -0.5f, 1.0f}, { 0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}  // Back-top-right
    };

    const size_t cube_strip_vertices_count = countOf(cube_strip_vertices);
    const size_t cube_strip_vertices_size = cube_strip_vertices_count * sizeof(vertex);
  }
}

#endif
