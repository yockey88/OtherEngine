/**
 * \file parsing/shader_parser.hpp
 **/
#ifndef OTHER_ENGINE_SHADER_PARSER_HPP
#define OTHER_ENGINE_SHADER_PARSER_HPP

#include "core/defines.hpp"
#include "core/errors.hpp"

#include "parsing/parsing_defines.hpp"
#include "parsing/ast_node.hpp"

namespace other {

  class ShaderAst {
    public:
      std::vector<Ref<AstNode>> nodes;
  };

  class ShaderParser {
    public:
      ShaderParser(const std::vector<Token>& tokens)
        : tokens(tokens) {}

      ShaderAst Parse();

    private:
      bool valid_input = false;

      size_t current = 0;
      std::vector<Token> tokens;

      ShaderAst result;

      struct Flags {
        bool initializer_valid = true;
      } flags;

      std::string ProcessSource(const std::string& source);

      Ref<AstStmt> ParseDecl();
      Ref<AstStmt> ResolveDecl();

      Ref<AstStmt> ParseLayoutDecl();
      Ref<AstStmt> ParseFunctionDecl(const Token& type , const Token& name);
      Ref<AstStmt> ParseStructDecl(const Token& name);
      
      // const Token& name , const Token& type = Token(SourceLocation{} , INVALID_TOKEN , "")
      Ref<AstStmt> ParseVarDecl(const Token& type , const Token& name);
      Ref<AstStmt> ParseArrayDecl(const Token& type , const Token& name);

      Ref<AstStmt> ParseBufferDecl();

      Ref<AstStmt> ParseLayoutStmt();

      Ref<AstStmt> ParseStatement();
      Ref<AstStmt> ParseInOutBlock(const Token& type);
      Ref<AstStmt> ParseBlock();                                                                          
      Ref<AstStmt> ParseIf();
      Ref<AstStmt> ParseWhile();
      Ref<AstStmt> ParseFor();
      Ref<AstStmt> ParseReturn();

      Ref<AstExpr> ParseLayoutDescriptor();

      Ref<AstExpr> HandleExpr();
      Ref<AstExpr> ParseExpression();                                                                     
      Ref<AstExpr> ParseAssignment();
      Ref<AstExpr> ParseOr();
      Ref<AstExpr> ParseAnd();
      Ref<AstExpr> ParseEquality();
      Ref<AstExpr> ParseComparison();
      Ref<AstExpr> ParseTerm();
      Ref<AstExpr> ParseFactor();
      Ref<AstExpr> ParseCall();
      Ref<AstExpr> FinishCall(Ref<AstExpr>& callee);
      Ref<AstExpr> ParseLiteral();
      Ref<AstExpr> ResolveExpr();

      Ref<AstExpr> ParseArrayExpr(Token size);
      Ref<AstExpr> ParseArrayAccess(Ref<AstExpr>& expr);
      Ref<AstExpr> ParseStructAccess(Ref<AstExpr>& expr);

      /**
       * declaration (layout, type, struct, ...)
       *
       * statement
       *
       * expression
       **/

      Token Peek() const;
      Token Previous() const;
      Token Advance();
      Token Consume(TokenType type, const std::string& message);
      Token ConsumeType(const std::string& message);

      bool AtEnd() const;
      bool Check(TokenType type) const;
      bool Match(const std::vector<TokenType>& types , bool advance = true);
      bool MatchLiterals(bool advance = true);
      bool MatchTypes(bool advance = true);

      template <typename... Args>
      ShaderException Error(ShaderError error , std::string_view message, Args&&... args) const {
        return ShaderException(fmtstr(message , std::forward<Args>(args)...) , error);
      }
  }; 

} // namespace other

#endif // !OTHER_ENGINE_SHADER_PARSER_HPP
