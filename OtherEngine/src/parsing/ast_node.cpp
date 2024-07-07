/**
 * \file parsing/ast_node.cpp
 **/
#include "parsing/ast_node.hpp"

namespace other {

  void LiteralExpr::Stream(std::ostream& stream) const {
    stream << value.value;
  }
  
  void LiteralExpr::Accept(TreeWalker& walker) {
  }
      
  void UnaryExpr::Stream(std::ostream& stream) const {
    
  }
  
  void UnaryExpr::Accept(TreeWalker& walker) {
  }

  void BinaryExpr::Stream(std::ostream& stream) const {
    left->Stream(stream);
    stream << " " << op.value << " ";
    right->Stream(stream);
  }
  
  void BinaryExpr::Accept(TreeWalker& walker) {
  }

  void CallExpr::Stream(std::ostream& stream) const {}
  
  void CallExpr::Accept(TreeWalker& walker) {
  }

  void GroupingExpr::Stream(std::ostream& stream) const {}
  
  void GroupingExpr::Accept(TreeWalker& walker) {
  }

  void VarExpr::Stream(std::ostream& stream) const {
    stream << name.value;
  }
  
  void VarExpr::Accept(TreeWalker& walker) {
  }

  void AssignExpr::Stream(std::ostream& stream) const {
    stream << name.value << " = ";
    value->Stream(stream);
  }
  
  void AssignExpr::Accept(TreeWalker& walker) {
  }

  void ArrayExpr::Stream(std::ostream& stream) const {}
  
  void ArrayExpr::Accept(TreeWalker& walker) {
  }

  void ArrayAccessExpr::Stream(std::ostream& stream) const {}
  
  void ArrayAccessExpr::Accept(TreeWalker& walker) {
  }

  void ObjAccessExpr::Stream(std::ostream& stream) const {}
  
  void ObjAccessExpr::Accept(TreeWalker& walker) {
  }

  void ExprStmt::Stream(std::ostream& stream) const {}
  
  void ExprStmt::Accept(TreeWalker& walker) {
  }

  void VarDecl::Stream(std::ostream& stream) const {
    stream << type.value << " " << type.value << " ";
    if (initializer != nullptr) {
      initializer->Stream(stream);
    }
  }
  
  void VarDecl::Accept(TreeWalker& walker) {
  }
  
  void ArrayDecl::Stream(std::ostream& stream) const {}
  
  void ArrayDecl::Accept(TreeWalker& walker) {
  }

  void BlockStmt::Stream(std::ostream& stream) const {}

  void BlockStmt::Accept(TreeWalker& stream) {}

  void IfStmt::Stream(std::ostream& stream) const {}
  
  void IfStmt::Accept(TreeWalker& stream) {}
  
  void WhileStmt::Stream(std::ostream& stream) const {}
  
  void WhileStmt::Accept(TreeWalker& stream) {}
  
  void ReturnStmt::Stream(std::ostream& stream) const {}
  
  void ReturnStmt::Accept(TreeWalker& stream) {}

  void FunctionStmt::Stream(std::ostream& stream) const {}
  
  void FunctionStmt::Accept(TreeWalker& stream) {}

  void StructStmt::Stream(std::ostream& stream) const {}
  
  void StructStmt::Accept(TreeWalker& stream) {}

} // namespace other
