#include "RAII.h"

#include "Internal.h"
#include "Core.h"
#include <fstream>

namespace yavf {
  namespace raii {
    ShaderModule::ShaderModule() : RAIIType1(FUNC_NAME_ENUM(vkCreateShaderModule)) {}
    ShaderModule::ShaderModule(Device* d, const VkShaderModuleCreateInfo &info) : RAIIType1(d->handle(), info, "vkCreateShaderModule", FUNC_NAME_ENUM(vkCreateShaderModule)) {}
    
    ShaderModule::ShaderModule(Device* d, const char* path) : RAIIType1(d->handle(), FUNC_NAME_ENUM(vkCreateShaderModule)) {
      auto file = std::ifstream(std::string(path), std::ios::binary);
      
      if (!file) {
        YAVF_ERROR_REPORT("addShader", "bad file path", 0)
      }
      
      file.seekg(0, std::ios::end);
      size_t length = file.tellg();
      
      std::vector<char> opcode(length);
      file.seekg(0, std::ios::beg);
      file.read(opcode.data(), opcode.size());
      
      const VkShaderModuleCreateInfo info{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        opcode.size(),
        (uint32_t*)opcode.data()
      };
      
      vkCheckError("vkCreateShaderModule", nullptr, 
      construct(d->handle(), info));
    }
  }
}
  
