#include "exceptions.h"
#include "ast.h"

ASTTreePrint::ASTTreePrint() {
}

ASTTreePrint::~ASTTreePrint() {
}

std::string ASTTreePrint::node_tag_to_string(int tag) const {
  switch (tag) {
  case AST_ADD:
    return "ADD";
  case AST_SUB:
    return "SUB";
  case AST_MULTIPLY:
    return "MULTIPLY";
  case AST_DIVIDE:
    return "DIVIDE";
  case AST_VARREF:
    return "VARREF";
  case AST_INT_LITERAL:
    return "INT_LITERAL";
  case AST_UNIT:
    return "UNIT";
  case AST_STATEMENT:
    return "STATEMENT";
  // DONE: add cases for other AST node kinds
  case AST_VARDEF:
    return "VARDEF";
  case AST_ASSIGN:
    return "ASSIGN";
  case AST_LOGICAL_OR:
    return "LOGICAL_OR";
  case AST_LOGICAL_AND:
    return "LOGICAL_AND";
  case AST_LESS:
    return "LESS";
  case AST_LESSEQUAL:
    return "LESSEQUAL";
  case AST_GREATER:
    return "GREATER";
  case AST_GREATEREQUAL:
    return "GREATEREQUAL";
  case AST_ISEQUAL:
    return "ISEQUAL";
  case AST_ISNOTEQUAL:
    return "ISNOTEQUAL";
  // MS2 DONE: add cases for other AST node kinds
  case AST_IF:
    return "IF";
  case AST_WHILE:
    return "WHILE";
  case AST_STATEMENT_LIST:
    return "STATEMENT_LIST";
  case AST_FNCALL:
    return "FNCALL";
  case AST_ARGLIST:
    return "ARGLIST";
  case AST_FUNCTION:
    return "FUNCTION";
  case AST_PARAM_LIST:
    return "PARAMETER_LIST";
  case AST_STRING_LITERAL:
    return "STRING_LITERAL";
  default:
    RuntimeError::raise("Unknown AST node type %d\n", tag);
  }
}
