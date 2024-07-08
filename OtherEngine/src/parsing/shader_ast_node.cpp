/**
 * \file parsing/shader_ast_node.cpp
 **/
#include "parsing/shader_ast_node.hpp"

#include "parsing/ast_node.hpp"
#include "parsing/parsing_defines.hpp"
#include "parsing/shader_glsl_transpiler.hpp"

namespace other {

  void VersionExpr::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << "#version " << version_number.value << " " << version_type.value << "\n\n";
  }
  
  void VersionExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }
  
  void ShaderAttribute::Stream(std::ostream& stream , TreeWalker& walker) const {
  }
  
  void ShaderAttribute::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void LayoutDescriptor::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << type.value << " = ";
    expr->Stream(stream , walker);  
  }
  
  void LayoutDescriptor::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }
  
  void LayoutDecl::Stream(std::ostream& stream , TreeWalker& walker) const {
    ShaderGlslTranspiler* tp = static_cast<ShaderGlslTranspiler*>(&walker);
    if (tp->mesh_layout.has_value() && tp->mesh_layout->override && descriptors.size() == 1) {
      Ref<LayoutDescriptor> desc = descriptors[0].As<LayoutDescriptor>();
      
      if (desc->type.type == LOCATION_KW) {
        OE_DEBUG("LayoutDecl::Stream {} = {} ({})" , descriptors.size() , kTokenStrings[desc->type.type] , desc->type.type); 
        return;
      }
    }

    stream << "layout (";

    /// this means there is a max size/layout type descibed (std140 , std430 , ....)
    if (layout_rules.type != INVALID_TOKEN) {
      stream << layout_rules.value;
      if (descriptors.size() > 0) {
        stream << " , ";
      }
    } 


    /// we have to stream with iterators because glsl does not allow hanging commas
    for (auto itr = descriptors.begin(); itr != descriptors.end();) {
      (*itr)->Stream(stream , walker);

      ++itr;
      if (itr != descriptors.end()) {
        stream << " , ";
      }
    }
    stream << ") ";

    data->Stream(stream , walker);
  }
  
  void LayoutDecl::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void LayoutVarDecl::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << in_out.value;
    if (type.has_value() && name.has_value()) {
      stream << " " << type.value().value
             << " " << name.value().value  << ";\n";
    } else {
      stream << ";\n";
    }
  }

  void LayoutVarDecl::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }
      
  void ShaderStorageStmt::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << type.value << " " << name.value << " ";
    body->Stream(stream , walker);
    stream << ";\n\n";
  }
  
  void ShaderStorageStmt::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }
  
  void InOutBlockStmt::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << in_out.value << " " << tag.value << " ";
    body->Stream(stream , walker);
    stream << " " << name.value;
    if (is_array) {
      stream << "[]";
    }
    stream << ";\n\n";
  }
  
  void InOutBlockStmt::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }
      
  void UniformDecl::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << "uniform ";
    var_decl->Stream(stream , walker);
  }

  void UniformDecl::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }
      
  void ShaderDecl::Stream(std::ostream& stream , TreeWalker& walker) const {
    for (auto& s : statements) {
      stream << "  ";
      s->Stream(stream , walker);
    }
  }

  void ShaderDecl::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

} // namespace other
