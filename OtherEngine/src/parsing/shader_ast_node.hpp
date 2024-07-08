/**
 * \file parsing/shader_ast_node.hpp
 **/
#ifndef OTHER_ENGIEN_SHADER_AST_NODE_HPP
#define OTHER_ENGIEN_SHADER_AST_NODE_HPP

#include <ostream>
#include <vector>

#include "parsing/parsing_defines.hpp"
#include "parsing/ast_node.hpp"

#include "rendering/shader.hpp"

namespace other {

  class VersionExpr : public AstExpr {
    public:
      VersionExpr(const Token& version_number , const Token& version_type = Token({} , INVALID_TOKEN , "")) 
          : AstExpr(VERSION_EXPR) , version_number(version_number) , version_type(version_type) {}
      virtual ~VersionExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;

      Token version_number;
      Token version_type;
  };

  class ShaderAttribute : public AstExpr {
    public:
      ShaderAttribute(const Token& key , const Token& value)
          : AstExpr(SHADER_ATTRIBUTE_EXPR) , key(key) , value(value) {}
      virtual ~ShaderAttribute() override {}
      
      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;

      Token key , value;

  };

  class LayoutDescriptor : public AstExpr {
    public:
      LayoutDescriptor(Token type , const Ref<AstExpr>& expr)
        : AstExpr(LAYOUT_DESCRIPTOR) , type(type) , expr(expr) {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;

      Token type;
      Ref<AstExpr> expr;
  };

  class LayoutDecl : public AstStmt {
    public:
      LayoutDecl(Token layout_rules , std::vector<Ref<AstExpr>>& dtors , const Ref<AstStmt>& data) 
          : AstStmt(LAYOUT_DECL_STMT) , layout_rules(layout_rules) , data(data) {
        descriptors.swap(dtors);
      }

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;

      Token layout_rules;
      std::vector<Ref<AstExpr>> descriptors;
      Ref<AstStmt> data;
  };

  class LayoutVarDecl : public AstStmt {
    public:
      LayoutVarDecl(Token in_out , Opt<Token> type = std::nullopt , Opt<Token> identifier = std::nullopt)
          : AstStmt(LAYOUT_VAR_DECL_STMT) , in_out(in_out) , type(type) , name(identifier) {}
      virtual ~LayoutVarDecl() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;

      Token in_out;
      Opt<Token> type , name;
  };

  class ShaderStorageStmt : public AstStmt {
    public:
      ShaderStorageStmt(const Token& type , const Token& id , const Ref<AstStmt>& body)
        : AstStmt(SHADER_STORAGE_STMT) , type(type) , name(id) , body(body) {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;

      Token type;
      Token name;
      Ref<AstStmt> body;
  };

  class InOutBlockStmt : public AstStmt {
    public:
      InOutBlockStmt(Token in_out , Token tag , Token identifier , const Ref<AstStmt>& body , bool is_array = false)
        : AstStmt(IN_OUT_BLOCK_STMT) , in_out(in_out) , tag(tag) , name(identifier) , body(body) , is_array(is_array) {}
      virtual ~InOutBlockStmt() override {}
      
      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;

      Token in_out;
      Token tag;
      Token name;
      Ref<AstStmt> body;

      bool is_array;
  };

  class UniformDecl : public AstStmt {
    public:
      UniformDecl(const Ref<AstStmt>& var_decl)
          : AstStmt(UNIFORM_DECL_STMT) , var_decl(var_decl) {}
      virtual ~UniformDecl() override {}
      
      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;

      Ref<AstStmt> var_decl;
  };
  
  class ShaderDecl : public AstStmt {
    public:
      ShaderDecl(ShaderType type , std::vector<Ref<AstExpr>>& attrs , std::vector<Ref<AstStmt>>& stmts) 
          : AstStmt(SHADER_DECL_STMT) , type(type) {
        attributes.swap(attrs);
        statements.swap(stmts);
      }
      virtual ~ShaderDecl() override {}
      
      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;
      
      ShaderType type;
      std::vector<Ref<AstExpr>> attributes;
      std::vector<Ref<AstStmt>> statements;
  };

} // namespace other

#endif // !OTHER_ENGIEN_SHADER_AST_NODE_HPP
