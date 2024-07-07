/**
 * \file parsing\shader_compiler.cpp
 **/
#include "parsing/shader_compiler.hpp"

#include "core/errors.hpp"
#include "core/filesystem.hpp"

#include "parsing/shader_preprocessor.hpp"
#include "parsing/shader_lexer.hpp"
#include "parsing/shader_parser.hpp"

namespace other {

  ShaderIr ShaderCompiler::Compile() {
    ShaderPreprocessor preprocessor(shader_src);
    ProcessedFile processed_shader = preprocessor.Process();

    ShaderLexer lexer(processed_shader.src);
    auto tokens = lexer.Lex();

    ShaderParser parser(tokens);
    return parser.Parse();
  }

} // namespace other
