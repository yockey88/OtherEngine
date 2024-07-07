/**
 * \file parsing/shader_ast_node.hpp
 **/
#ifndef OTHER_ENGIEN_SHADER_AST_NODE_HPP
#define OTHER_ENGIEN_SHADER_AST_NODE_HPP

#include "parsing/ast_node.hpp"
#include "parsing/parsing_defines.hpp"
#include <vector>

namespace other {

  class VersionExpr : public AstExpr {
    public:
      VersionExpr(const Token& version_number , const Token& version_type = Token({} , INVALID_TOKEN , "")) 
          : AstExpr(VERSION_EXPR) , version_number(version_number) , version_type(version_type) {}
      virtual ~VersionExpr() override {}

      virtual void Stream(std::ostream& stream) const override;

      Token version_number;
      Token version_type;
  };

  class LayoutDescriptor : public AstExpr {
    public:
      LayoutDescriptor(Token type , const Ref<AstExpr>& expr)
        : AstExpr(LAYOUT_DESCRIPTOR) , type(type) , expr(expr) {}

      virtual void Stream(std::ostream& stream) const override;

      Token type;
      Ref<AstExpr> expr;
  };

  class LayoutDecl : public AstStmt {
    public:
      LayoutDecl(Token layout_rules , std::vector<Ref<AstExpr>>& dtors , const Ref<AstStmt>& data) 
          : AstStmt(LAYOUT_DECL_STMT) , layout_rules(layout_rules) , data(data) {
        descriptors.swap(dtors);
      }

      virtual void Stream(std::ostream& stream) const override;

      Token layout_rules;
      std::vector<Ref<AstExpr>> descriptors;
      Ref<AstStmt> data;
  };
  
  class LayoutStmt : public AstStmt {
    public:
      LayoutStmt()
        : AstStmt(LAYOUT_STMT) {}

      virtual void Stream(std::ostream& stream) const override;
  };

  class ShaderStorageStmt : public AstStmt {
    public:
      ShaderStorageStmt(const Token& type , const Token& id , const Ref<AstStmt>& body)
        : AstStmt(SHADER_STORAGE_STMT) , type(type) , name(id) , body(body) {}

      virtual void Stream(std::ostream& stream) const override;

      Token type;
      Token name;
      Ref<AstStmt> body;
  };

} // namespace other

#endif // !OTHER_ENGIEN_SHADER_AST_NODE_HPP
