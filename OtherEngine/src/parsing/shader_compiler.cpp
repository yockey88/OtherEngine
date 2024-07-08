/**
 * \file parsing\shader_compiler.cpp
 **/
#include "parsing/shader_compiler.hpp"

#include "parsing/shader_preprocessor.hpp"
#include "parsing/shader_lexer.hpp"
#include "parsing/shader_parser.hpp"
#include "parsing/shader_glsl_transpiler.hpp"

namespace other {
      
  ShaderIr ShaderCompiler::Compile(const std::string& src) {
    return Compile(OTHER_SHADER , src);
  }

  ShaderIr ShaderCompiler::Compile(ShaderType type , const std::string& src) {
    ShaderPreprocessor preprocessor(src , type);
    ShaderProcessedFile processed_shader = preprocessor.Process();

    ShaderLexer lexer(processed_shader);
    ShaderLexResult tokens = lexer.Lex();

    ShaderParser parser(tokens);
    ShaderAst ast = parser.Parse();

    ShaderGlslTranspiler transpiler(ast);
    return transpiler.Transpile();
  }

} // namespace other
