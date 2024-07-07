/**
 * \file rendering/shader_compiler.hpp
 */
#ifndef OTHER_ENGINE_SHADER_COMPILER_HPP
#define OTHER_ENGINE_SHADER_COMPILER_HPP

#include "parsing/shader_parser.hpp"

#include "rendering/shader.hpp"


namespace other {

  class ShaderCompiler {
    public:
      ShaderCompiler(ShaderType type , const std::string& src)
          : shader_src(src) {}

      ShaderIr Compile();

    private:
      ShaderType type;

      std::string shader_src;
  };

} // namespace other

#endif // !OTHER_ENGINE_SHADER_COMPILER_HPP

