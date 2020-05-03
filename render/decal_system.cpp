#include "decal_system.h"
#include "EntityComponentSystem.h"
#include "Globals.h"
#include "decals_container_component.h"
#include "global_components_indicies.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "EntityComponentSystem.h"
#include "shared_collision_constants.h"

namespace devils_engine {
  namespace systems {
    Container<simd::mat4>* decals::matrix = nullptr;
    Container<Transform>* decals::transforms = nullptr;
    
    decals::decals() : current_time(0) {}
    decals::~decals() {}
    
    #define DE_SQRT12 0.7071067811865475244008443621048490
    void plane_space(const simd::vec4 &n, simd::vec4 &p, simd::vec4 &q) {
      float arr_n[4];
      n.storeu(arr_n);
      float arr_p[4];
      float arr_q[4];
      memset(arr_p, 0, sizeof(float)*4);
      memset(arr_q, 0, sizeof(float)*4);
      if (std::abs(arr_n[2]) > DE_SQRT12) {
        float a = arr_n[1]*arr_n[1] + arr_n[2]*arr_n[2];
        float k = 1.0f / std::sqrt(a);
        arr_p[0] = 0.0f;
        arr_p[1] = -arr_n[2] * k;
        arr_p[2] =  arr_n[1] * k;
        arr_q[0] = a * k;
        arr_q[1] = -arr_n[0] * arr_p[2];
        arr_q[2] =  arr_n[0] * arr_p[1];
      } else {
        float a = arr_n[0]*arr_n[0] + arr_n[1]*arr_n[1];
        float k = 1.0f / std::sqrt(a);
        arr_p[0] = -arr_n[1] * k;
        arr_p[1] =  arr_n[0] * k;
        arr_p[2] = 0.0f;
        arr_q[0] = -arr_n[2] * arr_p[1];
        arr_q[1] =  arr_n[2] * arr_p[0];
        arr_q[2] = a * k;
      }
      p.loadu(arr_p);
      q.loadu(arr_q);
      
//       float arr[4];
//       n.storeu(arr);
//       if (fast_fabsf(arr[0]) < EPSILON && fast_fabsf(arr[1]) < EPSILON) {
//         p = simd::vec4(1.0f, 0.0f, 0.0f, 0.0f);
//         q = simd::vec4(0.0f, 1.0f, 0.0f, 0.0f);
//       } else {
//         p = simd::normalize(simd::vec4(-arr[1], arr[0], 0.0f, 0.0f));
//         q = simd::normalize(simd::vec4(-arr[0]*arr[2], -arr[1]*arr[2], arr[0]*arr[0] + arr[1]*arr[1], 0.0f));
//       }
    }
    
    void decals::update(const size_t &time, const size_t &update_delta) {
      current_time += time;
      if (current_time < update_delta) return;
      current_time -= update_delta;
      if (pending_decals.empty()) return;
      
      for (auto &decal : pending_decals) {
        const auto pairs = Global::get<PhysicsEngine>()->getOverlappingPairsData();
        const uint32_t &count = Global::get<PhysicsEngine>()->getOverlappingDataSize();
        
        for (uint32_t i = 0; i < count; ++i) {
          const auto &pair = pairs->at(i);
          
          const auto cont1 = Global::get<PhysicsEngine>()->getIndexContainer(pair.firstIndex);
          const auto cont2 = Global::get<PhysicsEngine>()->getIndexContainer(pair.secondIndex);
          
          if (decal.container.objectDataIndex != pair.firstIndex && decal.container.objectDataIndex != pair.secondIndex) continue;
          
          const auto obj_ptr = decal.container.objectDataIndex == pair.firstIndex ? cont2 : cont1;
          if (obj_ptr->userData == nullptr) continue;
          
          // нам требуется еще найти подходящую нормаль у сложного объекта
          // если перед нами сложный объект, нужно обойти все плоскости
          // как отделить их друг от друга? в текущем виде это сложно
          // предположить что normal.w - это всегда индекс первой точки
          
          const uint32_t objFacesSize = Global::get<PhysicsEngine>()->getObjectShapeFacesSize(obj_ptr);
          const uint32_t objPointsSize = Global::get<PhysicsEngine>()->getObjectShapePointsSize(obj_ptr);
          
          if (objPointsSize+1 == objFacesSize) {
            const simd::vec4* objPoints = Global::get<PhysicsEngine>()->getObjectShapePoints(obj_ptr);
            const simd::vec4* objFaces = Global::get<PhysicsEngine>()->getObjectShapeFaces(obj_ptr);
            compute_polygon(obj_ptr, objPointsSize, objPoints, objFaces[0], objPoints[objPointsSize], decal);
          } else {
            compute_complex(obj_ptr, decal);
          }
        }
        
        transforms->erase(decal.transform_index);
        Global::get<PhysicsEngine>()->remove(&decal.container);
        computed_decals.push_back({
          decal.decal,
          decal.matrix
        });
      }
      
      pending_decals.clear();
    }
    
    yacs::entity* decals::add(const pending_data &data) {
      const uint32_t matrix_index = matrix->insert(data.orn.get_simd());
      const uint32_t transform_index = transforms->insert({data.pos.get_simd(), simd::vec4(0.0f, 0.0f, 1.0f, 0.0f), simd::vec4(1.0f, 1.0f, 1.0f, 1.0f)});
      auto entity = Global::get<yacs::world>()->create_entity();
      entity->add<components::decal_data>(entity);
//       auto trans = entity->add<TransformComponent>(data.pos.get_simd(), simd::vec4(0.0f, 0.0f, 1.0f, 0.0f), simd::vec4(1.0f, 1.0f, 1.0f, 1.0f));
      // TODO: незабыть исправить индексы передаваемые в PhysicsObjectCreateInfo
      const PhysicsObjectCreateInfo obj{
        PhysicsType(true, BBOX_TYPE, false, true, false, false),
        UINT32_MAX, // coll group
        0,
        WALL_COLLISION_TYPE | DOOR_COLLISION_TYPE,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        //UINT32_MAX,
        0,
        transform_index,
        0,
        matrix_index,
        UINT32_MAX,
        Type::get("boxShape")
      };
//       auto physics = entity->add<PhysicsComponent>();
      PhysicsIndexContainer cont;
      Global::get<PhysicsEngine>()->add(obj, &cont);
      
      const pending_decal pending {
        data,
        cont,
        entity,
        matrix_index,
        transform_index
      };
      pending_decals.push_back(pending);
      return entity;
    }
    
    void decals::remove(yacs::entity* decal) {
      for (size_t i = 0; i < computed_decals.size(); ++i) {
        if (computed_decals[i].decal == decal) {
          Global::get<yacs::world>()->destroy_entity(computed_decals[i].decal);
          matrix->erase(computed_decals[i].matrix);
          std::swap(computed_decals[i], computed_decals.back());
          computed_decals.pop_back();
          break;
        }
      }
    }
    
    void decals::clip(const std::vector<simd::vec4> &inVerts, std::vector<simd::vec4> &outVerts, const simd::vec4 &normal, const float &dist) {
      outVerts.clear();
      if (inVerts.empty()) return;
      
      // тут наверное лучше транспозить с помощью simd
      const simd::vec4 tmp = normal + simd::vec4(0.0f, 0.0f, 0.0f, -dist);
      const simd::mat4 matNorm = simd::transpose(simd::mat4(tmp, tmp, tmp, tmp));
      
      simd::vec4 prev = inVerts.back();
      float prevDot = simd::dot(prev, tmp);
      for (size_t k = 0; k < inVerts.size(); k+=4) {
    //           simd::vec4 vert; // каждую вершину умножим на invBasis
        // если p1 находится внутри декали dot(vert, normalS), dot(vert, normalT), dot(vert, -normalS), dot(vert, -normalT) < 0.0f
        // и если p(size-1) находится внутри, добавляем p1
        
        float arrVert1[4];
        inVerts[k+0].storeu(arrVert1);
        float arrVert2[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        if (k+1 < inVerts.size()) inVerts[k+1].storeu(arrVert2);
        float arrVert3[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        if (k+2 < inVerts.size()) inVerts[k+2].storeu(arrVert3);
        float arrVert4[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        if (k+3 < inVerts.size()) inVerts[k+3].storeu(arrVert4);
        
        const simd::vec4 dotRes = simd::dot4(simd::vec4(arrVert1[0], arrVert2[0], arrVert3[0], arrVert4[0]), simd::vec4(arrVert1[1], arrVert2[1], arrVert3[1], arrVert4[1]), 
                                             simd::vec4(arrVert1[2], arrVert2[2], arrVert3[2], arrVert4[2]), simd::vec4(arrVert1[3], arrVert2[3], arrVert3[3], arrVert4[3]), 
                                             matNorm[0], matNorm[1], matNorm[2], matNorm[3]);
        
        PRINT_VEC4("dotRes", dotRes)
        
        float dotResArr[4];
        dotRes.storeu(dotResArr);
        
        const int res = (dotRes < simd::vec4(0.0f)).movemask() | ((prevDot < 0.0f)<<4);
        
        const uint8_t count = std::min(inVerts.size()-k-1, size_t(4));
        for (uint8_t v = 0; v < count; ++v) {
          const bool inside = bool(res & (1<<(3-v))); // pk
          const bool insidePrev = bool(res & (1<<(4-v))); // p(k-1)
          const size_t pkindex = k+v;
          const size_t pk1index = k+v == 0 ? inVerts.size()-1 : k+v-1;
          
          if (inside) {
            if (insidePrev) {
              // добавляем pk
              outVerts.push_back(inVerts[pkindex]);
            } else {
              // находим пересечение p(k-1)-pk с одной из нормалей
              // добавляем пересечение
              // добавляем pk
              
              simd::vec4 point;
              intersect(normal*dist, normal, inVerts[pkindex], inVerts[pk1index], point);
              outVerts.push_back(point);
              outVerts.push_back(inVerts[pkindex]);
            }
          } else {
            if (insidePrev) {
              // находим пересечение pk-p(k-1) с одной из нормалей
              // добавляем пересечение
              simd::vec4 point;
              intersect(normal*dist, normal, inVerts[pk1index], inVerts[pkindex], point);
              outVerts.push_back(point);
            }
          }
        }
        
        // p(k-1) = pk
        const size_t last = std::min(inVerts.size()-1, k+3);
        prev = inVerts[last];
        prevDot = dotResArr[last-k];
      }
    }
    
    simd::vec4 intersect(const simd::vec4 &point_a, const simd::vec4 &point_b, const simd::vec4 &normal, const simd::vec4 &normal_point) {
      const simd::vec4 dir = point_b - point_a;
      if (glm::abs(simd::dot(normal, point_b - point_a)) < EPSILON) return simd::vec4(glm::uintBitsToFloat(UINT32_MAX), glm::uintBitsToFloat(UINT32_MAX), glm::uintBitsToFloat(UINT32_MAX), glm::uintBitsToFloat(UINT32_MAX));
      
      float s = (simd::dot(normal, normal_point - point_a)) / (simd::dot(normal, dir));
      //s = glm::clamp(s, 0.0f, 1.0f);
      if (s < 0.0f || s >= 1.0f) return simd::vec4(glm::uintBitsToFloat(UINT32_MAX), glm::uintBitsToFloat(UINT32_MAX), glm::uintBitsToFloat(UINT32_MAX), glm::uintBitsToFloat(UINT32_MAX));
      
      //const simd::vec4 norm_dir = simd::normalize(dir);
      return point_a + dir * s;
    }
    
    bool inside_clip_edge(const simd::vec4 &point, const simd::vec4 &clip, const simd::vec4 &clip_point) {
      return simd::dot(clip, point - clip_point) > 0.0f;
    }
    
    void decals::clip(std::vector<simd::vec4> &inVerts, std::vector<simd::vec4> &outVerts, const std::vector<std::pair<simd::vec4, simd::vec4>> &clip_poly) {
      outVerts = inVerts;
      for (const auto &clip : clip_poly) {
        inVerts = outVerts;
        outVerts.clear();
        
        for (size_t i = 0; i < inVerts.size(); ++i) {
          const simd::vec4 current_point = inVerts[i];
          const simd::vec4 prev_point = inVerts[(i + inVerts.size() - 1) % inVerts.size()];
          
//           const simd::vec4 dir = prev_point - current_point;
//           if (glm::abs(simd::dot(dir, clip.first)) < EPSILON) {
//             //if ()
//           }
          
          const simd::vec4 intersect_point = intersect(prev_point, current_point, clip.first, clip.second);
          
          if (inside_clip_edge(current_point, clip.first, clip.second)) {
            if (!inside_clip_edge(prev_point, clip.first, clip.second)) {
              float arr[4];
              intersect_point.storeu(arr);
              ASSERT(glm::floatBitsToUint(arr[0]) != UINT32_MAX);
              outVerts.push_back(intersect_point);
            }
            
            outVerts.push_back(current_point);
          } else if (inside_clip_edge(prev_point, clip.first, clip.second)) {
            float arr[4];
            intersect_point.storeu(arr);
            ASSERT(glm::floatBitsToUint(arr[0]) != UINT32_MAX);
            outVerts.push_back(intersect_point);
          }
        }
        
      }
    }
    
    void decals::compute_polygon(const PhysicsIndexContainer* obj_ptr, const uint32_t &points_count, const simd::vec4* points_arr, const simd::vec4 &normal, const simd::vec4 &center, pending_decal &decal) {
      auto obj_ent = reinterpret_cast<yacs::entity*>(obj_ptr->userData);
      // есть ли у энтити котейнер для декалей
      auto decal_container = obj_ent->at<components::decals_container>(game::wall::decal_container);
      if (!decal_container.valid()) return;
      
      auto decal_data = decal.decal->get<components::decal_data>();
      
      const uint32_t objPointsSize = points_count;
      const simd::vec4* objPoints = points_arr;
      
      const simd::vec4 obj_center = center;
      
      const simd::vec4 wall_normal = normal;
      const simd::vec4 decal_normal = decal.data.orn.get_simd()[2];
      //const simd::vec4 decal_normal = wall_normal;
      if (simd::dot(wall_normal, decal_normal) < 0.0f) return; // вообще для многих стен с нормалью в другую сторону все что ниже выполняется
      
      // посмотрел код декалей в кваке: основная задача - создать некую фигуру
      // на основе которой отсечь внешнюю, по отношению к фигуре, геометрию
      // и создать новый полигон из оставшейся
      
      // ВАЖНО!!! декаль у нас может пересечь стену уже немного пододвинутую (то есть открытую дверь)
      // это значит что точки объекта нужно сначало привести в мировые координаты
      // как это сделать? по идее нужно rotation * matrix * (pos+point) и вот это умножить на invBasis
      // и тоже самое нужно сделать (только в обратную сторону) когда мы вычисляем позицию уже непосредственно декалины
      // то есть базис это не просто ориентация декали, а orn * rotation * matrix и проще будет наверное еще сделать трансформ матрицу
      
//           simd::vec4 p, q;
//           plane_space(wall_normal, p, q);
//           p = simd::normalize(p);
//           q = simd::normalize(q);
      
      const float width = 1.0f;
      const float height = decal.data.scale;
      
      //const float sizeX = width * decal.data.size;
      //const float sizeY = height * decal.data.size;
      
      //const float halfSizeX = sizeX / 2.0f;
      //const float halfSizeY = sizeY / 2.0f;
      
      const uint32_t matrixIndex = Global::get<PhysicsEngine>()->getMatrixIndex(obj_ptr);
      const uint32_t transformIndex = Global::get<PhysicsEngine>()->getTransformIndex(obj_ptr);
      // ротатион
      const simd::mat4 objMatrix = matrixIndex == UINT32_MAX ? simd::mat4(1.0f) : matrix->at(matrixIndex);
      const simd::vec4 trans = transformIndex == UINT32_MAX ? simd::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(transformIndex).pos;
      const simd::vec4 scale = transformIndex == UINT32_MAX ? simd::vec4(1.0f, 1.0f, 1.0f, 1.0f) : transforms->at(transformIndex).scale;
      const simd::mat4 objBasis = simd::scale(simd::mat4(1.0f), scale) * simd::translate(simd::mat4(1.0f), trans) * objMatrix;
      const simd::mat4 invObjBasis = simd::inverse(objBasis);
      
      // как то так по идее
      const simd::mat4 data_mat = decal.data.orn.get_simd();
      const simd::mat4 decalMatrix = simd::mat4(data_mat[0], data_mat[1], data_mat[2], simd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
      const simd::mat4 decal_translation = simd::translate(simd::mat4(1.0f), -decal.data.pos.get_simd());
      const simd::mat4 final_decal_mat = decalMatrix * decal_translation; // simd::inverse(
      const simd::mat4 translate_mat2 = simd::translate(simd::mat4(1.0f), simd::vec4(0.5f, 0.5f, 0.0f, 1.0f)); // нужен ли отступ?
      
      const simd::vec4 proj = projectPointOnPlane(decal_normal, decal.data.pos.get_simd(), obj_center);
      
      const simd::mat4 translate_obj = simd::translate(simd::mat4(1.0f), -proj);
      const simd::mat4 obj_orn = simd::orientation(-wall_normal, decal_normal); //simd::inverse(
      const simd::mat4 translate_obj_back = simd::translate(simd::mat4(1.0f), proj);
      const simd::mat4 obj_transform_final = translate_obj_back * obj_orn * translate_obj;

      const simd::mat4 ortho = simd::ortho(-width, width, -height, height);
      
      // что я тут делаю? для того чтобы получить uv координаты можно перевести точки полигона в пространство uv
      // как это делается? 
      // 1) все точки приводим в мировые координаты, 
      // 2) поворачиваем точки относительно проекции на плоскость декали,
      // 3) приводим в пространство декали, 
      // 4) отступ для uv
      // 5) приводим в координаты [(0,0), (1,1)] с помощью ortho матрицы
      // нужно проверить как будет работать на более сложных поверхностях
      
//                                       5)         4)              3)                 2)              1)
      const simd::mat4 final_mat = ortho * translate_mat2 * final_decal_mat * obj_transform_final * objBasis;
      const simd::mat4 final_inv_mat = simd::inverse(final_mat); // возвращаем обратно в мировые координаты
      
      // заведем массив вершин
      // посчитаем их в новом спейсе
      std::vector<simd::vec4> verts;
      for (uint32_t k = 0; k < objPointsSize; ++k) {
        verts.push_back(final_mat * objPoints[k]);
      }
      
      //const simd::vec4 normals[3] = {simd::normalize(final_mat * p), simd::normalize(final_mat * q), simd::normalize(final_mat * wall_normal)};
      //const simd::vec4 decal_pos = decal.data.pos.get_simd();
      //const simd::vec4 center = simd::vec4(0.5f, 0.5f, 0.0f, 1.0f);
      //const simd::vec4 half = simd::vec4(0.5f, 0.5f, 0.5f, 0.0f);
      const std::vector<std::pair<simd::vec4, simd::vec4>> clip_edges = {
        std::make_pair(simd::vec4( 0.0f, 1.0f, 0.0f, 0.0f), simd::vec4(0.0f, 0.0f, 0.0f, 1.0f)),
        std::make_pair(simd::vec4(-1.0f, 0.0f, 0.0f, 0.0f), simd::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
        std::make_pair(simd::vec4( 0.0f,-1.0f, 0.0f, 0.0f), simd::vec4(1.0f, 1.0f, 0.0f, 1.0f)),
        std::make_pair(simd::vec4( 1.0f, 0.0f, 0.0f, 0.0f), simd::vec4(0.0f, 1.0f, 0.0f, 1.0f)),
        std::make_pair(simd::vec4( 0.0f, 0.0f, 1.0f, 0.0f), simd::vec4(0.0f, 0.0f,-0.5f, 1.0f)),
        std::make_pair(simd::vec4( 0.0f, 0.0f,-1.0f, 0.0f), simd::vec4(0.0f, 0.0f, 0.5f, 1.0f)),
      };
      
      // начинаем клипать, клипать лучше всего по каждой нормали по отдельности
      std::vector<simd::vec4> clippedVerts;
      clip(verts, clippedVerts, clip_edges);
      
      // если по итогу точек останется меньше 3, то значит это не то что нам нужно
      if (verts.size() < 3) return;
      
      // и после всего этого мы должны составить фигуру
      // то есть умножить точки обратно на базис
      // приладить к этому нормаль
      // и посчитать текстурную координату

      float normalArr[4];
      wall_normal.storeu(normalArr);
      std::vector<render::vertex> vertices;
      for (size_t k = 0; k < clippedVerts.size(); ++k) {
        float arr[4];
        clippedVerts[k].storeu(arr);
        
        // все вершины сейчас у нас в пространстве uv
        const float texX = arr[0];
        const float texY = arr[1];
        
        const render::vertex vert{
          final_inv_mat * clippedVerts[k], // 0.001f * wall_normal + 
          simd::vec4(normalArr[0], normalArr[1], normalArr[2], 0.0f),
          glm::vec2(texX, texY)
        };
        vertices.push_back(vert);
      }
      
      decal_data->add(obj_ent);
      decal_container->add({
        decal.decal,
        vertices,
        decal.data.state
      });
    }
    
    void decals::compute_complex(const PhysicsIndexContainer* obj_ptr, pending_decal &decal) {
      auto obj_ent = reinterpret_cast<yacs::entity*>(obj_ptr->userData);
      // есть ли у энтити котейнер для декалей
      auto decal_container = obj_ent->at<components::decals_container>(game::wall::decal_container);
      if (!decal_container.valid()) return;
      
      const uint32_t objPointsSize = Global::get<PhysicsEngine>()->getObjectShapePointsSize(obj_ptr);
      const simd::vec4* objPoints = Global::get<PhysicsEngine>()->getObjectShapePoints(obj_ptr);
      const uint32_t objFacesSize = Global::get<PhysicsEngine>()->getObjectShapeFacesSize(obj_ptr);
      const simd::vec4* objFaces = Global::get<PhysicsEngine>()->getObjectShapeFaces(obj_ptr);
      
      // нужно гарантировать что objFaces.w всегда указывает на первую точку
      // и точки плоскости следуют один за другим в массиве
      
      for (uint32_t i = 0; i < objFacesSize; ++i) {
        float arr[4];
        objFaces[i].storeu(arr);
        
        const uint32_t point_index = glm::floatBitsToUint(arr[3]);
        
        float arr2[4];
        if (i + 1 < objFacesSize) objFaces[i+1].storeu(arr2);
        const uint32_t last_index = i + 1 < objFacesSize ? glm::floatBitsToUint(arr2[3]) : objPointsSize;
        const uint32_t points_count = last_index - point_index;
        const simd::vec4* points_ptr = &objPoints[point_index];
        simd::vec4 center = points_ptr[0];
        for (uint32_t i = 1; i < points_count; ++i) {
          center += points_ptr[i];
        }
        center /= float(points_count);
        
        compute_polygon(obj_ptr, points_count, points_ptr, simd::vec4(arr[0], arr[1], arr[2], 0.0f), center, decal);
      }
    }
  }
}
