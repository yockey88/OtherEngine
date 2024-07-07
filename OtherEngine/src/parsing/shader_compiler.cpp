/**
 * \file parsing\shader_compiler.cpp
 **/
#include "parsing/shader_compiler.hpp"

#include "parsing/shader_preprocessor.hpp"
#include "parsing/shader_lexer.hpp"
#include "parsing/shader_parser.hpp"
#include "parsing/shader_glsl_transpiler.hpp"

namespace other {

  ShaderIr ShaderCompiler::Compile() {
    ShaderPreprocessor preprocessor(shader_src);
    ProcessedFile processed_shader = preprocessor.Process();

    ShaderLexer lexer(processed_shader.src);
    auto tokens = lexer.Lex();

    ShaderParser parser(tokens);
    ShaderAst ast = parser.Parse();

    ShaderGlslTranspiler transpiler(ast);
    result.glsl_source = transpiler.Transpile();
    std::cout << "transpiled source :\n" 
              << result.glsl_source << "\n";

    return result;
  }

} // namespace other
