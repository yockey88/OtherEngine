/**
 * \file parsing/ast_node.cpp
 **/
#include "parsing/ast_node.hpp"

#include "core/logger.hpp"

namespace other {

  void LiteralExpr::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << value.value;
  }
  
  void LiteralExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }
      
  void UnaryExpr::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << op.value;
    right->Stream(stream , walker);
  }
  
  void UnaryExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void BinaryExpr::Stream(std::ostream& stream , TreeWalker& walker) const {
    left->Stream(stream , walker);
    stream << " " << op.value << " ";
    right->Stream(stream , walker);
  }
  
  void BinaryExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void CallExpr::Stream(std::ostream& stream , TreeWalker& walker) const {
    callee->Stream(stream , walker);
    stream << "(";
    for (auto itr = args.begin(); itr != args.end();) {
      (*itr)->Stream(stream , walker);
      ++itr;
      
      if (itr != args.end()) {
        stream << " , ";
      }
    }
    stream << ")";
  }
  
  void CallExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void GroupingExpr::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << "(";
    expr->Stream(stream , walker); 
    stream << ")";
  }
  
  void GroupingExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void VarExpr::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << name.value;
  }
  
  void VarExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void AssignExpr::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << name.value << " = ";
    value->Stream(stream , walker);
  }
  
  void AssignExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void ArrayExpr::Stream(std::ostream& stream , TreeWalker& walker) const {}
  
  void ArrayExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void ArrayAccessExpr::Stream(std::ostream& stream , TreeWalker& walker) const {
    array->Stream(stream , walker);
    stream << "[";
    index->Stream(stream , walker);
    stream << "]";
  }
  
  void ArrayAccessExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void ObjAccessExpr::Stream(std::ostream& stream , TreeWalker& walker) const {
    obj->Stream(stream , walker);
    stream << "." << member.value;
    
    if (index != nullptr) {
      stream << "[";
      index->Stream(stream , walker);
      stream << "]";
    }

    if (assignment != nullptr) {
      stream << " = ";
      assignment->Stream(stream , walker);
    }
  }
  
  void ObjAccessExpr::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void ExprStmt::Stream(std::ostream& stream , TreeWalker& walker) const {
    expr->Stream(stream , walker);
  }
  
  void ExprStmt::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void VarDecl::Stream(std::ostream& stream , TreeWalker& walker) const {
    if (is_const) {
      stream << "const ";
    }

    stream << type.value << " " << name.value;
    if (initializer != nullptr) {
      stream << " = ";
      initializer->Stream(stream , walker);
    }

    if (is_global) {
      stream << ";\n\n";
    }
  }
  
  void VarDecl::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }
  
  void ArrayDecl::Stream(std::ostream& stream , TreeWalker& walker) const {
    if (is_const) {
      stream << "const ";
    }

    stream << type.value << " " << name.value << "[";
    if (size.value.length() > 0) {
      stream << size.value;
    }
    stream << "]";

    if (initializer != nullptr) {
      stream << " = ";
      initializer->Stream(stream , walker);
    }
  }
  
  void ArrayDecl::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void BlockStmt::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << "{\n";
    for (auto& s : statements) {
      stream << "  ";
      s->Stream(stream , walker);
      stream << ";\n";
    }
    stream << "}";
  }

  void BlockStmt::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void IfStmt::Stream(std::ostream& stream , TreeWalker& walker) const {}
  
  void IfStmt::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }
  
  void WhileStmt::Stream(std::ostream& stream , TreeWalker& walker) const {}
  
  void WhileStmt::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }
  
  void ReturnStmt::Stream(std::ostream& stream , TreeWalker& walker) const {}
  
  void ReturnStmt::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void FunctionStmt::Stream(std::ostream& stream , TreeWalker& walker) const {
    stream << type.value << " " << name.value << "(";
    for (auto itr = params.begin(); itr != params.end();) {
      stream << itr->type.value  << " " << itr->name.value;
      ++itr;

      if (itr != params.end()) {
        stream << " , ";
      }
    }
    stream << ") ";
    body->Stream(stream , walker);
    stream << "\n\n";
  }
  
  void FunctionStmt::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

  void StructStmt::Stream(std::ostream& stream , TreeWalker& walker) const {}
  
  void StructStmt::Accept(TreeWalker& walker) {
    walker.Visit(*this);
  }

} // namespace other
