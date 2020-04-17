#include "physics_context.h"
#include "broadphase_context.h"

namespace devils_engine {
  namespace physics {
    namespace core {
      void context::gravity_data::set_gravity(const vec4 &gravity) {
        this->gravity = gravity;
        const float length2 = simd::length2(gravity);
        const float length = length2 > EPSILON ? glm::sqrt(length2) : 0.0f;
        gravity_norm = length2 > EPSILON ? gravity / length : vec4(0,0,0,0);
        gravity_scalars = vec4(length, length2, 0, 0);
        orientation = length2 > EPSILON ? simd::orientation(gravity_norm, vec4(0.0f, 1.0f, 0.0f, 0.0f)) : mat4();
      }
      
      scalar context::gravity_data::length() const {
        float arr[4];
        gravity_scalars.storeu(arr);
        return arr[0];
      }
      
      scalar context::gravity_data::length2() const {
        float arr[4];
        gravity_scalars.storeu(arr);
        return arr[1];
      }
      
      context::context(const create_info &info) : transforms(info.transforms), gravity_data(new struct gravity_data), free_body(UINT32_MAX), broadphase(nullptr) {
        gravity_data->set_gravity(info.initial_gravity);
      }
      
      context::~context() {
        delete gravity_data;
      }
      
      uint32_t context::add_body(const rigid_body::create_info &info, void* user_data) {
        std::unique_lock<std::mutex> lock(mutex);
        
        uint32_t index = UINT32_MAX;
        
        ASSERT(broadphase != nullptr);
        
        if (new_transforms.size() != transforms->size()) {
          new_transforms.resize(transforms->size());
          old_transforms.resize(transforms->size());
        }
        
        if (free_body == UINT32_MAX) {
          index = bodies.size();
          bodies.array().emplace_back(info, this);
          bodies.update();
          user_datas.push_back(user_data);
        } else {
          index = free_body;
          free_body = bodies[index].proxy_index;
          bodies[index] = rigid_body(info, this);
          user_datas[index] = user_data;
        }
        
        new_transforms[bodies[index].transform_index] = transforms->at(bodies[index].transform_index);
        old_transforms[bodies[index].transform_index] = transforms->at(bodies[index].transform_index);
        
        const auto &shape = shapes[info.shape_index];
        bodies[index].proxy_index = broadphase->create_proxy(index, info.collision_group, info.collision_filter, info.collision_trigger, shape.type);
        return index;
      }
      
      void context::remove_body(const uint32_t &id) {
        std::unique_lock<std::mutex> lock(mutex);
        
        ASSERT(id < bodies.size());
        broadphase->destroy_proxy(bodies[id].proxy_index);
        bodies[id].transform_index = UINT32_MAX;
        bodies[id].shape_index = UINT32_MAX;
        bodies[id].proxy_index = free_body;
        free_body = id;
      }
      
      void* context::get_user_data(const uint32_t &id) const {
        std::unique_lock<std::mutex> lock(mutex);
        
        ASSERT(id < bodies.size());
        if (bodies[id].valid()) return user_datas[id];
        return nullptr;
      }
      
      void context::add_joint(joint_interface* joint) {
        joints.push_back(joint);
      }
      
      void context::remove_joint(joint_interface* joint) {
        for (size_t i = 0; i < joints.size(); ++i) {
          if (joints[i] == joint) {
            std::swap(joints[i], joints.back());
            joints.pop_back();
            return;
          }
        }
      }
      
      uint32_t context::create_collision_shape(const id &shape_id, const uint32_t &type, const shape_creation_data &info, const scalar &margin) {
        ASSERT(type < collision::shape::max_type);
        
        if (type == collision::shape::box) {
          ASSERT(info.faces_points.size() == 0);
          ASSERT(info.faces.size() == 1);
          
          uint32_t shape_index = shapes.size();
          shapes_indices.insert(std::make_pair(shape_id, shape_index));
          shapes.push_back({
            type,
            static_cast<uint32_t>(points.size()),
            8,
            1,
            margin
          });
          
          points.push_back(info.faces[0]);
          
          return shape_index;
        }
        
        if (type == collision::shape::sphere) {
          ASSERT(info.faces_points.size() == 0);
          ASSERT(info.faces.size() == 1);
          
          float arr[4];
          info.faces[0].storeu(arr);
          
          uint32_t shape_index = shapes.size();
          shapes_indices.insert(std::make_pair(shape_id, shape_index));
          shapes.push_back({
            type,
            static_cast<uint32_t>(points.size()),
            1,
            glm::floatBitsToUint(arr[0]),
            margin
          });
          
          return shape_index;
        }
        
        ASSERT(info.faces_points.size() == info.faces.size());
        
        uint32_t shape_index = shapes.size();
        shapes_indices.insert(std::make_pair(shape_id, shape_index));
        shapes.push_back({
          type,
          static_cast<uint32_t>(points.size()),
          static_cast<uint32_t>(info.points.size()),
          static_cast<uint32_t>(info.faces.size()),
          margin
        });
        
        vec4 center = vec4(0,0,0,0);
        for (const auto &p : info.points) {
          center += p;
        }
        center /= scalar(info.points.size());
        
        for (const auto &p : info.points) {
          points.push_back(p);
        }
        
        points.push_back(center);
        
        for (size_t i = 0; i < info.faces.size(); ++i) {
          float arr[4];
          info.faces[i].storeu(arr);
          const vec4 new_face = vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(info.faces_points[i]));
          points.push_back(new_face);
        }
        
        ASSERT((shapes[shape_index].offset + shapes[shape_index].points_count + 1 + shapes[shape_index].faces_count) == points.size());
        
        return shape_index;
      }
      
      uint32_t context::collision_shape_index(const id &shape_id) {
        auto itr = shapes_indices.find(shape_id);
        if (itr == shapes_indices.end()) return UINT32_MAX;
        return itr->second;
      }
      
      uint32_t context::add_precision_ray(const collision::ray &ray) {
        std::unique_lock<std::mutex> lock(mutex);
        
        const uint32_t index = precision_rays.size();
        precision_rays.push_back(ray);
        
        if (precision_rays.size() + rays.size() > ray_test_data.size()) {
          ray_test_data.resize(precision_rays.size() + rays.size());
        }
        
        return index;
      }
      
      uint32_t context::add_ray(const collision::ray &ray) {
        std::unique_lock<std::mutex> lock(mutex);
        
        const uint32_t index = rays.size();
        rays.push_back(ray);
        
        if (precision_rays.size() + rays.size() > ray_test_data.size()) {
          ray_test_data.resize(precision_rays.size() + rays.size());
        }
        
        return index;
      }
      
      uint32_t context::add_frustum(const collision::frustum &frustum, const vec4 &pos) {
        std::unique_lock<std::mutex> lock(mutex);
        
        ASSERT(frustums.size() == frustums_pos.size());
        
        const uint32_t index = frustums.size();
        frustums.push_back(frustum);
        frustums_pos.push_back(pos);
        
        return index;
      }
      
      const collision::ray::test_data & context::get_precision_ray_intersection(const uint32_t &index) const {
        ASSERT(index < precision_rays.size());
        ASSERT(index < ray_test_data.size());
        return ray_test_data[index];
      }
      
      const collision::ray::test_data & context::get_ray_intersection(const uint32_t &index) const {
//         std::unique_lock<std::mutex> lock(mutex);
        
        ASSERT(index < rays.size());
        ASSERT(index+precision_rays.size() < ray_test_data.size());
        return ray_test_data[index+precision_rays.size()];
      }
      
      uint32_t context::frustum_objects_count() const {
        return frustum_pairs_count;
      }
      
      const collision::frustum::test_data & context::get_frustum_intersection(const uint32_t &index) const {
        ASSERT(index < frustum_pairs_count);
        ASSERT(index < frustum_test_data.size());
        return frustum_test_data[index];
      }
      
      void context::set_gravity(const vec4 &gravity) {
        gravity_data->set_gravity(gravity);
      }
      
      void context::begin() {
        // ??
      }
      
      void context::clear() {
        precision_rays.clear();
        rays.clear();
        frustums.clear();
        frustums_pos.clear();
      }
      
      size_t context::memory() const {
        return sizeof(*this) + 
               old_transforms.size()*sizeof(old_transforms[0]) + 
               new_transforms.size()*sizeof(new_transforms[0]) + 
               transforms->size()*sizeof(transforms->at(0)) + 
               bodies.size()*sizeof(bodies[0]) + 
               shapes.size()*sizeof(shapes[0]) + 
               points.size()*sizeof(points[0]) + 
               joints.size()*sizeof(joints[0]) + 
               precision_rays.size()*sizeof(precision_rays[0]) + 
               rays.size()*sizeof(rays[0]) + 
               frustums.size()*sizeof(frustums[0]) + 
               frustums_pos.size()*sizeof(frustums_pos[0]) + 
               ray_test_data.size()*sizeof(ray_test_data[0]) + 
               frustum_test_data.size()*sizeof(frustum_test_data[0]) + 
               shapes_indices.size()*sizeof(std::pair<id, uint32_t>) + 
               sizeof(*gravity_data) + 
               trigger_pairs.size()*sizeof(trigger_pairs[0]) +
               user_datas.size()*sizeof(user_datas[0]);
      }
    }
  }
}
