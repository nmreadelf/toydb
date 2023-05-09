#include "state.h"

#include <cctype>
#include <variant>

struct Error {
  std::string message;
};

template <typename T> using Result = std::variant<T, Error>;

namespace toydb::sql {
enum TokenType {
  // A number literal
  Number,
  // A string literal
  String,
  // A textual identifier
  Ident,
  // The period symbol .
  Period,
  // The equals symbol =
  Equals,
  // The greater-than symbol >
  GreaterThan,
  // The less-than symbol <
  LessThan,
  // The addition symbol +
  Plus,
  // The subtraction symbol -
  Minus,
  // The multiplication symbol *
  Asterisk,
  // The division symbol /
  Slash,
  // THe exponentiation symbol ^
  Caret,
  // The modulo symbol %
  Percent,
  // THe factorial or not symbol !
  Exclamation,
  // THe query parameter market ?
  Question,
  // An opening parenthesis (
  OpenParen,
  // A closing parenthesis )
  CloseParen,
  // An expression separator ,
  Comma,

  // End of file or line
  Eof,
  // Illegal
  Illegal,
};

struct Token {
  TokenType Type;
  std::string Literal;

  Token(TokenType type) : Type(type) {}

  Token(TokenType type, std::string &&value) : Type(type), Literal(value) {}

  bool operator==(const Token &t) {
    return Type == t.Type && Literal == t.Literal;
  }
};

class Lexer {
public:
  Lexer(std::string &data)
      : iter_(data.begin()), readIter_(data.begin()), end_(data.end()), ch_(0) {
    readChar();
  };

  Result<Token> NextToken() {
    skipWhitespace();
    Token tok(Illegal);

    switch (ch_) {
    case '.':
      tok = {Period};
      break;
    case '=':
      tok = {Equals};
      break;
    case '>':
      tok = {GreaterThan};
      break;
    case '<':
      tok = {LessThan};
      break;
    case '+':
      tok = {Plus};
      break;
    case '-':
      tok = {Minus};
      break;
    case '*':
      tok = {Asterisk};
      break;
    case '/':
      tok = {Slash};
      break;
    case '^':
      tok = {Caret};
      break;
    case '%':
      tok = {Percent};
      break;
    case '!':
      tok = {Exclamation};
      break;
    case '?':
      tok = {Question};
      break;
    case '(':
      tok = {OpenParen};
      break;
    case ')':
      tok = {CloseParen};
      break;
    case ',':
      tok = {Comma};
      break;
    case '\'':
      return Token{String, readString()};
      break;
    case 0:
      tok = {Eof};
      break;
    default:
      if (std::isdigit(ch_)) {
        return Token{Number, readNumber()};
      } else if (std::isalpha(ch_)) {
        return Token(Ident, readIdentifier());
      } else {
        tok = {Illegal};
      }
    }
    readChar();
    return tok;
  }

private:
  void readChar() {
    if (readIter_ >= end_) {
      ch_ = 0;
    } else {
      ch_ = *readIter_;
    }
    iter_ = readIter_;
    readIter_++;
  }

  std::string readIdentifier() {
    auto pos = iter_;
    while (std::isalnum(ch_) || ch_ == '_') {
      readChar();
    }

    return {pos, iter_};
  }

  void skipWhitespace() {
    while (ch_ == ' ' || ch_ == '\t' || ch_ == '\n' || ch_ == '\r') {
      readChar();
    }
  }

  std::string readNumber() {
    auto pos = iter_;
    while (std::isdigit(ch_)) {
      readChar();
    }

    if (ch_ == '.') {
      readChar();
      while (std::isdigit(ch_)) {
        readChar();
      }
    }

    if (ch_ == 'e' || ch_ == 'E') {
      readChar();
      if (ch_ == '+' || ch_ == '-') {
        readChar();
      }
      while (std::isdigit(ch_)) {
        readChar();
      }
    }

    return {pos, iter_};
  }

  // TODO: replace multi readChar, use need_if rewrite this
  std::string readString() {
    if (ch_ != '\'') {
      return "";
    }
    readChar();
    auto pos = iter_;
    while (true) {
      readChar();
      if (ch_ == 0) {
        break;
      }
      if (ch_ == '\'') {
        if (peekChar() == '\'') {
          readChar();
        } else {
          break;
        }
      }
    }

    std::string res(pos, iter_);
    readChar();

    return res;
  }

  char peekChar() {
    if (readIter_ >= end_) {
      return 0;
    }

    return *readIter_;
  }

  std::string::const_iterator iter_;
  std::string::const_iterator readIter_;
  std::string::const_iterator end_;
  char ch_;
};
}; // namespace toydb::sql
