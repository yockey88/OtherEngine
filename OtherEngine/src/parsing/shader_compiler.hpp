/**
 * \file rendering/shader_compiler.hpp
 */
#ifndef OTHER_ENGINE_SHADER_COMPILER_HPP
#define OTHER_ENGINE_SHADER_COMPILER_HPP

#include "rendering/shader.hpp"

namespace other {

  class ShaderCompiler {
    public:
      static ShaderIr Compile(const std::string& src);
      static ShaderIr Compile(ShaderType type , const std::string& src);
  };

} // namespace other

#endif // !OTHER_ENGINE_SHADER_COMPILER_HPP

