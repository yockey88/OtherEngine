/**
 * \file parsing/tree_walker.hpp
 **/
#ifndef OTHER_ENGINE_TREE_WALKER_HPP
#define OTHER_ENGINE_TREE_WALKER_HPP

namespace other {

  class AstNode;
  class AstExpr;
  class AstStmt;

  class LiteralExpr;
  class UnaryExpr;
  class BinaryExpr;
  class CallExpr;
  class GroupingExpr;
  class VarExpr;
  class AssignExpr;
  class ArrayExpr;
  class ArrayAccessExpr;
  class ObjAccessExpr;

  class ExprStmt;
  class VarDecl;
  class ArrayDecl;
  class BlockStmt;
  class PrintStmt;
  class IfStmt;
  class WhileStmt;
  class ReturnStmt;
  class FunctionStmt;
  class StructStmt;

  /// for shaders
  class VersionExpr;
  class ShaderAttribute;
  class LayoutDescriptor;

  class LayoutDecl;
  class LayoutVarDecl;
  class ShaderStorageStmt;
  class InOutBlockStmt;
  class UniformDecl;

  class ShaderDecl;

  class TreeWalker {
    public:
      virtual ~TreeWalker() = default;

      // Expressions
      virtual void Visit(LiteralExpr& expr) = 0;
      virtual void Visit(UnaryExpr& expr) = 0;
      virtual void Visit(BinaryExpr& expr) = 0;
      virtual void Visit(CallExpr& expr) = 0;
      virtual void Visit(GroupingExpr& expr) = 0;
      virtual void Visit(VarExpr& expr) = 0;
      virtual void Visit(AssignExpr& expr) = 0;
      virtual void Visit(ArrayExpr& expr) = 0;
      virtual void Visit(ArrayAccessExpr& expr) = 0;
      virtual void Visit(ObjAccessExpr& expr) = 0;

      // Statements
      virtual void Visit(ExprStmt& stmt) = 0;
      virtual void Visit(VarDecl& stmt) = 0;
      virtual void Visit(ArrayDecl& stmt) = 0;
      virtual void Visit(BlockStmt& stmt) = 0;
      virtual void Visit(IfStmt& stmt) = 0;
      virtual void Visit(WhileStmt& stmt) = 0;
      virtual void Visit(ReturnStmt& stmt) = 0;
      virtual void Visit(FunctionStmt& stmt) = 0;
      virtual void Visit(StructStmt& stmt) = 0;
      
      virtual void Visit(VersionExpr& expr) {}
      virtual void Visit(ShaderAttribute& expr) {}
      virtual void Visit(LayoutDescriptor& expr) {}

      virtual void Visit(LayoutDecl& stmt) {}
      virtual void Visit(LayoutVarDecl& stmt) {}
      virtual void Visit(ShaderStorageStmt& stmt) {}
      virtual void Visit(InOutBlockStmt& stmt) {}
      virtual void Visit(UniformDecl& stmt) {}

      virtual void Visit(ShaderDecl& stmt) {}
  };

} // namespace other

#endif // !OTHER_ENGINE_TREE_WALKER_HPP
