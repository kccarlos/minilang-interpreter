#include <cassert>
#include <map>
#include <string>
#include <memory>
#include "token.h"
#include "ast.h"
#include "exceptions.h"
#include "parser2.h"

////////////////////////////////////////////////////////////////////////
// Parser2 implementation
// This version of the parser builds an AST directly,
// rather than first building a parse tree.
////////////////////////////////////////////////////////////////////////

// This is the grammar (Unit is the start symbol):
//
// Unit -> Stmt
// Unit -> Stmt Unit
// Stmt -> E ;
// E -> T E'
// E' -> + T E'
// E' -> - T E'
// E' -> epsilon
// T -> F T'
// T' -> * F T'
// T' -> / F T'
// T' -> epsilon
// F -> number
// F -> ident
// F -> ( E )

// New grammar:
// Unit -> Stmt
// Unit -> Stmt Unit
// Stmt -> var ident ;   (New)
// Stmt -> A ;           (Changed)
// A    -> ident = A     (NEW, start from here)
// A    -> L
// L    -> R || R
// L    -> R && R
// L    -> R
// R    -> E < E
// R    -> E <= E
// R    -> E > E
// R    -> E >= E
// R    -> E == E
// R    -> E != E
// R    -> E             (NEW, ends here)
// E    -> T E'
// E'   -> + T E'
// E'   -> - T E'
// E'   -> epsilon
// T    -> F T'
// T'   -> * F T'
// T'   -> / F T'
// T'   -> epsilon
// F    -> number
// F    -> ident
// F    -> ( A )         (Changed)

// A2 MS1 :
// Unit -> TStmt                                   (Changed)
// Unit -> TStmt Unit                              (Changed)
// TStmt -> Stmt                                   (New)
// Stmt -> if ( A ) { SList }                      (New)
// Stmt -> if ( A ) { SList } else { SList }       (New)
// Stmt -> while ( A ) { SList }                   (New)
// SList -> Stmt                                   (New)
// SList -> Stmt SList                             (New)
// F -> ident ( OptArgList )                       (New) function call
// OptArgList -> ArgList                           (New) call function with arguments
// OptArgList -> epsilon                           (New) call function without arguments
// ArgList -> L                                    (New) single argument
// ArgList -> L , ArgList                          (New) multiple arguments

// A2 MS2 :
// TStmt -> Func                                   (New)
// Func -> function ident ( OptPList ) { SList }   (New)
// OptPList -> PList                               (New)
// OptPList -> epsilon                             (New)
// PList -> ident                                  (New)
// PList -> ident , PList                          (New)
// F    -> string_literal                          (New)


Parser2::Parser2(Lexer *lexer_to_adopt)
  : m_lexer(lexer_to_adopt)
  , m_next(nullptr) {
}

Parser2::~Parser2() {
  delete m_lexer;
}

Node *Parser2::parse() {
  return parse_Unit();
}
Node *Parser2::parse_Unit() {
  // note that this function produces a "flattened" representation
  // of the unit
  // Unit → Stmt
  // Unit → Stmt Unit
  std::unique_ptr<Node> unit(new Node(AST_UNIT));
  for (;;) {
    unit->append_kid(parse_TStmt());
    if (m_lexer->peek() == nullptr)
      break;
  }
  return unit.release();
}

Node *Parser2::parse_TStmt() {
  Node *next_tok = m_lexer->peek();
  if (next_tok->get_tag() == TOK_FUNCTION) {
    // TStmt -> Func
    return parse_Func();
  } else {
    // TStmt -> Stmt
    return parse_Stmt();
  }
}

Node *Parser2::parse_Func() {
  // Func -> function ident ( OptPList ) { SList }
  std::unique_ptr<Node> func(new Node(AST_FUNCTION));
  func->set_loc(m_lexer->peek()->get_loc());
  expect_and_discard(TOK_FUNCTION);

  // AST node for the function name
  std::unique_ptr<Node> ident(expect(TOK_IDENTIFIER));
  std::unique_ptr<Node> varRef(new Node(AST_VARREF));
  varRef->set_str(ident->get_str());
  varRef->set_loc(ident->get_loc());
  func->append_kid(varRef.release());

  // AST nodes for the parameters
  expect_and_discard(TOK_LPAREN);
  func->append_kid(parse_OptPList());
  expect_and_discard(TOK_RPAREN);

  // AST nodes for the block
  std::unique_ptr<Node> block(new Node(AST_STATEMENT_LIST));
  expect_and_discard(TOK_LBRACE);
  for (;;) {
    block->append_kid(parse_Stmt());
    if (m_lexer->peek() == nullptr || m_lexer->peek()->get_tag() == TOK_RBRACE)
      break;
  }
  expect_and_discard(TOK_RBRACE);
  func->append_kid(block.release());

  return func.release();
}

Node *Parser2::parse_OptPList() {
  // OptPList -> PList
  // OptPList -> epsilon
  std::unique_ptr<Node> ast(new Node(AST_PARAM_LIST));

  Node *next_tok = m_lexer->peek(1);
  if (next_tok != nullptr && next_tok->get_tag() != TOK_RPAREN) {
    // OptPList -> PList
    return parse_PList(ast.release());
  }
  return ast.release();
}

Node *Parser2::parse_PList(Node *ast_){
  // PList -> ident
  // PList -> ident , PList
  for (;;) {
    Node *ident = expect(TOK_IDENTIFIER);
    std::unique_ptr<Node> varRef(new Node(AST_VARREF));
    varRef->set_str(ident->get_str());
    varRef->set_loc(ident->get_loc());
    ast_->append_kid(varRef.release());
    Node* next_tok = m_lexer->peek(1);
    if (next_tok == nullptr || next_tok->get_tag() != TOK_COMMA)
      break;
    else {
      expect_and_discard(TOK_COMMA);
    }
  }
  return ast_;
}

Node *Parser2::parse_Stmt() {
  // Stmt -> var ident ;
  // Stmt -> A ;
  std::unique_ptr<Node> s(new Node(AST_STATEMENT));

  Node *next_tok = m_lexer->peek();
  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }

  if (next_tok->get_tag() == TOK_VAR) {
    // Stmt -> var ident ;

    // Create AST nodes for the variable definition key
    std::unique_ptr<Node> varDef(new Node(AST_VARDEF));
    varDef->set_loc(next_tok->get_loc());
    expect_and_discard(TOK_VAR);

    // Check if the next token is an identifier
    std::unique_ptr<Node> ident(expect(TOK_IDENTIFIER));

    // Create AST nodes for the identifier
    std::unique_ptr<Node> varRef(new Node(AST_VARREF));
    varRef->set_str(ident->get_str());
    varRef->set_loc(ident->get_loc());

    // Add as children to the variable definition node
    varDef->append_kid(varRef.release());
    s->append_kid(varDef.release());

    expect_and_discard(TOK_SEMICOLON);

  } else if (next_tok->get_tag() == TOK_IF) {
    // Stmt -> if ( A ) { SList }

    // Create AST nodes for if key
    std::unique_ptr<Node> ifNode(new Node(AST_IF));
    ifNode->set_loc(next_tok->get_loc());
    expect_and_discard(TOK_IF);
    expect_and_discard(TOK_LPAREN);

    // AST node for the condition
    Node* ast_A = parse_A();
    ifNode->append_kid(ast_A);
    expect_and_discard(TOK_RPAREN);

    // AST nodes for the block
    std::unique_ptr<Node> block(new Node(AST_STATEMENT_LIST));
    expect_and_discard(TOK_LBRACE);
    for (;;) {
      block->append_kid(parse_Stmt());
      if (m_lexer->peek() == nullptr || m_lexer->peek()->get_tag() == TOK_RBRACE)
        break;
    }
    expect_and_discard(TOK_RBRACE);
    ifNode->append_kid(block.release());

    // Check if there is an else block
    if (m_lexer->peek() != nullptr && m_lexer->peek()->get_tag() == TOK_ELSE) {
      // Stmt -> if ( A ) { SList } else { SList }

      // AST nodes for the else block
      std::unique_ptr<Node> elseBlock(new Node(AST_STATEMENT_LIST));
      expect_and_discard(TOK_ELSE);
      expect_and_discard(TOK_LBRACE);
      for (;;) {
        elseBlock->append_kid(parse_Stmt());
        if (m_lexer->peek() == nullptr || m_lexer->peek()->get_tag() == TOK_RBRACE)
          break;
      }
      expect_and_discard(TOK_RBRACE);
      ifNode->append_kid(elseBlock.release());
    }

    s->append_kid(ifNode.release());

  } else if (next_tok->get_tag() == TOK_WHILE) {
    // Stmt -> while ( A ) { SList };

    // Create AST nodes for while key
    std::unique_ptr<Node> whileNode(new Node(AST_WHILE));
    whileNode->set_loc(next_tok->get_loc());
    expect_and_discard(TOK_WHILE);

    // AST node for the condition
    expect_and_discard(TOK_LPAREN);
    Node* ast_A = parse_A();
    whileNode->append_kid(ast_A);
    expect_and_discard(TOK_RPAREN);

    // AST nodes for the block
    std::unique_ptr<Node> block(new Node(AST_STATEMENT_LIST));
    expect_and_discard(TOK_LBRACE);
    for (;;) {
      block->append_kid(parse_Stmt());
      if (m_lexer->peek() == nullptr || m_lexer->peek()->get_tag() == TOK_RBRACE)
        break;
    }
    expect_and_discard(TOK_RBRACE);
    whileNode->append_kid(block.release());
    s->append_kid(whileNode.release());

  } else {
    // Stmt -> ^ A ;
    s->append_kid(parse_A());
    expect_and_discard(TOK_SEMICOLON);
  }
  return s.release();
}

Node *Parser2::parse_A() {
  // A    -> ident = A
  // A    -> L
  Node *next_tok = m_lexer->peek(1);
  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for A");
  }
  // Two tokens of look ahead
  Node *next_next_tok = m_lexer->peek(2);
  if (next_next_tok != nullptr && next_next_tok->get_tag() == TOK_ASSIGN && next_tok->get_tag() == TOK_IDENTIFIER) {
    // A    → ident = A

    // Create AST nodes for the identifier
    std::unique_ptr<Node> varRef(new Node(AST_VARREF));
    varRef->set_str(next_tok->get_str());
    varRef->set_loc(next_tok->get_loc());
    expect_and_discard(TOK_IDENTIFIER);

    // Create AST nodes for the assign sign
    std::unique_ptr<Node> assign(new Node(AST_ASSIGN));
    assign->set_loc(next_next_tok->get_loc());
    expect_and_discard(TOK_ASSIGN);

    // Add as children to the assign node
    Node* ast_A = parse_A();
    assign->append_kid(varRef.release());
    assign->append_kid(ast_A);

    return assign.release();

  } else {
    // A    → L
    return parse_L();
  }
}

Node *Parser2::parse_L() {
  // L    → R || R
  // L    → R && R
  // L    → R

  Node *ast_ = parse_R();
  std::unique_ptr<Node> ast(ast_);

  Node *next_tok = m_lexer->peek(1);
  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for L");
  }
  int next_tok_tag = next_tok->get_tag();
  if (next_tok_tag == TOK_OR || next_tok_tag == TOK_AND)  {
    // L    → R || R
    // L    → R && R
    std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

    // build AST for next term, incorporate into current AST
    Node *right = parse_R();
    ast.reset(new Node(next_tok_tag == TOK_OR ? AST_LOGICAL_OR : AST_LOGICAL_AND, {ast.release(), right}));

    // copy source information from operator node
    ast->set_loc(op->get_loc());
  }
  return ast.release();
}

Node *Parser2::parse_R() {
  //  R    → E < E
  //  R    → E <= E
  //  R    → E > E
  //  R    → E >= E
  //  R    → E == E
  //  R    → E != E
  //  R    → E

  Node *ast_ = parse_E();
  std::unique_ptr<Node> ast(ast_);

  Node *next_tok = m_lexer->peek(1);
  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for R");
  }
  int next_tok_tag = next_tok->get_tag();
  if (next_tok_tag == TOK_LESS || next_tok_tag == TOK_GREATER || next_tok_tag == TOK_IS_EQUAL ||
  next_tok_tag == TOK_LESS_EQUAL || next_tok_tag == TOK_GREATER_EQUAL || next_tok_tag == TOK_NOT_EQUAL ){
    std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

    Node *right = parse_E();
    int op_tag;
    switch (next_tok_tag) {
      case TOK_LESS:
        op_tag = AST_LESS;
        break;
      case TOK_GREATER:
        op_tag = AST_GREATER;
        break;
      case TOK_IS_EQUAL:
        op_tag = AST_ISEQUAL;
        break;
      case TOK_LESS_EQUAL:
        op_tag = AST_LESSEQUAL;
        break;
      case TOK_GREATER_EQUAL:
        op_tag = AST_GREATEREQUAL;
        break;
      case TOK_NOT_EQUAL:
        op_tag = AST_ISNOTEQUAL;
        break;
    }
    ast.reset(new Node(op_tag, {ast.release(), right}));
    ast->set_loc(op->get_loc());
  }
  return ast.release();
}

Node *Parser2::parse_E() {
  // E -> ^ T E'

  // Get the AST corresponding to the term (T)
  Node *ast = parse_T();

  // Recursively continue the additive expression
  return parse_EPrime(ast);
}

// This function is passed the "current" portion of the AST
// that has been built so far for the additive expression.
Node *Parser2::parse_EPrime(Node *ast_) {
  // E' -> ^ + T E'
  // E' -> ^ - T E'
  // E' -> ^ epsilon

  std::unique_ptr<Node> ast(ast_);

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr) {
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_PLUS || next_tok_tag == TOK_MINUS)  {
      // E' -> ^ + T E'
      // E' -> ^ - T E'
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // build AST for next term, incorporate into current AST
      Node *term_ast = parse_T();
      ast.reset(new Node(next_tok_tag == TOK_PLUS ? AST_ADD : AST_SUB, {ast.release(), term_ast}));

      // copy source information from operator node
      ast->set_loc(op->get_loc());

      // continue recursively
      return parse_EPrime(ast.release());
    }
  }

  // E' -> ^ epsilon
  // No more additive operators, so just return the completed AST
  return ast.release();
}

Node *Parser2::parse_T() {
  // T -> F T'

  // Parse primary expression
  Node *ast = parse_F();

  // Recursively continue the multiplicative expression
  return parse_TPrime(ast);
}

Node *Parser2::parse_TPrime(Node *ast_) {
  // T' -> ^ * F T'
  // T' -> ^ / F T'
  // T' -> ^ epsilon

  std::unique_ptr<Node> ast(ast_);

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr) {
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_TIMES || next_tok_tag == TOK_DIVIDE)  {
      // T' -> ^ * F T'
      // T' -> ^ / F T'
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // build AST for next primary expression, incorporate into current AST
      Node *primary_ast = parse_F();
      ast.reset(new Node(next_tok_tag == TOK_TIMES ? AST_MULTIPLY : AST_DIVIDE, {ast.release(), primary_ast}));

      // copy source information from operator node
      ast->set_loc(op->get_loc());

      // continue recursively
      return parse_TPrime(ast.release());
    }
  }

  // T' -> ^ epsilon
  // No more multiplicative operators, so just return the completed AST
  return ast.release();
}

Node *Parser2::parse_F() {
  // F -> ^ number
  // F -> ^ ident
  // F -> ^ ( E ) Done - Changed to F -> ^ ( A )
  // F -> ^ ident ( OptArgList ) Done
  // F -> ^ string_literal Done

  Node *next_tok = m_lexer->peek();
  if (next_tok == nullptr) {
    error_at_current_loc("Unexpected end of input looking for primary expression");
  }

  int tag = next_tok->get_tag();

  // F -> ^ ident ( OptArgList )
  Node *next_next_tok = m_lexer->peek(2);
  if (next_next_tok != nullptr && next_next_tok->get_tag() == TOK_LPAREN && next_tok->get_tag() == TOK_IDENTIFIER){
    std::unique_ptr<Node> tok(expect(TOK_IDENTIFIER));
    std::unique_ptr<Node> fncall(new Node(AST_FNCALL));

    // function reference
    std::unique_ptr<Node> varRef(new Node(AST_VARREF));
    varRef->set_str(tok->get_str());
    varRef->set_loc(tok->get_loc());
    fncall->append_kid(varRef.release());

    // function arguments
    expect_and_discard(TOK_LPAREN);
    fncall->append_kid(parse_OptArgList());
    expect_and_discard(TOK_RPAREN);

    tok.release();
    return fncall.release();
  };

  if (tag == TOK_INTEGER_LITERAL || tag == TOK_IDENTIFIER) {
    // F -> ^ number
    // F -> ^ ident
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(tag)));
    int ast_tag = tag == TOK_INTEGER_LITERAL ? AST_INT_LITERAL : AST_VARREF;
    std::unique_ptr<Node> ast(new Node(ast_tag));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    return ast.release();
  } else if (tag == TOK_LPAREN) {
    // F -> ^ ( E ) DONE - Changed to F -> ^ ( A )
    expect_and_discard(TOK_LPAREN);
    std::unique_ptr<Node> ast(parse_A());
    expect_and_discard(TOK_RPAREN);
    return ast.release();
  } else if (tag == TOK_STRING) {
    // F -> string_literal
    std::unique_ptr<Node> tok(expect(TOK_STRING));
    std::unique_ptr<Node> ast(new Node(AST_STRING_LITERAL));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    return ast.release();
  } else {
    SyntaxError::raise(next_tok->get_loc(), "Invalid primary expression");
  }
}

Node *Parser2::parse_OptArgList() {
  // OptArgList -> ArgList
  // OptArgList -> epsilon
  std::unique_ptr<Node> ast(new Node(AST_ARGLIST));

  Node *next_tok = m_lexer->peek(1);
  if (next_tok != nullptr && next_tok->get_tag() != TOK_RPAREN) {
    return parse_ArgList(ast.release());
  }
  return ast.release();
}

Node *Parser2::parse_ArgList(Node *ast_) {
  // ArgList -> L
  // ArgList -> L , ArgList
  for (;;) {
    ast_->append_kid(parse_L());
    Node* next_tok = m_lexer->peek(1);
    if (next_tok == nullptr || next_tok->get_tag() != TOK_COMMA)
      break;
    else {
      expect_and_discard(TOK_COMMA);
    }
  }
  return ast_;
}


Node *Parser2::expect(enum TokenKind tok_kind) {
  std::unique_ptr<Node> next_terminal(m_lexer->next());
  if (next_terminal->get_tag() != tok_kind) {
    SyntaxError::raise(next_terminal->get_loc(), "Unexpected token '%s'", next_terminal->get_str().c_str());
  }
  return next_terminal.release();
}

void Parser2::expect_and_discard(enum TokenKind tok_kind) {
  Node *tok = expect(tok_kind);
  delete tok;
}

void Parser2::error_at_current_loc(const std::string &msg) {
  SyntaxError::raise(m_lexer->get_current_loc(), "%s", msg.c_str());
}
