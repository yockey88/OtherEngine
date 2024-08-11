/**
 * \file parsing/shader_glsl_transpiler.hpp
 **/
#ifndef OTHER_ENGINE_SHADER_GLSL_TRANSPILER_HPP
#define OTHER_ENGINE_SHADER_GLSL_TRANSPILER_HPP

#include "parsing/shader_parser.hpp"

#include "rendering/uniform.hpp"
#include "rendering/shader.hpp"

namespace other {  

  class ShaderGlslTranspiler : public TreeWalker {
    public: 
      ShaderGlslTranspiler(ShaderAst ast)
        : ast(ast) {}

      ShaderIr Transpile();

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
      virtual void Visit(VarDecl& stmt) override;
      virtual void Visit(ArrayDecl& stmt) override;
      virtual void Visit(BlockStmt& stmt) override;
      virtual void Visit(IfStmt& stmt) override;
      virtual void Visit(WhileStmt& stmt) override;
      virtual void Visit(ReturnStmt& stmt) override;
      virtual void Visit(FunctionStmt& stmt) override;
      virtual void Visit(StructStmt& stmt) override;

      virtual void Visit(VersionExpr& expr) override;
      virtual void Visit(ShaderAttribute& expr) override;
      virtual void Visit(LayoutDescriptor& expr) override;

      virtual void Visit(LayoutDecl& stmt) override;
      virtual void Visit(LayoutVarDecl& stmt) override;
      virtual void Visit(ShaderStorageStmt& stmt) override;
      virtual void Visit(InOutBlockStmt& stmt) override;
      virtual void Visit(UniformDecl& stmt) override;
      
      virtual void Visit(ShaderDecl& stmt) override;
      
    private:

    public:
      Opt<MeshLayout> mesh_layout = std::nullopt;

    private:
      ShaderIr result;

      ShaderAst ast;
      ShaderType context;

      struct Flags {
        std::stack<bool> is_uniform;
        std::stack<bool> push_uniforms;
      } flags;

      std::stack<MeshAttr> vertex_attr_stack;

      std::stack<Uniform> uniform_stack;
      std::stack<ShaderStorage> shader_storage_stack;

      std::stack<Token> token_stack;

      std::string  TranspileTo(std::vector<Ref<AstNode>>& nodes);
      void ProcessNodes();

      void SetMeshLayout(const std::string& value);

      uint32_t SizeFromString(const std::string& str);
      ValueType ValueTypeFromString(const std::string& str);

      template <typename... Args>
      ShaderException Error(ShaderError error , std::string_view message, Args&&... args) const {
        return ShaderException(fmtstr(message , std::forward<Args>(args)...) , error , 0 , 0);
      }

      friend class AstNode;
  };

} // namespace other 

#endif // !OTHER_ENGINE_SHADER_GLSL_TRANSPILER_HPP
