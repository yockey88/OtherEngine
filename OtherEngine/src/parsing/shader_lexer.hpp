/**
 * \file parsing/shader_lexer.hpp
 **/
#ifndef OTHER_ENGINE_SHADER_LEXER_HPP
#define OTHER_ENGINE_SHADER_LEXER_HPP

#include "core/errors.hpp"

#include "parsing/parsing_defines.hpp"

namespace other {

  class ShaderLexer {
    public:
      ShaderLexer(const std::string& src)
        : src(src) {}
      
      std::vector<Token> Lex();

    private:
      std::string src;

      std::string current_token;
      SourceLocation loc;

      std::vector<Token> tokens;

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
        return ShaderException(fmtstr(message , std::forward<Args>(args)...) , error);
      }
  };

} // namespace other

#endif // !OTHER_ENGINE_SHADER_LEXER_HPP
