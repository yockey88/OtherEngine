/**
 * \file parsing/shader_lexer.hpp
 **/
#ifndef OTHER_ENGINE_SHADER_LEXER_HPP
#define OTHER_ENGINE_SHADER_LEXER_HPP

#include "core/errors.hpp"

#include <vector>

#include "parsing/parsing_defines.hpp"
#include "parsing/shader_preprocessor.hpp"

namespace other {

  struct ShaderLexResult {
    std::vector<Token> tokens;
    ShaderType shader_type;
  };

  class ShaderLexer {
    public:
      ShaderLexer(const ShaderProcessedFile& processed_file)
          : file_data(processed_file) {
        result.shader_type = processed_file.shader_type; 
      }
      
      ShaderLexResult Lex();

    private:
      ShaderProcessedFile file_data;

      std::string current_token;
      SourceLocation loc;

      ShaderLexResult result;

      struct Flags {
        bool sign = false;
      } flags;

      void HandleWhitespace();

      void HandleNumeric();
      void HandleFloat();
      void HandleScientific();

      void HandleAlpha();
      void HandleAlphaNumeric();

      void HandleOperator();
      void HandleComment();

      void AddToken(TokenType type);

      void NewLine(bool advance = true);
      void Consume();
      void Advance();
      void DiscardToken();

      bool AtEnd() const;
      char Peek() const;
      char PeekNext() const;
      bool Check(char c) const;
      bool CheckNext(char c) const;
      bool Match(char expected);
      TokenType GetKeywordType(const std::string& str) const;      

      template <typename... Args>
      ShaderException Error(ShaderError error , std::string_view message, Args&&... args) const {
        return ShaderException(fmtstr(message , std::forward<Args>(args)...) , error , loc.line , loc.column);
      }
  };

} // namespace other

#endif // !OTHER_ENGINE_SHADER_LEXER_HPP
