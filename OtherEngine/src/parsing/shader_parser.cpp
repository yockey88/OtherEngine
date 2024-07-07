/**
 * \file parsing/shader_parser.cpp
 **/
#include "parsing/shader_parser.hpp"

#include "core/errors.hpp"
#include "core/logger.hpp"

#include "parsing/ast_node.hpp"
#include "parsing/parsing_defines.hpp"
#include "parsing/shader_ast_node.hpp"

namespace other {
      
  ShaderIr ShaderParser::Parse() {
    if (tokens.size() < 2) {
      return {};
    }

    current = 0;
    Consume(START_SRC , "Expected Start of File");

    std::vector<Ref<AstStmt>> parsed_file{};
    while (!AtEnd()) {
      if (Check(END_SRC)) {
        break;
      }

      Ref<AstStmt> s = ParseDecl();
      if (s != nullptr) {
        parsed_file.push_back(s);
      } else {
        throw Error(SYNTAX_ERROR , "Failed to parse declaration!");
      }
    }

    Consume(END_SRC , "Expected End of File");

    /// walk ast to produce ir

    return result; 
  }

  Ref<AstStmt> ShaderParser::ParseDecl() {
    bool no_advance = false;
    
    if (MatchTypes(no_advance)) {
      return ResolveDecl();
    }

    if (Match({ IDENTIFIER } , false)) {
      return ResolveDecl();
    }

    if (Match({ LAYOUT_KW })) {
      flags.initializer_valid = false;
      Ref<AstStmt> stmt = ParseLayoutDecl();
      flags.initializer_valid = true;
      return stmt;
    }

    if (Match({ STRUCT_KW })) {
      Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier! found {}" , Peek().value));
      return ParseStructDecl(identifier);
    }

    if (Match({ IN_KW })) {
      Token type = ConsumeType(fmtstr("Expected type keyword after {}! found {}" , Previous().value , Peek().value));
      Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier! found {}" , Peek().value));

      flags.initializer_valid = false;
      Ref<AstStmt> stmt = ParseVarDecl(type , identifier);
      flags.initializer_valid = true;

      return stmt;
    }

    if (Match({ OUT_KW })) {
      if (Check(IDENTIFIER) && Peek().type == OPEN_BRACE) {
        return ParseInOutBlock(Previous()); 
      }

      Token type = ConsumeType(fmtstr("Expected type keyword after {}! found {}" , Previous().value , Peek().value));
      Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier! found {}" , Peek().value));

      flags.initializer_valid = false;
      Ref<AstStmt> stmt = ParseVarDecl(type , identifier);
      flags.initializer_valid = true;

      return stmt;
    }

    return ParseStatement();
  }

  Ref<AstStmt> ShaderParser::ResolveDecl() {
    if (Match({ IDENTIFIER })) {
      Token identifier = Previous();

      if (Match({ EQUAL_OP })) {
        Ref<AstExpr> initializer = ParseExpression();
        Consume(SEMICOLON , fmtstr("Expected ';' to end assignment expression! found {}" , Peek().value));

        /// we don't know the type here so static analysis later to verify whats happening here
        Token type = Token({} , INVALID_TOKEN , "");
        return NewRef<VarDecl>(type , identifier , initializer);
      }

      // if dot
      // if etc...

      throw Error(SYNTAX_ERROR , fmtstr("Unable to resolve identifier {}! found {}" , identifier.value , Peek().value));
    }

    Token type = ConsumeType(fmtstr("Expected type keyword! found {}" , Peek().value));
    Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier! found {}" , Peek().value));

    /// if initializers are not valid then functions and function calls are not either
    Ref<AstStmt> initializer;
    if (flags.initializer_valid) {
      if (Check(OPEN_PAREN)) {
        return ParseFunctionDecl(type , identifier);
      }

    } else if (Match({ OPEN_BRACKET })) {
      return ParseArrayDecl(type , identifier);
    } else {
      Consume(SEMICOLON , fmtstr("Expected semicolon! found {}" , Peek().value));
    }

    return NewRef<VarDecl>(type , identifier , initializer); 
  }

  Ref<AstStmt> ShaderParser::ParseLayoutDecl() {
    Consume(OPEN_PAREN , fmtstr("Expected open parathesis after 'layout'! found {}" , Peek().value));

    Token rules = Token({} , INVALID_TOKEN , "");
    std::vector<Ref<AstExpr>> descriptors;
    if (Match({ STDXXX_KW })) {
      rules = Previous();
      if (Match({ CLOSE_PAREN })) {
        Ref<AstStmt> layout = ParseLayoutStmt();
        return NewRef<LayoutDecl>(rules , descriptors , layout);
      }

      Consume(COMMA , fmtstr("Expected ',' or ')' in layout definition! found {}" , Peek().value));
    }

    while(!Check(CLOSE_PAREN)) {
      descriptors.push_back(ParseLayoutDescriptor()); 
      if (!Check(CLOSE_PAREN)) {
        Consume(COMMA , fmtstr("Expected ',' or ')' in layout definition! found {}" , Peek().value));
      }
    }

    Consume(CLOSE_PAREN , "Expected close parathesis after layout descriptor set");

    Ref<AstStmt> layout = ParseLayoutStmt();

    return NewRef<LayoutDecl>(rules , descriptors , layout);
  }
      
  Ref<AstStmt> ShaderParser::ParseFunctionDecl(const Token& type , const Token& name) { 
    std::vector<Token> params;
    if (Match({ OPEN_PAREN })) {
      while (!Check(CLOSE_PAREN) && !AtEnd()) {
        Token param = Consume(IDENTIFIER , "Expected identifier for function parameter");
        param.type = INVALID_TOKEN;

        Consume(COLON , "Expected ':' after function parameter identifier");

        Token type = ConsumeType("Expected type after ':' for function parameter");
        param.type = type.type;

        if (Match({ OPEN_BRACKET })) {
          Consume(CLOSE_BRACKET , "Expected closing bracket for array type");
        }

        params.push_back(param);
        if (!Match({ COMMA })) {
          break;
        }
      }
      Consume(CLOSE_PAREN , "Expected closing parenthesis for function parameters");
    }
    
    Ref<AstStmt> body = ParseBlock();
    if (body == nullptr) {
      throw Error(SYNTAX_ERROR , "Expected function body");
    }

    if (Match({ SEMICOLON })) {
      // do nothing
    }

    return NewRef<FunctionStmt>(name , params , type , body);
  }

  Ref<AstStmt> ShaderParser::ParseStructDecl(const Token& name) { 
    OE_DEBUG("Struct decl current token {}" , Peek().value);
    return nullptr; 
  }

  Ref<AstStmt> ShaderParser::ParseVarDecl(const Token& type , const Token& name) {
    if (Match({ SEMICOLON })) {
      return NewRef<VarDecl>(type , name);
    } else if (Match({ OPEN_BRACKET })) {
      return ParseArrayDecl(type , name);
    }

    Ref<AstExpr> initializer = nullptr;
    if (flags.initializer_valid) {
      initializer = ParseExpression();
      Consume(SEMICOLON , fmtstr("Expected semicolon! found {}" , Peek().value));
    } else {
      Consume(SEMICOLON , "Initializer invalid in this context!");
    }

    return NewRef<VarDecl>(type , name , initializer);
  }

  Ref<AstStmt> ShaderParser::ParseArrayDecl(const Token& type , const Token& name) { 
    Token size = Token({} , INT_LIT , "");
    if (Match({ INT_LIT })) {
      size = Consume(INT_LIT , fmtstr("Expected integer literal for array size! found {}" , Peek().type));
    } 
    Consume(CLOSE_BRACKET , fmtstr("Expected close bracket after array size! found {}" , Peek().type));

    Ref<AstExpr> initializer = nullptr;
    if (flags.initializer_valid) {
      if (Match({ EQUAL_OP })) {
        /// do nothing
      }

      initializer = ParseArrayExpr(size);

      Consume(SEMICOLON , fmtstr("Expected semicolon! found {}" , Peek().value));
    } else {
      Consume(SEMICOLON , "Initializer invalid in this context!");
    }

    return NewRef<ArrayDecl>(type , name , size , initializer);
  }
      
  Ref<AstStmt> ShaderParser::ParseBufferDecl() {
    Token buff_type = Token({} , INVALID_TOKEN , "");
    if (Peek().type == READONLY_KW) {
      Advance();

      buff_type = Consume(BUFFER_KW , fmtstr("Can only make Shader Storage Buffers read only! found {}" , Peek().value));
    } else {
      if (Match({ UNIFORM_KW , BUFFER_KW } , false)) {
        buff_type = Advance();
      } else {
        throw Error(SYNTAX_ERROR , fmtstr("Expected 'uniform' or 'buffer'! found {}" , Peek().value));
      }
    }

    Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier to name Shader Storage! found {}" , Peek().value));

    Ref<AstStmt> body = ParseBlock();

    return NewRef<ShaderStorageStmt>(buff_type , identifier , body);
  }

  Ref<AstStmt> ShaderParser::ParseLayoutStmt() {
    if (Match({ IN_KW })) {
      Token type = ConsumeType(fmtstr("Expected type keyword after {}! found {}" , Previous().value , Peek().value));
      Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier! found {}" , Peek().value));

      flags.initializer_valid = false;
      Ref<AstStmt> stmt = ParseVarDecl(type , identifier);
      flags.initializer_valid = true;

      return stmt;
    }

    if (Match({ READONLY_KW , BUFFER_KW , UNIFORM_KW } , false)) {
      return ParseBufferDecl();
    }

    throw Error(SYNTAX_ERROR , fmtstr("Failed to parse layout statement! found {}" , Peek().value));
  }

  Ref<AstStmt> ShaderParser::ParseStatement() { 
    if (Match({ OPEN_BRACE })) {
      Ref<AstStmt> block = ParseBlock();
      if (block == nullptr) {
        throw current;
      }

      return block;
    }

    Ref<ExprStmt> stmt = ParseExpression();
    // Consume(SEMICOLON , fmtstr("Expected ';' to end statement! found {}" , Peek().value));
    
    return stmt;
  }

  Ref<AstStmt> ShaderParser::ParseInOutBlock(const Token& type) {
    Token tag = Consume(IDENTIFIER , fmtstr("Expected tag for in/out block! found {}" , Peek().value));
    
    Consume(OPEN_BRACE , fmtstr("Expected open brace for in/out block! found {}" , Peek().value));

    std::vector<Ref<AstStmt>> var_decls;
    flags.initializer_valid = false;
    while (!Check(CLOSE_BRACE)) {
      var_decls.push_back(ParseDecl());
    }
    flags.initializer_valid = true;
    
    Consume(CLOSE_BRACE , fmtstr("Expected close brace for in/out block! found {}" , Peek().value));

    return nullptr;
  }

  Ref<AstStmt> ShaderParser::ParseBlock() {
    Consume(TokenType::OPEN_BRACE , "Expected opening brace for function declaration"); 
    std::vector<Ref<AstStmt>> stmts;

    // current_scope++;

    while (!Check(TokenType::CLOSE_BRACE) && !AtEnd()) {
      Ref<AstStmt> stmt = ParseDecl();
      if (stmt != nullptr) {
        stmts.push_back(stmt);
      } else {
        throw Error(SYNTAX_ERROR , "Failed to parse block");
      }
    }

    Consume(TokenType::CLOSE_BRACE , "Expected closing brace");
    if (Match({ TokenType::SEMICOLON })) {
      // do nothing
    }

    // declarations[current_scope].types.clear();
    // declarations[current_scope].vars.clear();
    // declarations[current_scope].funcs.clear();
    // current_scope--;

    return NewRef<BlockStmt>(stmts);
  }

  Ref<AstStmt> ShaderParser::ParseIf() { return nullptr; }
  Ref<AstStmt> ShaderParser::ParseWhile() { return nullptr; }
  Ref<AstStmt> ShaderParser::ParseFor() { return nullptr; }
  Ref<AstStmt> ShaderParser::ParseReturn() { return nullptr; }
      
  Ref<AstExpr> ShaderParser::ParseLayoutDescriptor() {
    if (Match({ BINDING_KW , LOCATION_KW })) {
      Token type = Previous();
      Consume(EQUAL_OP , fmtstr("Expected '=' after layout decsriptor! found {}" , Peek().value));
      Ref<AstExpr> val = ParseExpression();
      return NewRef<LayoutDescriptor>(type , val);
    }
    
    throw Error(SYNTAX_ERROR , fmtstr("Expected 'binding' or 'location'! found {}" , Peek().value));
  }

  Ref<AstExpr> ShaderParser::ParseExpression() {
    return ParseAssignment();
  }

  Ref<AstExpr> ShaderParser::ParseAssignment() {
    auto expr = ParseOr();

    if (Match({ TokenType::EQUAL_OP })) {
      /// Token('=') == Previous()
      Ref<AstExpr> right = ParseAssignment();
      if (expr->GetType() == AstNodeType::VAR_EXPR) {
        Ref<VarExpr> var = Ref<AstExpr>::Cast<VarExpr>(expr);
        return NewRef<AssignExpr>(var->name , right);
      }
      throw Error(SYNTAX_ERROR , fmtstr("Invalid assignment target {}" , expr->GetType()));
    }

    return expr;
  }

  
  Ref<AstExpr> ShaderParser::ParseOr() {
    auto expr = ParseAnd();

    while (Match({ LOGICAL_OR })) {
      Token op = Previous();
      Ref<AstExpr> right = ParseAnd();
      expr = NewRef<BinaryExpr>(expr , op , right);
    }

    return expr; 
  }

  Ref<AstExpr> ShaderParser::ParseAnd() {
    auto expr = ParseEquality();
    
    while (Match({ TokenType::LOGICAL_AND })) {
      Token op = Previous();
      Ref<AstExpr> right = ParseEquality();
      expr = NewRef<BinaryExpr>(expr , op , right);
    }

    return expr;
  }

  Ref<AstExpr> ShaderParser::ParseEquality() {
    auto expr = ParseComparison();

    while (Match({ BANG_EQUAL , EQUAL_EQUAL })) {
      Token op = Previous();
      Ref<AstExpr> right = ParseComparison();
      expr = NewRef<BinaryExpr>(expr , op , right);
    }

    return expr;
  }

  Ref<AstExpr> ShaderParser::ParseComparison() {
    auto expr = ParseTerm();

    while (Match({ GREATER_OP , GREATER_EQUAL_OP , LESS_OP , LESS_EQUAL_OP })) {
      Token op = Previous();
      Ref<AstExpr> right = ParseTerm();
      expr = NewRef<BinaryExpr>(expr , op , right);
    }

    return expr;
  }

  Ref<AstExpr> ShaderParser::ParseTerm() {
    auto expr = ParseFactor();
    
    while (Match({ PLUS , MINUS })) {
      Token op = Previous();
      Ref<AstExpr> right = ParseFactor();
      expr = NewRef<BinaryExpr>(expr , op , right);
    }

    return expr;
  }

  Ref<AstExpr> ShaderParser::ParseFactor() {
    auto expr = ParseCall();

    while (Match({ STAR , F_SLASH })) {
      Token op = Previous();
      Ref<AstExpr> right = ParseCall();
      expr = NewRef<BinaryExpr>(expr , op , right);
    }

    return expr;
  }

  Ref<AstExpr> ShaderParser::ParseCall() {
    auto expr = ParseLiteral();

    do {
      if (Match({ OPEN_PAREN })) {
        expr = FinishCall(expr);
      } else if (Match({ DOT })) {
        Token name = Consume(IDENTIFIER , fmtstr("Expected property or name after '.'! found {}" , Peek().value));
        expr = NewRef<ObjAccessExpr>(expr , name);
      } else {
        break;
      }
    } while (true);

    return expr;
  }

  Ref<AstExpr> ShaderParser::FinishCall(Ref<AstExpr>& callee) {
    std::vector<Ref<AstExpr>> args;
    if (!Check(CLOSE_PAREN)) {
      do {
        if (args.size() >= 255) {
          throw Error(SYNTAX_ERROR , "Funciton can not have more than 255 parameters");
        }
        args.push_back(ParseExpression());
      } while (Match({ COMMA }));
    }

    Consume(CLOSE_PAREN , fmtstr("Expected closing parenthesis after function parameters! found {}" , Peek().value));

    return NewRef<CallExpr>(callee , args);
  }

  Ref<AstExpr> ShaderParser::ParseLiteral() {
    if (MatchLiterals()) {
      return NewRef<LiteralExpr>(Previous());
    }

    return ResolveExpr();
  }

  Ref<AstExpr> ShaderParser::ResolveExpr() {
    if (Match({ HASH })) {
      Token version = Consume(IDENTIFIER , fmtstr("Expected 'version' after '#'! found {}" , Peek().type));
      if (version.value != "version") {
        throw Error(SYNTAX_ERROR , fmtstr("Expected 'version' after '#'! found {}" , version.value));
      }

      Token number = Consume(INT_LIT , fmtstr("Expected a version number for glsl! found {}" , Peek().type));
      Token type = Consume(CORE_KW , fmtstr("Expected 'core' after '#version XXX'! found {}" , Peek().type));

      return NewRef<VersionExpr>(number , type);
    }

    if (Match({ TokenType::OPEN_PAREN })) {
      Ref<AstExpr> expr = ParseExpression();
      Consume(TokenType::CLOSE_PAREN , "Expected closing parenthesis");
      return NewRef<GroupingExpr>(expr);
    }

    if (Match({ TokenType::PLUS , TokenType::MINUS })) {
      Token op = Previous();
      Ref<AstExpr> right = ParseLiteral();
      return NewRef<UnaryExpr>(op , right);
    }
    
    if (Match({ TokenType::IDENTIFIER })) {
      Ref<AstExpr> id = NewRef<VarExpr>(Previous());

      if (Match({ TokenType::OPEN_BRACKET } , false)) {
        return ParseArrayAccess(id);
      }


      if (Match({ TokenType::DOT })) {
        return ParseStructAccess(id);
      }

      return NewRef<VarExpr>(Previous());
    }

    if (MatchTypes()) {
      Ref<AstExpr> var_expr = NewRef<VarExpr>(Previous());
      
      /// if parentheses assume ctor call otherwise error out
      if (Match({ OPEN_PAREN })) {
        Ref<AstExpr> ctor_call = FinishCall(var_expr);
        return ctor_call;
      }
    }

    throw Error(SYNTAX_ERROR , fmtstr("Unable to resolve expression! found {}" , Peek().value));
  }

  Ref<AstExpr> ShaderParser::ParseArrayExpr(Token size) {
    Consume(OPEN_BRACE , fmtstr("Expected open brace to define array values! found {}" , Peek().value));

    std::vector<Ref<AstExpr>> elements;
    while (!Match({ TokenType::CLOSE_BRACE })) {
      do {
        if (elements.size() > std::stoi(size.value)) {
          throw Error(SYNTAX_ERROR , "Array size does not match number of values in initializer!");
        }
        elements.push_back(ParseExpression());
      } while (Match({ TokenType::COMMA }));
    }

    Consume(SEMICOLON , fmtstr("Expected closing bracket for array literal! found {}" , Peek().type));

    return NewRef<ArrayExpr>(elements);
  }
  
  Ref<AstExpr> ShaderParser::ParseArrayAccess(Ref<AstExpr>& expr) {
    Consume(TokenType::OPEN_BRACKET , "Expected opening bracket for array access");

    Ref<AstExpr> index = ParseExpression();
    Consume(TokenType::CLOSE_BRACKET , "Expected closing bracket for array access");

    return NewRef<ArrayAccessExpr>(expr , index);
  }

  Ref<AstExpr> ShaderParser::ParseStructAccess(Ref<AstExpr>& expr) {
    Token name = Consume(IDENTIFIER , fmtstr("Expected identifier for struct access! found {}" , Peek().value));

    Ref<AstExpr> assignment = nullptr;
    Ref<AstExpr> index = nullptr;
    
    if (Match({ EQUAL_OP })) {
      assignment = ParseExpression();
      Consume(SEMICOLON , "Expected semicolon after assignment expression");
    }

    if (Match({ OPEN_BRACKET })) {
      index = ParseExpression();
      Consume(CLOSE_BRACKET , "Expected closing bracket for struct array access");

      assignment = nullptr;
      if (Match({ EQUAL_OP })) {
        assignment = ParseExpression();
        Consume(SEMICOLON , "Expected semicolon after assignment expression");
      }
    }

    return NewRef<ObjAccessExpr>(expr , name , index , assignment);
  }

  Token ShaderParser::Peek() const {
    if (AtEnd()) {
      return tokens.back();
    }

    return tokens[current];
  }

  Token ShaderParser::Previous() const {
    if (current == 0) {
      return tokens.front();
    }

    return tokens[current - 1];
  }

  Token ShaderParser::Advance() {
    if (!AtEnd()) {
      current++;
    }

    return Previous();
  }
      
  Token ShaderParser::ConsumeType(const std::string& message) {
    if (AtEnd()) {
      throw Error(SYNTAX_ERROR , fmtstr("Consuming type at the end of file : {}" , message));
    }

    switch (Peek().type) {
      case VOID_KW:
      case INT_KW:
      case FLOAT_KW:
      case VEC2_KW:
      case VEC3_KW:
      case VEC4_KW:
      case MAT2_KW:
      case MAT3_KW:
      case MAT4_KW:
        return Advance();
      default:
        break;
    }

    throw Error(SYNTAX_ERROR , message);
  }
 
  Token ShaderParser::Consume(TokenType type, const std::string& message) {
    if (Check(type)) {
      return Advance();
    }

    throw Error(SYNTAX_ERROR , message);
  }
  
  bool ShaderParser::AtEnd() const {
    return current >= tokens.size();
  }

  bool ShaderParser::Check(TokenType type) const {
    if (AtEnd()) {
      return false;
    }

    return Peek().type == type;
  }
  
  bool ShaderParser::Match(const std::vector<TokenType>& types , bool advance) {
    for (const auto& type : types) {
      if (Check(type)) {
        if (advance) {
          Advance();
        }

        return true;
      }
    }

    return false;
  }
      
  bool ShaderParser::MatchLiterals(bool advance) {
    return Match({ 
      INT_LIT , FLOAT_LIT
    }); 
  }

  bool ShaderParser::MatchTypes(bool advance) {
    return Match({ 
      VOID_KW ,
      INT_KW , 
      FLOAT_KW ,
      VEC2_KW ,
      VEC3_KW ,
      VEC4_KW ,
      MAT2_KW ,
      MAT3_KW ,
      MAT4_KW ,
    }, advance);
  }

} // namespace other
