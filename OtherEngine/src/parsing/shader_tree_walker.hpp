/**
 * \file parsing/shader_tree_walker.hpp
 **/
#ifndef OTHER_ENGINE_SHADER_TREE_WALKER_HPP
#define OTHER_ENGINE_SHADER_TREE_WALKER_HPP

#include "parsing/shader_ast_node.hpp"
#include "parsing/tree_walker.hpp"

namespace other {

  class VersionExpr;
  class LayoutDescriptor;

  class LayoutDecl;
  class LayoutStmt;
  class ShaderStorageStmt;

  class ShaderTreeWalker : public TreeWalker {
    public:
      // Expressions
      virtual void Visit(LiteralExpr& expr) override {}
      virtual void Visit(UnaryExpr& expr) override {}
      virtual void Visit(BinaryExpr& expr) override {}
      virtual void Visit(CallExpr& expr) override {}
      virtual void Visit(GroupingExpr& expr) override {}
      virtual void Visit(VarExpr& expr) override {}
      virtual void Visit(AssignExpr& expr) override {}
      virtual void Visit(ArrayExpr& expr) override {}
      virtual void Visit(ArrayAccessExpr& expr) override {}
      virtual void Visit(ObjAccessExpr& expr) override {}

      // Statements
      virtual void Visit(ExprStmt& stmt) override {}
      virtual void Visit(VarDeclStmt& stmt) override {}
      virtual void Visit(ArrayDeclStmt& stmt) override {}
      virtual void Visit(BlockStmt& stmt) override {}
      virtual void Visit(PrintStmt& stmt) override {}
      virtual void Visit(IfStmt& stmt) override {}
      virtual void Visit(WhileStmt& stmt) override {}
      virtual void Visit(ReturnStmt& stmt) override {}
      virtual void Visit(FunctionStmt& stmt) override {}
      virtual void Visit(StructStmt& stmt) override {}

      virtual void Visit(VersionExpr& expr) {}
      virtual void Visit(LayoutDescriptor& expr) {}

      virtual void Visit(LayoutDecl& stmt) {}
      virtual void Visit(LayoutStmt& stmt) {}
      virtual void Visit(ShaderStorageStmt& stmt) {}
  };

} // namespace other

#endif // !OTHER_ENGINE_SHADER_TREE_WALKER_HPP
