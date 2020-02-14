#include "pathfinder_system.h"

#include "astar_search.h"
#include "graph.h"
#include "vertex_component.h"
#include "ThreadPool.h"

namespace devils_engine {
  namespace systems {
    pathfinder::pathfinder(const create_info &info) : pool(info.pool), graph(info.graph), search(new utils::astar_search[pool->size()+1]) {
      for (size_t i = 0; i < pool->size()+1; ++i) {
        stack.push_back(&search[i]);
      }
    }
    
    pathfinder::~pathfinder() {
      for (auto &t : types) {
        for (auto q : t.queue) {
          if (q.responce.path != nullptr) paths_pool.deleteElement(q.responce.path);
        }
      }
      delete [] search;
    }
    
    void pathfinder::register_type(const utils::id &type, const std::function<bool(const components::vertex*, const components::vertex*, const graph::edge*)> &predicate, const float &offset) {
      types.push_back({});
      types.back().offset = offset;
      types.back().predicate = predicate;
      types.back().id = type;
    }
    
    void pathfinder::queue_request(const path::request &request) {
      const size_t index = find_type(request.id);
      if (index == SIZE_MAX) throw std::runtime_error("Could not find path finding type");
      
      const path::response ret{
        path::find_state::delayed,
        nullptr,
        1
      };
      
      {
        std::unique_lock<std::mutex> lock(types[index].mutex);
        
        const size_t queue_index = find_req(types[index].queue, request.start, request.end);
        if (queue_index != SIZE_MAX) {
          ++types[index].queue[queue_index].responce.counter;
          return;
        }
        
        types[index].queue.push_back({request.start, request.end, ret});
      }
    }
    
    struct raw_path_piece {
      const components::vertex* vertex;
      const graph::edge* edge;
    };
    
    void pathfinder::update() {
      using local_itr = std::vector<type::queue_data>::iterator;
      static const auto pathFinding = [&] (const utils::astar_search::predicate_f &predicate, const float &funnelOffset, local_itr itr) {
        //if (itr->second.path != nullptr && itr->second.state == path_finding_state::has_path) return;
        // здесь можно поставить условие, чтобы мы не обрабатывали запрос, которыей уже ранее был обработан
        // для того чтобы заного его обработать, его нужно передобавить
        if (itr->responce.state != path::find_state::delayed) return;
        
        utils::astar_search* searchPtr = nullptr;
        {
          std::unique_lock<std::mutex> lock(stack_mutex);
          searchPtr = stack.back();
          stack.pop_back();
        }
        
        searchPtr->set(itr->start, itr->end, predicate);
        
        utils::astar_search::state state = utils::astar_search::state::searching;
        while (state == utils::astar_search::state::searching) {
          state = searchPtr->step();
        }
        
        if (state == utils::astar_search::state::invalid || 
            state == utils::astar_search::state::not_initialised || 
            state == utils::astar_search::state::out_of_memory) {
          throw std::runtime_error("Something wrong with searching");
        }
        
        if (state == utils::astar_search::state::failed) {
          // ошибка
    //       searchPtr->freeSolutionNodes();
          
          itr->responce.path = nullptr;
          itr->responce.state = path::find_state::not_exist;
          
          std::unique_lock<std::mutex> lock(stack_mutex);
          stack.push_back(searchPtr);
          return;
        }
        
    //     PRINT_VAR("start", itr->first.first)
    //     PRINT_VAR("goal ", itr->first.second)
        
        // мы уже можем стоять на месте гоала, нужно вернуть только один нод
        const auto &solution = searchPtr->solution();
    //     for (size_t i = 0; i < solution.size(); ++i) {
    //       PRINT_VAR("index ", i)
    //       PRINT_VAR("node  ", solution[i])
    //       PRINT_VAR("vertex", solution[i]->vertex)
    //     }
    //     
    //     if (solution.size() < 2) throw std::runtime_error("dl;ma;krnkba';");
        
        path::container* path = nullptr;
        {
          std::unique_lock<std::mutex> lock(pool_mutex);
          path = pathsPool.newElement();
          std::cout << "path " << path << " created" << "\n";
        }
        
        std::vector<path::raw> raw_path(solution.size());
        for (size_t i = 0; i < solution.size(); ++i) {
          const path::raw p{
            solution[i]->vertex,
            i+1 == solution.size() ? nullptr : solution[i+1]->edge // solution[i]->vertex->edge(solution[i+1]->vertex), // solution[i]->edge
          };
          raw_path[i] = p;
        }
        
        searchPtr->free_solution();
        {
          std::unique_lock<std::mutex> lock(stack_mutex);
          stack.push_back(searchPtr);
        }
        searchPtr = nullptr;
        
        computeFunnel(path, funnelOffset);
        
        for (size_t i = 0; i < path->array.size(); ++i) {
          PRINT_VAR("index", i)
          PRINT_VEC3("point1", path->array[i].pos)
    //       PRINT_VEC3("point2", path->funnelData()[i].another)
          PRINT_VEC3("dir  ", path->array[i].dir)
        }
        
    //     throw std::runtime_error("no more");
        
        {
    //       std::unique_lock<std::mutex> lock(types[type].mutex);
          itr->responce.path = path;
          itr->responce.state = path::find_state::exist;
        }
        
        // выглядит это как то так
      };
      
      // тут нам еще нужно добавить балансирование нагрузки
      // как оно должно выглядеть? я должен часть работы спихивать на другой кадр
      // скорее всего мне нужно задать какой нибудь предел выполнения задач на этом кадре
      // то есть число после которого я перестаю запихивать команды в очередь, 
      // ситуация когда у меня каждый раз добавляется в очередь больше задач чем предел в кадре
      // ТЕОРЕТИЧЕСКИ возможна, но на практике скорее всего нет, главное чтобы число было как можно больше
      
      size_t taskCount = 0;
      for (auto &type : types) {
        for (auto itr = type.queue.begin(); itr != type.queue.end(); ++itr) {
          pool->submitnr(pathFinding, type.predicate, type.offset, itr);
          ++taskCount;
          // макс размер
        }
      }
      
      pool->compute();
      pool->wait();
    }
    
    path::response pathfinder::get_path(const path::request &req) {
      const size_t index = find_type(req.id);
      if (index == SIZE_MAX) throw std::runtime_error("Could not find path finding type");
      
      std::unique_lock<std::mutex> lock(types[index].mutex);
      const size_t queue_index = find_req(types[index].queue, req.start, req.end);
      if (queue_index != SIZE_MAX) return types[index].queue[queue_index].responce;
      return {
        path::find_state::not_exist,
        nullptr,
        0
      };
    }
    
    void pathfinder::release_path(const path::request &req) {
      const size_t index = find_type(req.id);
      if (index == SIZE_MAX) throw std::runtime_error("Could not find path finding type");
      
      std::unique_lock<std::mutex> lock(types[index].mutex);
      const size_t queue_index = find_req(types[index].queue, req.start, req.end);
      if (queue_index == SIZE_MAX) return;
      
      auto &resp = types[index].queue[queue_index].responce;
      --resp.counter;
      if (resp.counter == 0) {
        std::unique_lock<std::mutex> lock(pool_mutex);
        std::cout << "path " << resp.path << " deleted" << "\n";
        paths_pool.deleteElement(resp.path);
        resp.path = nullptr;
      }
      
      std::swap(types[index].queue[queue_index], types[index].queue.back());
      types[index].queue.pop_back();
    }
    
    void pathfinder::compute_funnel(const std::vector<path::raw> &raw_path, path::container* path, const float &offset) {
      // тут мне нужно заполнить данные всех вершин, то есть точку справа (или все же ближайшую?),
      // направление от этой точки (наверное еще длину ребра), точку в которую я направляюсь
      
      // в незаполненных позициях фуннел поинт заполняется нулями
      simd::vec4 apex, leftP, rightP, left, right;
      size_t apexIndex = 0, leftIndex = 0, rightIndex = 0;
      
      size_t firstIndex = 0;
      const simd::vec4 center = raw_path[firstIndex].vertex->center();
      const simd::vec4 normal = raw_path[firstIndex].vertex->normal();
      apex = center;
      
      path->array.push_back({apex, simd::vec4(0.0f)});
      
      simd::vec4 firstA;
      simd::vec4 firstB;
      const graph::edge* edge = raw_path[firstIndex].edge;
      if (edge->is_fake()) {
        float b = 0.5f / glm::tan(edge->angle);
        
        if (b > edge->seg.distance()) edge->seg.left_right(center, normal, firstA, firstB);
        else {
          utils::line_segment l(edge->seg.point_a(), edge->seg.direction(), b); // edge->getSegment().a + edge->getSegment().dir()*b
          l.left_right(center, normal, firstA, firstB);
        }
      } else {
        edge->seg.left_right(center, normal, firstA, firstB);
      }
      
      leftP = firstA;
      rightP = firstB;
      
      // нужно ко всем точкам добавлять оффсет, как это сделать правильно?
      // причем для fakeEdge нужно наоборот вычитать
      // у нас есть левая и правая точка и вектор направления
      // нам нужно сместить левую и правую точки ближе к центру на небольшой оффсет
      // причем он может быть совсем маленьким, только лишь для того чтобы точки на углах не были похожи 1к1
      const simd::vec4 tmpEdgeDir = edge->seg.direction();
      correct_corner_points(tmpEdgeDir, edge->seg.distance(), offset, leftP, rightP);
      
      for (size_t i = 1; i < raw_path.size(); ++i) {
        const auto vertex = raw_path[i].vertex;
        
        const simd::vec4 center = vertex->center();
        const simd::vec4 normal = vertex->normal();
        
        // обработка fake edge, нужно передать правильно высоту + ступеньки, как разные ступеньки организовать
        // по идее мы на стадии поиска должны их выщелкивать, но я не помню работает ли это
        if (i+1 != raw_path.size()) {
          const auto edge = raw_path[i].edge;
          
          if (edge->is_fake()) {
            float b = 0.5f / glm::tan(edge->angle);
            
            if (b > edge->seg.distance()) edge->seg.left_right(center, normal, left, right);
            else {
              utils::line_segment l(edge->seg.point_a(), edge->seg.direction(), b);
              l.left_right(center, normal, left, right);
            }
          } else {
            edge->seg.left_right(center, normal, left, right);
          }
          
          const simd::vec4 tmpEdgeDir = edge->seg.direction();
          correctCornerPoints(tmpEdgeDir, edge->seg.distance(), offset, left, right);
        } else {
          left = center;
          right = center;
        }
        
        // тут наверное нужно использовать нормаль гравитации (?)
        if (sideOf(apex, rightP, right, normal) <= 0.0f) {
          if (simd::all(simd::equal(apex, rightP, EPSILON)) || sideOf(apex, leftP, right, normal) > 0.0f) {
            rightP = right;
            rightIndex = i;
            
            // точка внутри воронки
          } else {
            const auto edge = raw_path[leftIndex].edge;
            if (edge == nullptr) {
              path->array.push_back({leftP, simd::vec4(0.0f)});
            } else {
              const simd::vec4 &aS = edge->seg.point_a();
              const simd::vec4 &bS = edge->seg.point_b();
              const bool isLeft = simd::all(simd::equal(leftP, aS, EPSILON));
              const simd::vec4 dir = simd::normalize(bS - aS);
//               float arr[4];
//               dir.storeu(arr);
//               const simd::vec4 finalDir = isLeft ? simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i)) : simd::vec4(-arr[0], -arr[1], -arr[2], glm::uintBitsToFloat(i));
              path->array.push_back({leftP, dir});
            }
            
            apex = leftP;
            apexIndex = leftIndex;
            leftP = apex;
            rightP = apex;
            leftIndex = apexIndex;
            rightIndex = apexIndex;
            
            i = apexIndex;
            continue;
          }
        }
        
        if (sideOf(apex, leftP, left, normal) >= 0.0f) {
          if (simd::all(simd::equal(apex, leftP, EPSILON)) || sideOf(apex, rightP, left, normal) < 0.0f) {
            leftP = left;
            leftIndex = i;
            
            // точка внутри воронки
          } else {
            const auto edge = raw_path[rightIndex].edge;
            if (edge == nullptr) {
              path->array.push_back({rightP, simd::vec4(0.0f)});
            } else {
              const simd::vec4 &aS = edge->seg.point_a();
              const simd::vec4 &bS = edge->seg.point_b();
              const bool isRight = simd::all(simd::equal(rightP, aS, EPSILON));
              const simd::vec4 dir = simd::normalize(bS - aS);
//               float arr[4];
//               dir.storeu(arr);
//               const simd::vec4 finalDir = isRight ? simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i)) : simd::vec4(-arr[0], -arr[1], -arr[2], glm::uintBitsToFloat(i));
              path->array.push_back({rightP, dir});
            }
            
            apex = rightP;
            apexIndex = rightIndex;
            leftP = apex;
            rightP = apex;
            leftIndex = apexIndex;
            rightIndex = apexIndex;
            
            i = apexIndex;
            continue;
          }
        }
      }
      
      const simd::vec4 &lastPoint = raw_path.back().vertex->center();
      path->array.push_back({lastPoint, simd::vec4(0.0f)});
    }
    
    void pathfinder::correct_corner_points(const simd::vec4 &dir, const float &width, const float &offset, simd::vec4 &left, simd::vec4 &right) {
      // предположительно dir это направление от одной из точек к другой, width это растояние между точками, offset - требуемое отстояние
      const float finalOffset = std::min(offset, width/2.0f);
      if (finalOffset < EPSILON) return;
      
      const simd::vec4 &localDir = right - left;
      const float dot = simd::dot(localDir, dir);
      const simd::vec4 &finalDir = dot > 0.0f ? dir : -dir;
      left = left + finalDir * finalOffset;
      right = right - finalDir * finalOffset;
    }
    
    size_t pathfinder::find_type(const utils::id &id) const {
      for (size_t i = 0; i < types.size(); ++i) {
        if (types[i].id == id) return i;
      }
      return SIZE_MAX;
    }
    
    size_t pathfinder::find_req(const std::vector<type::queue_data> &queue, const components::vertex* start, const components::vertex* end) const {
      for (size_t i = 0; i < queue.size(); ++i) {
        if (queue[i].start == start && queue[i].end == end) return i;
      }
      return SIZE_MAX;
    }
  }
}
