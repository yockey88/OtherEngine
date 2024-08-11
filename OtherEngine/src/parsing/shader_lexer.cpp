/**
 * \file parsing/shader_lexer.cpp
 **/
#include "parsing/shader_lexer.hpp"
#include "parsing/parsing_defines.hpp"

namespace other {

  ShaderLexResult ShaderLexer::Lex() {
    if (file_data.src.empty() || file_data.src == "") {
      return {};
    }

    result.tokens.push_back(Token(loc ,TokenType::START_SRC , ""));

    loc.index = 0;
    loc.line = 1;
    loc.column = 1;

    while (!AtEnd()) {
      char c = Peek();

      if (IsWhitespace(c)) {
        HandleWhitespace();
        continue;
      }

      if (IsNumeric(c)) {
        HandleNumeric();
        continue;
      }

      if (IsAlpha(c)) {
        HandleAlpha();
        continue;
      }

      if (std::find(kOperators.begin() , kOperators.end() , c) != kOperators.end()) {
        HandleOperator();
        continue;
      }

      throw Error(SYNTAX_ERROR , "Could not handle character : '{}'" , c);
    }

    result.tokens.push_back(Token(loc , TokenType::END_SRC , ""));

    return result;
  }

  void ShaderLexer::HandleWhitespace() {
    while (IsWhitespace(Peek())) {
      if (Check('\n')) {
        NewLine();
      } else {
        Consume();
      }
    }
  }

  void ShaderLexer::HandleNumeric() {
    while (IsNumeric(Peek())) {
      Advance();

      if (AtEnd()) {
        break;
      }
    }

    if (Check('.')) { 
      HandleFloat();
    } else if (Check('e') || Check('E')) { 
      HandleScientific();
    } else if (Check('x') || Check('X')) {
      throw Error(SYNTAX_ERROR , "Hexadecimal numbers not supported");
    } else {
      AddToken(TokenType::INT_LIT);
    }
    flags.sign = false;
  }

  void ShaderLexer::HandleFloat() {
    Advance(); // Consume '.'
    if (IsNumeric(Peek())) {
      while (IsNumeric(Peek())) {
        Advance();
      }
    } 

    if (Check('.')) {
      throw Error(SYNTAX_ERROR , "Invalid float literal");
    }

    if (Check('e') || Check('E')) {
      HandleScientific();
      return;
    }

    if (Check('f')) {
      Consume();
    }

    if (std::stod(current_token) > std::numeric_limits<float>::max()) {
      throw Error(INVALID_VALUE , "Float value in shader is too large {}" , current_token);
    }
      
    AddToken(TokenType::FLOAT_LIT);
  }

  void ShaderLexer::HandleScientific() {
    Consume(); // Consume 'e' or 'E'

    bool small = false;

    double lead_digit;
    try {
      lead_digit = std::stod(current_token);
    } catch (std::out_of_range& e) {
      throw Error(INVALID_VALUE , "Scientific notation lead digit out of range");
    } catch (std::invalid_argument& e) {
      throw Error(INVALID_VALUE , "Scientific notation lead digit is not a number");
    }

    DiscardToken();

    if (Check('-')) {
      Consume();
      small = true;
    }

    if (IsNumeric(Peek())) {
      while (IsNumeric(Peek())) {
        Advance();
      }
    }

    uint32_t val;
    try {
      val = std::stoi(current_token);
    } catch (std::out_of_range& e) {
      throw Error(INVALID_VALUE , "Scientific notation exponent out of range");
    } catch (std::invalid_argument& e) {
      throw Error(INVALID_VALUE , "Scientific notation exponent is not a number");
    } 

    std::string small_str = "0.";
    if (small) {
      auto pos = std::to_string(lead_digit).find('.');
      std::string lead_digit_str = pos == std::string::npos ? 
        std::to_string(lead_digit) : 
        std::to_string(lead_digit).substr(0, pos) + std::to_string(lead_digit).substr(pos + 1, current_token.size() - pos - 1);

      while (lead_digit_str[lead_digit_str.size() - 1] == '0') {
        lead_digit_str.pop_back();
      }

      for (uint32_t i = 0; i < val - 1; i++) {
        small_str += "0";
      }
      small_str += lead_digit_str;
    }

    std::string  result = small ? 
      small_str :
      std::to_string(lead_digit * std::pow(10, val));
    current_token = result;

    if (std::stod(current_token) > std::numeric_limits<float>::max()) {
      throw Error(ShaderError::INVALID_VALUE ,"Float value in shader is too large {}" , current_token);
    }
      
    AddToken(TokenType::FLOAT_LIT);
  }

  void ShaderLexer::HandleAlpha() {
    while (IsAlphaNumeric(Peek()) || Peek() == '_') {
      Advance();
    }

    if (IsKeyword(current_token)) {
      AddToken(GetKeywordType(current_token));
    } else {
      if (current_token.substr(0 , 3) == "std") {
        AddToken(STDXXX_KW);
      } else {
        AddToken(IDENTIFIER);
      }
    }
  }
  
  void ShaderLexer::HandleOperator() {
    char c = Peek();

    Advance();

    switch (c) {
      case '{':
        AddToken(OPEN_BRACE);
      break;
      case '}':
        AddToken(CLOSE_BRACE);
      break;
      case '(':
        AddToken(OPEN_PAREN);
      break;
      case ')':
        AddToken(CLOSE_PAREN);
      break;
      case '[':
        AddToken(OPEN_BRACKET);
      break;
      case ']':
        AddToken(CLOSE_BRACKET);
      break;
      case '+': 
        if (Check('=')) {
          Advance();
          AddToken(PLUS_EQUAL);
        } else {
          AddToken(PLUS);
        }
      break;
      case '-':
        AddToken(MINUS);
        if (IsNumeric(Peek())) {
          flags.sign = true;
        }
      break;
      case '*':
        AddToken(STAR);
      break;
      case '/':
        if (Check('/') || Check('*')) {
          HandleComment();
        } else {
          AddToken(F_SLASH);
        }
      break;
      case '=':
        if (Check('=')) {
          Advance();
          AddToken(EQUAL_EQUAL);
        } else {
          AddToken(EQUAL_OP);
        }
      break;
      case '<':
        if (Check('=')) {
          Advance();
          AddToken(LESS_EQUAL_OP);
        } else {
          AddToken(LESS_OP);
        }
      break;
      case '>':
        if (Check('=')) {
          Advance();
          AddToken(GREATER_EQUAL_OP);
        } else {
          AddToken(GREATER_OP);
        }
      break;
      case '!':
        if (Check('=')) {
          Advance();
          AddToken(BANG_EQUAL);
        } else {
          AddToken(BANG);
        }
      break;
      case '&':
        if (Check('&')) {
          Advance();
          AddToken(LOGICAL_AND);
        } else {
          throw Error(ShaderError::SYNTAX_ERROR , "Unknown operator : '&'");
        }
      break;
      case '|':
        if (Check('|')) {
          Advance();
          AddToken(LOGICAL_OR);
        } else {
          throw Error(ShaderError::SYNTAX_ERROR , "Unknown operator : '|'");
        }
      break;
      case ';':
        AddToken(SEMICOLON);
      break;
      case ':':
        AddToken(COLON);
      break;
      case ',':
        AddToken(COMMA);
      break;
      case '.':
        AddToken(DOT);
      break;
      case '#':
        AddToken(HASH);
      break;

      default:
        throw Error(ShaderError::SYNTAX_ERROR , "Unknown operator : '{}'" , c);
    }
  }
      
  void ShaderLexer::HandleComment() {
    // SourceLocation start = loc;

    if (Check('/')) {
      while (!Check('\n') && !AtEnd()) {
        Consume();
      }
    } else if (Check('*')) {
      while (!(Check('*') && CheckNext('/')) && !AtEnd()) {
        if (Check('\n')) {
          NewLine();
        }
        Consume();
      }

      if (AtEnd()) {
        throw Error(ShaderError::SYNTAX_ERROR , "Unterminated block comment");
      }

      Consume();
      Consume();
    } else {
      throw Error(ShaderError::SYNTAX_ERROR , "Unknown comment type");
    }

    DiscardToken();
  }
void ShaderLexer::AddToken(TokenType type) {
    result.tokens.push_back(Token(loc, type, current_token));
    DiscardToken();
  }

  void ShaderLexer::NewLine(bool advance) {
    ++loc.line;
    loc.column = 1;
    if (advance) {
      ++loc.index;
    }
  }

  void ShaderLexer::Consume() {
    ++loc.index;
    ++loc.column;
  }

  void ShaderLexer::Advance() {
    current_token += file_data.src[loc.index];
    Consume();
  }

  void ShaderLexer::DiscardToken() {
    current_token = "";
  }

  bool ShaderLexer::AtEnd() const {
    return loc.index >= file_data.src.size();
  }

  char ShaderLexer::Peek() const {
    if (AtEnd()) {
      return '\0';
    }
    return file_data.src[loc.index];
  }

  char ShaderLexer::PeekNext() const {
    if (AtEnd() || loc.index + 1 >= file_data.src.size()) {
      return '\0';
    }
    return file_data.src[loc.index + 1];
  }

  bool ShaderLexer::Check(char c) const {
    return Peek() == c;
  }

  bool ShaderLexer::CheckNext(char c) const {
    return PeekNext() == c;
  }

  bool ShaderLexer::Match(char expected) {
    if (!Check(expected)) {
      return false;
    }
    Advance();
    return true;
  }
      
  TokenType ShaderLexer::GetKeywordType(const std::string& str) const {
    try {
      auto itr = std::find_if(kKeywordMap.begin() , kKeywordMap.end() , [&str](const KeyWordPair& kw) -> bool {
        return FNV(str) == kw.first;
      });
      if (itr != kKeywordMap.end()) {
        return itr->second;
      }
      return VOID_KW;
    } catch (...) {
      std::string msg = fmtstr("Unknown operator : '{}'" , str);
      throw Error(ShaderError::SYNTAX_ERROR , msg);
    }
  }

} // namespace other
