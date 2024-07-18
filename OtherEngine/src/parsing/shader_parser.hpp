/**
 * \file parsing/shader_parser.hpp
 **/
#ifndef OTHER_ENGINE_SHADER_PARSER_HPP
#define OTHER_ENGINE_SHADER_PARSER_HPP

#include <stack>

#include "core/defines.hpp"
#include "core/errors.hpp"
#include "core/logger.hpp"

#include "parsing/parsing_defines.hpp"
#include "parsing/ast_node.hpp"
#include "parsing/shader_lexer.hpp"

namespace other {

  struct ShaderAst {
    ShaderType type;
    std::vector<Ref<AstNode>> vertex_nodes;
    std::vector<Ref<AstNode>> fragment_nodes;
    std::vector<Ref<AstNode>> geometry_nodes;
  };

  class ShaderParser {
    public:
      ShaderParser(ShaderLexResult& shader_lex_result) {
        tokens.swap(shader_lex_result.tokens);
        result.type = shader_lex_result.shader_type;
        OE_DEBUG("Shader Type {}" , result.type);
        switch (result.type) {
          case VERTEX_SHADER:
            start_context = VERTEX_CTX;
          break;
          case FRAGMENT_SHADER:
            start_context = FRAGMENT_CTX;
          break;
          case GEOMETRY_SHADER:
            start_context = GEOMETRY_CTX;
          break;
          case OTHER_SHADER: 
            start_context = VARIABLE_CTX; 
          break;
          default:
            throw Error(INVALID_SHADER_TYPE , "Unsupported shader type being parsed!");
        }

        current_context = start_context;
      }

      ShaderAst Parse();

    private:
      bool valid_input = false;

      size_t current = 0;
      std::vector<Token> tokens;

      ShaderAst result;

      struct Flags {
        std::stack<bool> initializer_blocker;
        std::stack<bool> scoping;
      } flags;

      enum ShaderTypeContext {
        VERTEX_CTX ,
        FRAGMENT_CTX , 
        GEOMETRY_CTX ,
        /// variable as in multiple shader types in one file
        VARIABLE_CTX ,
      } current_context;
      ShaderTypeContext start_context;

      ShaderType GetCurrentShaderType();

      Ref<AstStmt> ParseDecl();

      Ref<AstStmt> ParseShaderDecl();

      Ref<AstStmt> ParseLayoutDecl();
      Ref<AstStmt> ParseFunctionDecl(const Token& type , const Token& name);
      Ref<AstStmt> ParseStructDecl(const Token& name);
      
      Ref<AstStmt> ParseUniformDecl();
      Ref<AstStmt> ParseVarDecl();
      Ref<AstStmt> ParseArrayDecl(const Token& type , const Token& name , bool is_const = false);

      Ref<AstStmt> ParseBufferDecl();

      Ref<AstStmt> ParseLayoutStmt();

      Ref<AstStmt> ParseStatement();
      Ref<AstStmt> ParseInOutBlock(const Token& type , const Token& tag);
      Ref<AstStmt> ParseBlock();                                                                          
      Ref<AstStmt> ParseIf();
      Ref<AstStmt> ParseWhile();
      Ref<AstStmt> ParseFor();
      Ref<AstStmt> ParseReturn();

      Ref<AstExpr> ParseShaderAttribute();

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

      void AddTopLevelNode(const Ref<AstStmt>& node);

      template <typename... Args>
      ShaderException Error(ShaderError error , std::string_view message, Args&&... args) const {
        return ShaderException(fmtstr(message , std::forward<Args>(args)...) , error , 
                               tokens[current].location.line , tokens[current].location.column);
      }
  }; 

} // namespace other

#endif // !OTHER_ENGINE_SHADER_PARSER_HPP
