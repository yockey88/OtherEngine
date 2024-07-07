/**
 * \file parsing/shader_glsl_transpiler.hpp
 **/
#ifndef OTHER_ENGINE_SHADER_GLSL_TRANSPILER_HPP
#define OTHER_ENGINE_SHADER_GLSL_TRANSPILER_HPP

#include "parsing/shader_tree_walker.hpp"
#include "parsing/shader_parser.hpp"

namespace other {

  class ShaderGlslTranspiler : public ShaderTreeWalker {
    public: 
      ShaderGlslTranspiler(ShaderAst ast)
        : ast(ast) {}

      std::string Transpile();

      // Expressions
      virtual void Visit(LiteralExpr& expr) override;
      virtual void Visit(UnaryExpr& expr) override;
      virtual void Visit(BinaryExpr& expr) override;
      virtual void Visit(CallExpr& expr) override;
      virtual void Visit(GroupingExpr& expr) override;
      virtual void Visit(VarExpr& expr) override;
      virtual void Visit(AssignExpr& expr) override;
      virtual void Visit(ArrayExpr& expr) override;
      virtual void Visit(ArrayAccessExpr& expr) override;
      virtual void Visit(ObjAccessExpr& expr) override;

      // Statements
      virtual void Visit(ExprStmt& stmt) override;
      virtual void Visit(VarDeclStmt& stmt) override;
      virtual void Visit(ArrayDeclStmt& stmt) override;
      virtual void Visit(BlockStmt& stmt) override;
      virtual void Visit(PrintStmt& stmt) override;
      virtual void Visit(IfStmt& stmt) override;
      virtual void Visit(WhileStmt& stmt) override;
      virtual void Visit(ReturnStmt& stmt) override;
      virtual void Visit(FunctionStmt& stmt) override;
      virtual void Visit(StructStmt& stmt) override;

      virtual void Visit(VersionExpr& expr) override;
      virtual void Visit(LayoutDescriptor& expr) override;

      virtual void Visit(LayoutDecl& stmt) override;
      virtual void Visit(LayoutStmt& stmt) override;
      virtual void Visit(ShaderStorageStmt& stmt) override;

    private:
      ShaderAst ast;
  };

} // namespace other 

#endif // !OTHER_ENGINE_SHADER_GLSL_TRANSPILER_HPP
