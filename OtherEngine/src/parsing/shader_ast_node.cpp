/**
 * \file parsing/shader_ast_node.cpp
 **/
#include "parsing/shader_ast_node.hpp"

namespace other {

  void VersionExpr::Stream(std::ostream& stream) const {
    stream << "#version " << version_number.value << " " << version_type.value;
  }

  void LayoutDescriptor::Stream(std::ostream& stream) const {
    expr->Stream(stream);
  }
  
  void LayoutDecl::Stream(std::ostream& stream) const {
    stream << "layout (" << layout_rules.value << " , ";
    for (auto itr = descriptors.begin(); itr != descriptors.end();) {
      (*itr)->Stream(stream);
      
      ++itr;
      if (itr != descriptors.end()) {
        stream << " , ";
      }
    }
    stream << ")";

    data->Stream(stream);
  }
  
  void LayoutStmt::Stream(std::ostream& stream) const {
  }
      
  void ShaderStorageStmt::Stream(std::ostream& stream) const {
  }

} // namespace other
