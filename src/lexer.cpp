#include <map>
#include <cassert>
#include <cctype>
#include <string>
#include "cpputil.h"
#include "token.h"
#include "exceptions.h"
#include "lexer.h"

////////////////////////////////////////////////////////////////////////
// Lexer implementation
////////////////////////////////////////////////////////////////////////

Lexer::Lexer(FILE *in, const std::string &filename)
  : m_in(in)
  , m_filename(filename)
  , m_line(1)
  , m_col(1)
  , m_eof(false) {
}

Lexer::~Lexer() {
  // delete any cached lookahead tokens
  for (auto i = m_lookahead.begin(); i != m_lookahead.end(); ++i) {
    delete *i;
  }
  fclose(m_in);
}

Node *Lexer::next() {
  fill(1);
  if (m_lookahead.empty()) {
    SyntaxError::raise(get_current_loc(), "Unexpected end of input");
  }
  Node *tok = m_lookahead.front();
  m_lookahead.pop_front();
  return tok;
}

Node *Lexer::peek(int how_many) {
  // try to get as many lookahead tokens as required
  fill(how_many);

  // if there aren't enough lookahead tokens,
  // then the input ended before the token we want
  if (int(m_lookahead.size()) < how_many) {
    return nullptr;
  }

  // return the pointer to the Node representing the token
  return m_lookahead.at(how_many - 1);
}

Location Lexer::get_current_loc() const {
  return Location(m_filename, m_line, m_col);
}

// Read the next character of input, returning -1 (and setting m_eof to true)
// if the end of input has been reached.
int Lexer::read() {
  if (m_eof) {
    return -1;
  }
  int c = fgetc(m_in);
  if (c < 0) {
    m_eof = true;
  } else if (c == '\n') {
    m_col = 1;
    m_line++;
  } else {
    m_col++;
  }
  return c;
}

// "Unread" a character.  Useful for when reading a character indicates
// that the current token has ended and the next one has begun.
void Lexer::unread(int c) {
  ungetc(c, m_in);
  if (c == '\n') {
    m_line--;
    m_col = 99;
  } else {
    m_col--;
  }
}

void Lexer::fill(int how_many) {
  assert(how_many > 0);
  while (!m_eof && int(m_lookahead.size()) < how_many) {
    Node *tok = read_token();
    if (tok != nullptr) {
      m_lookahead.push_back(tok);
    }
  }
}

Node *Lexer::read_token() {
  int c, line = -1, col = -1;

  // skip whitespace characters until a non-whitespace character is read
  for (;;) {
    line = m_line;
    col = m_col;
    c = read();
    if (c < 0 || !isspace(c)) {
      break;
    }
  }

  if (c < 0) {
    // reached end of file
    return nullptr;
  }

  std::string lexeme;
  lexeme.push_back(char(c));

  if (isalpha(c)) {
    Node *tok = read_continued_token(TOK_IDENTIFIER, lexeme, line, col, isalnum);
    // DONE: use set_tag to change the token kind if it's actually a keyword
    if (tok -> get_str() == "var") {
        tok->set_tag(TOK_VAR);

    // A2 DONE: add cases for control flow and function
    } else if (tok -> get_str() == "if") {
        tok -> set_tag(TOK_IF);
    } else if (tok -> get_str() == "else") {
        tok -> set_tag(TOK_ELSE);
    } else if (tok -> get_str() == "while") {
        tok -> set_tag(TOK_WHILE);
    } else if (tok -> get_str() == "function") {
        tok->set_tag(TOK_FUNCTION);
    }
    return tok;
  } else if (isdigit(c)) {
    return read_continued_token(TOK_INTEGER_LITERAL, lexeme, line, col, isdigit);
  } else {
    switch (c) {
    case '+':
      return token_create(TOK_PLUS, lexeme, line, col);
    case '-':
      return token_create(TOK_MINUS, lexeme, line, col);
    case '*':
      return token_create(TOK_TIMES, lexeme, line, col);
    case '/':
      return token_create(TOK_DIVIDE, lexeme, line, col);
    case '(':
      return token_create(TOK_LPAREN, lexeme, line, col);
    case ')':
      return token_create(TOK_RPAREN, lexeme, line, col);
    case ';':
      return token_create(TOK_SEMICOLON, lexeme, line, col);
    // DONE: add cases for other kinds of tokens
    case '=':
      return try_two_character_token('=', '=', TOK_ASSIGN, TOK_IS_EQUAL, line, col);
    case '<':
      return try_two_character_token('<', '=', TOK_LESS, TOK_LESS_EQUAL, line, col);
    case '>':
      return try_two_character_token('>', '=', TOK_GREATER, TOK_GREATER_EQUAL, line, col);
    case '&':
      return try_two_character_token('&', '&', ERROR, TOK_AND, line, col);
    case '|':
      return try_two_character_token('|', '|', ERROR, TOK_OR, line, col);
    case '!':
      return try_two_character_token('!', '=', ERROR, TOK_NOT_EQUAL, line, col);
    // A2 DONE: add cases for grouping and sequencing tokens
    case '{':
      return token_create(TOK_LBRACE, lexeme, line, col);
    case '}':
      return token_create(TOK_RBRACE, lexeme, line, col);
    case ',':
      return token_create(TOK_COMMA, lexeme, line, col);
    case '"':
      return string_create(lexeme, line, col);
    default:
      SyntaxError::raise(get_current_loc(), "Unrecognized character '%c'", c);
    }
  }
}

// Helper function to create a Node object to represent a token.
Node *Lexer::token_create(enum TokenKind kind, const std::string &lexeme, int line, int col) {
  Node *token = new Node(kind, lexeme);
  Location source_info(m_filename, line, col);
  token->set_loc(source_info);
  return token;
}

// Read the continuation of a (possibly) multi-character token, such as
// an identifier or integer literal.  pred is a pointer to a predicate
// function to determine which characters are valid continuations.
Node *Lexer::read_continued_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int (*pred)(int)) {
  std::string lexeme(lexeme_start);
  for (;;) {
    int c = read();
    if (c >= 0 && pred(c)) {
      // token has finished
      lexeme.push_back(char(c));
    } else {
      if (c >= 0) {
        unread(c);
      }
      return token_create(kind, lexeme, line, col);
    }
  }
}

// DONE: implement additional member functions if necessary
Node *Lexer::try_two_character_token(char first, char second, TokenKind oneCharKind, TokenKind twoCharKind, int line, int col) {
  std::string lexeme(1, first);
  int next_c = read();
  if (next_c == second) {
    lexeme.push_back(char(next_c));
    return token_create(twoCharKind, lexeme, line, col);
  } else if (oneCharKind == ERROR) {
    SyntaxError::raise(get_current_loc(), "Unrecognized character '%c'", first);
  } else {
    unread(next_c);
    return token_create(oneCharKind, lexeme, line, col);
  }
}

Node *Lexer::string_create(const std::string &lexeme_start, int line, int col) {
  std::string lexeme = lexeme_start;
  lexeme.pop_back();
  while (true) {
    int c = read();
    if (c < 0) {
      SyntaxError::raise(get_current_loc(), "Unexpected end of string input");
    } else if (c == '\\') {
      int next_c = read();
      switch (next_c) {
        case 'n':
          lexeme.push_back('\n');
          break;
        case 't':
          lexeme.push_back('\t');
          break;
        case 'r':
          lexeme.push_back('\r');
          break;
        case '\"':
          lexeme.push_back('\"');
          break;
        default:
          SyntaxError::raise(get_current_loc(), "Invalid escape character");
      }
    } else if (c == '\"') {
      break;
    } else {
      lexeme.push_back(static_cast<char>(c));
    }
  }
  return token_create(TOK_STRING, lexeme, line, col);
}

