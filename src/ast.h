#ifndef AST_H
#define AST_H

#include "treeprint.h"

// AST node tags
enum ASTKind {
  AST_ADD = 2000,
  AST_SUB,
  AST_MULTIPLY,
  AST_DIVIDE,
  AST_VARREF,
  AST_INT_LITERAL,
  AST_UNIT,
  AST_STATEMENT,
  // DONE: add members for other AST node kinds
  AST_VARDEF,         // var
  AST_ASSIGN,         // =
  AST_LOGICAL_OR,     // ||
  AST_LOGICAL_AND,    // &&
  AST_LESS,           // <
  AST_LESSEQUAL,      // <=
  AST_GREATER,        // >
  AST_GREATEREQUAL,   // >=
  AST_ISEQUAL,        // ==
  AST_ISNOTEQUAL,     // !=
  // MS2 DONE: add members for other AST node kinds
  AST_IF,             // if
  AST_WHILE,          // while
  AST_STATEMENT_LIST,
  AST_FNCALL,
  AST_ARGLIST,
  AST_FUNCTION,
  AST_PARAM_LIST,
  AST_STRING_LITERAL,
};

class ASTTreePrint : public TreePrint {
public:
  ASTTreePrint();
  virtual ~ASTTreePrint();

  virtual std::string node_tag_to_string(int tag) const;
};

#endif // AST_H
