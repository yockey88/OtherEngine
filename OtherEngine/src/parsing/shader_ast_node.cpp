/**
 * \file parsing/shader_ast_node.cpp
 **/
#include "parsing/shader_ast_node.hpp"

#include "core/logger.hpp"
#include "parsing/parsing_defines.hpp"

namespace other {

  void VersionExpr::Stream(std::ostream& stream) const {
    stream << "#version " << version_number.value << " " << version_type.value << "\n\n";
  }
  
  void VersionExpr::Accept(TreeWalker& walker) {

  }

  void LayoutDescriptor::Stream(std::ostream& stream) const {
    stream << type.value << " = ";
    expr->Stream(stream);
  }
  
  void LayoutDescriptor::Accept(TreeWalker& walker) {
  }
  
  void LayoutDecl::Stream(std::ostream& stream) const {
    stream << "layout (";

    /// this means there is a max size/layout type descibed (std140 , std430 , ....)
    if (layout_rules.type != INVALID_TOKEN) {
      stream << layout_rules.value << " , ";
    } 

    OE_DEBUG("Streaming layout decl with {} descriptors" , descriptors.size());

    /// we have to stream with iterators because glsl does not allow hanging commas
    for (auto itr = descriptors.begin(); itr != descriptors.end();) {
      (*itr)->Stream(stream);

      ++itr;
      if (itr != descriptors.end()) {
        stream << " , ";
      }
    }
    stream << ") ";

    data->Stream(stream);
    stream << "\n\n";
  }
  
  void LayoutDecl::Accept(TreeWalker& walker) {
  }
      
  void ShaderStorageStmt::Stream(std::ostream& stream) const {
  }
  
  void ShaderStorageStmt::Accept(TreeWalker& walker) {
  }

} // namespace other
