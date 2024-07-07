/**
 * \file rendering/shader_compiler.hpp
 */
#ifndef OTHER_ENGINE_SHADER_COMPILER_HPP
#define OTHER_ENGINE_SHADER_COMPILER_HPP

#include "rendering/shader.hpp"
#include "rendering/uniform.hpp"

namespace other {

  struct ShaderStorages {
    /// type (std140 , std430)
    std::string name;
    std::vector<Uniform> uniforms;
  };
  
  struct ShaderIr {
    std::vector<Uniform> uniform;
    std::vector<ShaderStorages> storages;
    std::string glsl_source = ""; 
  };

  class ShaderCompiler {
    public:
      ShaderCompiler(ShaderType type , const std::string& src)
          : shader_src(src) {}

      ShaderIr Compile();

    private:
      ShaderType type;

      std::string shader_src;

      ShaderIr result;
  };

} // namespace other

#endif // !OTHER_ENGINE_SHADER_COMPILER_HPP

