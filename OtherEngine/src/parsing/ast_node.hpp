/**
 * \file parsing\ast_node.hpp
 **/
#ifndef OTHER_ENGINE_AST_NODE_HPP
#define OTHER_ENGINE_AST_NODE_HPP

#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "parsing/parsing_defines.hpp"
#include "parsing/tree_walker.hpp"


namespace other {

  class AstNode : public RefCounted {
    public:
      AstNodeType type;

    
      AstNode(AstNodeType type) 
        : type(type) {}
      virtual ~AstNode() = default;
      virtual void Stream(std::ostream& stream , TreeWalker& walker) const = 0;
      virtual void Accept(TreeWalker& walker) = 0;
      // virtual std::vector<Instruction> Emit() = 0;

      inline AstNodeType GetType() const {
        return type;
      }
  };

  class AstExpr : public AstNode {
    public:
      AstExpr(AstNodeType type) 
        : AstNode(type) {}
      virtual ~AstExpr() override {}
  };
  
  class AstStmt : public AstNode {
    public:
      AstStmt(AstNodeType type) 
        : AstNode(type) {}
      virtual ~AstStmt() override {}
  };

  class LiteralExpr : public AstExpr {
    public:
      LiteralExpr(Token value) 
        : AstExpr(AstNodeType::LITERAL_EXPR) , value(value) {}
      virtual ~LiteralExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      void Accept(TreeWalker& walker) override;
      // std::vector<Instruction> Emit() override;

      Token value;
  };

  class UnaryExpr : public AstExpr {
    public:
      UnaryExpr(Token op, const Ref<AstExpr>& right) 
        : AstExpr(UNARY_EXPR) , op(op), right(right) {}
      virtual ~UnaryExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      void Accept(TreeWalker& walker) override;
      // std::vector<Instruction> Emit() override;

      Token op;
      Ref<AstExpr> right;
    };

  class BinaryExpr : public AstExpr {
    public:
      BinaryExpr(const Ref<AstExpr>& left, Token op, const Ref<AstExpr>& right) 
        : AstExpr(AstNodeType::BINARY_EXPR) , left(left), op(op), right(right) {}
      virtual ~BinaryExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      void Accept(TreeWalker& walker) override;
      // std::vector<Instruction> Emit() override;

      Ref<AstExpr> left;
      Token op;
      Ref<AstExpr> right;
  };

  class CallExpr : public AstExpr {
    public:
      CallExpr(const Ref<AstExpr>& callee, std::vector<Ref<AstExpr>>& as) 
          : AstExpr(AstNodeType::CALL_EXPR) , callee(callee) {
        args.swap(as);
      }
      virtual ~CallExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      void Accept(TreeWalker& walker) override;
      // std::vector<Instruction> Emit() override;

      Ref<AstExpr> callee;
      std::vector<Ref<AstExpr>> args;
  };

  class GroupingExpr : public AstExpr {
    public:
      GroupingExpr(const Ref<AstExpr>& expr) 
        : AstExpr(AstNodeType::GROUPING_EXPR) , expr(expr) {}
      virtual ~GroupingExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      void Accept(TreeWalker& walker) override;
      // std::vector<Instruction> Emit() override;

      Ref<AstExpr> expr;
  };

  class VarExpr : public AstExpr {
    public:
      VarExpr(Token name) 
        : AstExpr(AstNodeType::VAR_EXPR) , name(name) {}
      virtual ~VarExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      void Accept(TreeWalker& walker) override;
      // std::vector<Instruction> Emit() override;

      Token name;
  };

  class AssignExpr : public AstExpr {
    public:
      AssignExpr(Token name, const Ref<AstExpr>& value) 
        : AstExpr(AstNodeType::ASSIGN_EXPR) , name(name), value(value) {}
      virtual ~AssignExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      void Accept(TreeWalker& walker) override;
      // std::vector<Instruction> Emit() override;

      Token name;
      Ref<AstExpr> value;
  };

  class ArrayExpr : public AstExpr {
    public:
      ArrayExpr(std::vector<Ref<AstExpr>>& elts) 
          : AstExpr(AstNodeType::ARRAY_EXPR) {
        elements.swap(elts);
      }
      virtual ~ArrayExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      void Accept(TreeWalker& walker) override;
      // std::vector<Instruction> Emit() override;

      std::vector<Ref<AstExpr>> elements;
  };

  class ArrayAccessExpr : public AstExpr {
    public:
      ArrayAccessExpr(const Ref<AstExpr>& array, const Ref<AstExpr>& index) 
        : AstExpr(AstNodeType::ARRAY_ACCESS_EXPR) , array(array), index(index) {}
      virtual ~ArrayAccessExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      void Accept(TreeWalker& walker) override;
      // std::vector<Instruction> Emit() override;

      Ref<AstExpr> array;
      Ref<AstExpr> index;
  };

  class ObjAccessExpr : public AstExpr {
    public:
      ObjAccessExpr(const Ref<AstExpr>& obj, Token member , const Ref<AstExpr>& index = nullptr  , const Ref<AstExpr>& assignment = nullptr) 
        : AstExpr(AstNodeType::OBJ_ACCESS_EXPR) , obj(obj), member(member) , index(index) , assignment(assignment) {}
      virtual ~ObjAccessExpr() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      void Accept(TreeWalker& walker) override;
      // std::vector<Instruction> Emit() override;

      Ref<AstExpr> obj;
      Token member;
      Ref<AstExpr> index;
      Ref<AstExpr> assignment;
  };

  class ExprStmt : public AstStmt {
    public:
      ExprStmt(const Ref<AstExpr>& expr) 
        : AstStmt(AstNodeType::EXPR_STMT) , expr(expr) {}
      virtual ~ExprStmt() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;
      // virtual std::vector<Instruction> Emit() override;

      Ref<AstExpr> expr = nullptr;
  };

  class VarDecl : public AstStmt {
    public:
      VarDecl(const Token& type , const Token& name , const Ref<AstExpr>& initializer = nullptr , bool is_const = false , bool is_global = false) 
        : AstStmt(AstNodeType::VAR_DECL_STMT) , name(name) ,  type(type) , initializer(initializer) , is_const(is_const) , is_global(is_global) {}
      virtual ~VarDecl() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;
      // virtual std::vector<Instruction> Emit() override;

      Token name;
      Token type;
      Ref<AstExpr> initializer = nullptr;
      bool is_const = false;
      bool is_global = false;
  };

  class ArrayDecl : public AstStmt {
    public:
      ArrayDecl(const Token& type , const Token& name , Token size , const Ref<AstExpr>& initializer , bool is_const = false) 
        : AstStmt(AstNodeType::ARRAY_DECL_STMT) , name(name) , type(type)  , 
          size(size) , initializer(initializer) , is_const(is_const) {}
      virtual ~ArrayDecl() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;
      // virtual std::vector<Instruction> Emit() override;

      Token name;
      Token type;
      Token size;
      Ref<AstExpr> initializer = nullptr;
      bool is_const;
  };

  class BlockStmt : public AstStmt {
    public:
      BlockStmt(std::vector<Ref<AstStmt>>& stmts) 
          : AstStmt(AstNodeType::BLOCK_STMT) {
        statements.swap(stmts);
      }
      virtual ~BlockStmt() override {
      }

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;
      // virtual std::vector<Instruction> Emit() override;

      std::vector<Ref<AstStmt>> statements;
  };

  class IfStmt : public AstStmt {
    public:
      IfStmt(const Ref<AstExpr>& condition , const Ref<AstStmt>& then_branch , const Ref<AstStmt>& else_branch = nullptr) 
        : AstStmt(AstNodeType::IF_STMT) , condition(condition) , then_branch(then_branch) , else_branch(else_branch) {}
      virtual ~IfStmt() override {
      }

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;
      // virtual std::vector<Instruction> Emit() override;

      Ref<AstExpr> condition;
      Ref<AstStmt> then_branch;
      Ref<AstStmt> else_branch;
  };

  class WhileStmt : public AstStmt {
    public:
      WhileStmt(const Ref<AstExpr>& condition , const Ref<AstStmt>& body) 
        : AstStmt(AstNodeType::WHILE_STMT) , condition(condition) , body(body) {}
      virtual ~WhileStmt() override {
      }

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;
      // virtual std::vector<Instruction> Emit() override;

      Ref<AstExpr> condition;
      Ref<AstStmt> body;
  };

  class ReturnStmt : public AstStmt {
    public:
      ReturnStmt(const Ref<AstStmt>& expr = nullptr) 
        : AstStmt(AstNodeType::RETURN_STMT) , stmt(expr) {}
      virtual ~ReturnStmt() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;
      // virtual std::vector<Instruction> Emit() override;

      Ref<AstStmt> stmt = nullptr;
  };

  struct FunctionParam {
    Token type;
    Token name;
  };

  class FunctionStmt : public AstStmt {
    public:
      FunctionStmt(const Token& name , const std::vector<FunctionParam>& params , const Token& type ,
                   const Ref<AstStmt>& body = nullptr) 
        : AstStmt(AstNodeType::FUNCTION_STMT) , name(name) , params(params) , body(body) , type(type) {}
      virtual ~FunctionStmt() override {}

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;
      // virtual std::vector<Instruction> Emit() override;

      Token name;
      std::vector<FunctionParam> params;
      Ref<AstStmt> body;
      Token type;
  };

  class StructStmt : public AstStmt {
    public:
      StructStmt(const Token& name , std::vector<Ref<AstStmt>>& fs) 
          : AstStmt(AstNodeType::STRUCT_STMT) , name(name) {
        fields.swap(fs);
      }
      virtual ~StructStmt() override {
      }

      virtual void Stream(std::ostream& stream , TreeWalker& walker) const override;
      virtual void Accept(TreeWalker& walker) override;
      // virtual std::vector<Instruction> Emit() override;

      Token name;
      std::vector<Ref<AstStmt>> fields;
  };

} // namespace other

#endif // !OTHER_ENGINE_AST_NODE_HPP
