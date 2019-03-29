#ifndef SCENE_DATA_H
#define SCENE_DATA_H

// здесь будут всякие константные объекты

#include "RenderStructures.h"

#define countOf(x) (sizeof(x) / sizeof((x)[0]))

const Vertex monsterDefaultVertices[] = {
  {{-0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
  {{ 0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
  {{ 0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
  {{-0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const size_t monsterDefaultVerticesCount = countOf(monsterDefaultVertices);
const size_t monsterDefaultVerticesSize = monsterDefaultVerticesCount * sizeof(Vertex);

const Vertex cubeStripVertices[] = {
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

const size_t cubeStripVerticesCount = countOf(cubeStripVertices);
const size_t cubeStripVerticesSize = cubeStripVerticesCount * sizeof(Vertex);

#endif
