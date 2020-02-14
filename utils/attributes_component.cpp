#include "attributes_component.h"

#include "Utility.h"

namespace devils_engine {
  namespace components {
    attributes::attributes(const create_info &info) : ent(info.ent), float_size(info.float_init.size()), int_size(info.int_init.size()) {
      ASSERT(float_size <= max_float_attribs);
      ASSERT(int_size <= max_int_attribs);
      
      {
        size_t offset = 0;
        for (size_t i = 0; i < float_size; ++i) {
          char* mem = &float_attribs[offset];
          new (mem) core::attribute_t<core::float_type>(info.float_init[i].base, info.float_init[i].t);
          offset += sizeof(core::attribute_t<core::float_type>);
        }
      }
      
      {
        size_t offset = 0;
        for (size_t i = 0; i < float_size; ++i) {
          char* mem = &float_attribs[offset];
          new (mem) core::attribute_t<core::int_type>(info.int_init[i].base, info.int_init[i].t);
          offset += sizeof(core::attribute_t<core::int_type>);
        }
      }
    }
    
    void attributes::update(const size_t &time) {
      (void)time;
      
      auto float_ptr = getf();
      auto int_ptr = geti();
      
      for (size_t i = 0; i < float_size; ++i) {
        float_ptr[i].compute(ent);
      }
      
      for (size_t i = 0; i < int_size; ++i) {
        int_ptr[i].compute(ent);
      }
    }
    
    template <>
    const core::attribute_t<core::float_type>* attributes::find(const utils::id &type) const {
      return core::attribute_t<core::float_type>::const_finder(float_size, getf()).find(type);
    }
    
    template <>
    const core::attribute_t<core::int_type>* attributes::find(const utils::id &type) const {
      return core::attribute_t<core::int_type>::const_finder(int_size, geti()).find(type);
    }
    
    template <>
    core::attribute_t<core::float_type>* attributes::find(const utils::id &type) {
      return core::attribute_t<core::float_type>::finder(float_size, getf()).find(type);
    }
    
    template <>
    core::attribute_t<core::int_type>* attributes::find(const utils::id &type) {
      return core::attribute_t<core::int_type>::finder(int_size, geti()).find(type);
    }
    
//     core::attribute_t<core::float_type>::finder attributes::get() {
//       return core::attribute_t<core::float_type>::finder(float_size, getf());
//     }
//     
//     core::attribute_t<core::int_type>::finder attributes::get() {
//       return core::attribute_t<core::int_type>::finder(int_size, geti());
//     }
    
    core::attribute_t<core::float_type>* attributes::getf() {
      return reinterpret_cast<core::attribute_t<core::float_type>*>(float_attribs);
    }
    
    core::attribute_t<core::int_type>* attributes::geti() {
      return reinterpret_cast<core::attribute_t<core::int_type>*>(int_attribs);
    }
    
    const core::attribute_t<core::float_type>* attributes::getf() const {
      return reinterpret_cast<const core::attribute_t<core::float_type>*>(float_attribs);
    }
    
    const core::attribute_t<core::int_type>* attributes::geti() const {
      return reinterpret_cast<const core::attribute_t<core::int_type>*>(int_attribs);
    }
  }
}
