/**
 * \file parsing/shader_glsl_transpiler.cpp
 **/
#include "parsing/shader_glsl_transpiler.hpp"

namespace other {
      
  std::string ShaderGlslTranspiler::Transpile() {
    std::stringstream ss;

    for (auto& n : ast.nodes) {
      n->Stream(ss);
    }

    return ss.str();
  }

  void ShaderGlslTranspiler::Visit(LiteralExpr& expr) {
  }

  void ShaderGlslTranspiler::Visit(UnaryExpr& expr) {
  }

  void ShaderGlslTranspiler::Visit(BinaryExpr& expr) {}

  void ShaderGlslTranspiler::Visit(CallExpr& expr) {}

  void ShaderGlslTranspiler::Visit(GroupingExpr& expr) {}

  void ShaderGlslTranspiler::Visit(VarExpr& expr) {}

  void ShaderGlslTranspiler::Visit(AssignExpr& expr) {}

  void ShaderGlslTranspiler::Visit(ArrayExpr& expr) {}

  void ShaderGlslTranspiler::Visit(ArrayAccessExpr& expr) {}

  void ShaderGlslTranspiler::Visit(ObjAccessExpr& expr) {}
  
  void ShaderGlslTranspiler::Visit(ExprStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(VarDeclStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(ArrayDeclStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(BlockStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(PrintStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(IfStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(WhileStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(ReturnStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(FunctionStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(StructStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(VersionExpr& expr) {}

  void ShaderGlslTranspiler::Visit(LayoutDescriptor& expr) {}

  void ShaderGlslTranspiler::Visit(LayoutDecl& stmt) {
  }

  void ShaderGlslTranspiler::Visit(LayoutStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(ShaderStorageStmt& stmt) {}

} // namespace other
