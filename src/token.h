#ifndef TOKEN_H
#define TOKEN_H

// This header file defines the tags used for tokens (i.e., terminal
// symbols in the grammar.)

enum TokenKind {
  TOK_IDENTIFIER,
  TOK_INTEGER_LITERAL,
  TOK_PLUS,
  TOK_MINUS,
  TOK_TIMES,
  TOK_DIVIDE,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_SEMICOLON,
  // DONE: add members for additional kinds of tokens
  TOK_ASSIGN,       // =
  TOK_LESS,         // <
  TOK_GREATER,      // >
  TOK_IS_EQUAL,     // ==
  TOK_AND,          // &&
  TOK_LESS_EQUAL,   // <=
  TOK_GREATER_EQUAL,// >=
  TOK_OR,           // ||
  TOK_NOT_EQUAL,    // !=
  TOK_VAR,          // var
  ERROR,
  // A2 Done: add tags for the remaining kinds of tokens
  TOK_IF,           // if
  TOK_ELSE,         // else
  TOK_WHILE,        // while
  TOK_LBRACE,       // {
  TOK_RBRACE,       // }
  TOK_COMMA,        // ,
  TOK_FUNCTION,     // function
  TOK_STRING,       // "xxx"
};

#endif // TOKEN_H
