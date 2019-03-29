//#include "Graph.h"
#include "Graph.h"

edge_t::edge_t(Graph* graph, size_t graphIndex, const EdgeData &data) : active(true), graph(graph), graphIndex(graphIndex), data(data) {}
edge_t::~edge_t() {}

vertex_t* edge_t::first() const {
  return ends[0];
}

vertex_t* edge_t::second() const {
  return ends[1];
}

vertex_t* edge_t::operator[] (const uint8_t &index) const {
  return index > 1 ? nullptr : ends[index];
}

void edge_t::swapSides() {
  std::swap(ends[0], ends[1]);
}

void edge_t::setFirst(vertex_t* ptr) {
  ends[0] = ptr;
}

void edge_t::setSecond(vertex_t* ptr) {
  ends[1] = ptr;
}

void edge_t::set(const uint8_t &index, vertex_t* ptr) {
  if (index > 1) return;
  
  ends[index] = ptr;
}

bool edge_t::isFake() const {
  return data.isFakeEdge;
}

float edge_t::getDistance() const {
  return data.distance;
}

float edge_t::getWidth() const {
  return data.width;
}

float edge_t::getAngle() const {
  return data.angle;
}

float edge_t::getHeight() const {
  return data.height;
}

glm::vec4 edge_t::getDir() const {
  return data.segDir;
}

LineSegment edge_t::getSegment() const {
  return data.segment;
}

bool edge_t::isValid() const {
  return graph != nullptr;
}

bool edge_t::isActive() const {
  return active;
}

void edge_t::setActive(const bool active) {
  this->active = active;
}

size_t edge_t::internalIndex() const {
  return graphIndex;
}

vertex_t::vertex_t(Graph* graph, size_t graphIndex, const Vertex &data) : active(true), graph(graph), graphIndex(graphIndex), vertex(data) {}
vertex_t::~vertex_t() {}

size_t vertex_t::degree() const {
  return edges.size();
}

edge_t* vertex_t::operator[] (const size_t &index) const {
  return edges[index];
}

bool vertex_t::hasEdge(const vertex_t* other, edge_t** edge) const {
  for (const auto tmpEdge : edges) {
    if (tmpEdge->first() == other || tmpEdge->second() == other) {
      *edge = tmpEdge;
      return true;
    }
  }
  
  return false;
}

bool vertex_t::hasEdge(const vertex_t* other) const {
  for (const auto tmpEdge : edges) {
    if (tmpEdge->first() == other || tmpEdge->second() == other) return true;
  }
  
  return false;
}

vertex_t* vertex_t::neighbor(const size_t &index) const {
  if (this == edges[index]->first()) return edges[index]->second();
  
  return edges[index]->first();
}

void vertex_t::addEdge(edge_t* edge) {
  edges.push_back(edge);
}

void vertex_t::removeEdge(edge_t* edge) {
  for (const auto tmpEdge : edges) {
    if (tmpEdge == edge) {
      std::swap(edge, edges.back());
      edges.pop_back();
      return;
    }
  }
}

Vertex vertex_t::getVertexData() const {
  return vertex;
}

float vertex_t::getAngle(const vertex_t* other) const {
  edge_t* edge;
  const bool ret = this->hasEdge(other, &edge);
  
  if (ret) return edge->getAngle();
  
  return -1.0f;
}

float vertex_t::goalDistanceEstimate(const vertex_t* nodeGoal) const {
  //throw std::runtime_error("kmwvnenjc;lemw;onveowvnvon");
  return glm::distance(vertex.center, nodeGoal->vertex.center);
}

// bool vertex_t::getSuccessors(AStarSearch* astarSearch, const vertex_t* parentNode) const {
//   
// }

float vertex_t::getCost(const vertex_t* successor) const {
//   throw std::runtime_error("kmwvnenjc;lemw;onveowvnvon");
  return glm::distance(vertex.center, successor->vertex.center);
  
  // скорее всего это быстрее чем
//   edge_t* edge;
//   const bool ret = this->hasEdge(successor, &edge);
//   if (ret) return edge->getDistance();
//   return -1.0f;
}

bool vertex_t::isValid() const {
  return graph != nullptr;
}

bool vertex_t::isActive() const {
  return active;
}

void vertex_t::setActive(const bool active) {
  this->active = active;
}

size_t vertex_t::internalIndex() const {
  return graphIndex;
}

Graph::Graph() {}
Graph::~Graph() {
  for (size_t i = 0; i < verts.size(); ++i) {
    vertexPool.deleteElement(verts[i]);
  }
  vertexPool.clear();
  
  for (size_t i = 0; i < edgs.size(); ++i) {
    edgePool.deleteElement(edgs[i]);
  }
  edgePool.clear();
}

bool Graph::null() const {
  return verts.empty();
}

size_t Graph::order() const {
  return verts.size();
}

bool Graph::empty() const {
  return edgs.empty();
}

size_t Graph::size() const {
  return edgs.size();
}

edge_t* Graph::edge(const size_t &index) {
  return edgs[index];
}

const edge_t* Graph::edge(const size_t &index) const {
  return edgs[index];
}

vertex_t* Graph::vertex(const size_t &index) {
  return verts[index];
}

const vertex_t* Graph::vertex(const size_t &index) const {
  return verts[index];
}

size_t Graph::getIndex(const vertex_t* vert) const {
  return vert->internalIndex();
}

vertex_t* Graph::addVertex(const Vertex &data) {
  vertex_t* vert = vertexPool.newElement(this, verts.size(), data);
  verts.push_back(vert);
  
  return vert;
}

void Graph::deleteVertex(vertex_t* vertex) {
  if (this != vertex->graph) return;
  
  for (size_t i = 0; i < vertex->degree(); ++i) {
    auto edge = vertex->edge(i);
    auto neighbor = edge->first() == vertex ? edge->second() : edge->first();
    neighbor->removeEdge(edge);
    
    edgs.back()->graphIndex = edge->graphIndex;
    std::swap(edgs[edge->graphIndex], edgs.back());
    edgs.pop_back();
    
    edgePool.deleteElement(edge);
  }
  
  verts.back()->graphIndex = vertex->graphIndex;
  std::swap(verts[vertex->graphIndex], verts.back());
  verts.pop_back();
  
  vertexPool.deleteElement(vertex);
}

void Graph::deleteAdjacent(const edge_t* edge, const vertex_t* vert) {
  if (this != vert->graph) return;
  
  auto vertex = edge->first() == vert ? edge->second() : edge->first();
  
  for (size_t i = 0; i < vertex->degree(); ++i) {
    auto edge = vertex->edge(i);
    auto neighbor = edge->first() == vertex ? edge->second() : edge->first();
    neighbor->removeEdge(edge);
    
    edgs.back()->graphIndex = edge->graphIndex;
    std::swap(edgs[edge->graphIndex], edgs.back());
    edgs.pop_back();
    
    edgePool.deleteElement(edge);
  }
  
  verts.back()->graphIndex = vertex->graphIndex;
  std::swap(verts[vertex->graphIndex], verts.back());
  verts.pop_back();
  
  vertexPool.deleteElement(vertex);
}

edge_t* Graph::addEdge(vertex_t* vert1, vertex_t* vert2, const EdgeData &data) {
  edge_t* edge = edgePool.newElement(this, edgs.size(), data);
  edge->setFirst(vert1);
  edge->setSecond(vert2);
  vert1->addEdge(edge);
  vert2->addEdge(edge);
  
  edgs.push_back(edge);
  
  return edge;
}

void Graph::deleteEdge(vertex_t* vert1, vertex_t* vert2) {
  edge_t* edge;
  const bool ret = vert1->hasEdge(vert2, &edge);
  if (ret) {
    vert1->removeEdge(edge);
    vert2->removeEdge(edge);
    
    edgs.back()->graphIndex = edge->graphIndex;
    std::swap(edgs[edge->graphIndex], edgs.back());
    edgs.pop_back();
    
    edgePool.deleteElement(edge);
  }
}

void Graph::deleteEdge(edge_t* edge) {
  edge->first()->removeEdge(edge);
  edge->second()->removeEdge(edge);
  
  edgs.back()->graphIndex = edge->graphIndex;
  std::swap(edgs[edge->graphIndex], edgs.back());
  edgs.pop_back();
  
  edgePool.deleteElement(edge);
}
