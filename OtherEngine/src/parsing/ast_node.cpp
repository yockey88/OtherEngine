/**
 * \file parsing/ast_node.cpp
 **/
#include "parsing/ast_node.hpp"

namespace other {

  void LiteralExpr::Stream(std::ostream& stream) const {
  }
      
  void UnaryExpr::Stream(std::ostream& stream) const {
    
  }

  void BinaryExpr::Stream(std::ostream& stream) const {}

  void CallExpr::Stream(std::ostream& stream) const {}

  void GroupingExpr::Stream(std::ostream& stream) const {}

  void VarExpr::Stream(std::ostream& stream) const {}

  void AssignExpr::Stream(std::ostream& stream) const {}

  void ArrayExpr::Stream(std::ostream& stream) const {}

  void ArrayAccessExpr::Stream(std::ostream& stream) const {}

  void ObjAccessExpr::Stream(std::ostream& stream) const {}

  void ExprStmt::Stream(std::ostream& stream) const {}

  void VarDecl::Stream(std::ostream& stream) const {}
  
  void ArrayDecl::Stream(std::ostream& stream) const {}

  void BlockStmt::Stream(std::ostream& stream) const {}

  void IfStmt::Stream(std::ostream& stream) const {}
  
  void WhileStmt::Stream(std::ostream& stream) const {}
  
  void ReturnStmt::Stream(std::ostream& stream) const {}

  void FunctionStmt::Stream(std::ostream& stream) const {}

  void StructStmt::Stream(std::ostream& stream) const {}

} // namespace other
