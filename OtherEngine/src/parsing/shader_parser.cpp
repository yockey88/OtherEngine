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
      
  ShaderAst ShaderParser::Parse() {
    if (tokens.size() < 2) {
      return {};
    }

    current = 0;
    Consume(START_SRC , "Expected Start of File");

    while (!AtEnd()) {
      if (Check(END_SRC)) {
        break;
      }

      Ref<AstStmt> s = ParseDecl();
      if (s != nullptr) {
        AddTopLevelNode(s);
      } else {
        throw Error(SYNTAX_ERROR , "Failed to parse declaration!");
      }
    }

    Consume(END_SRC , "Expected End of File");

    return result; 
  }
      
  ShaderType ShaderParser::GetCurrentShaderType() {
    switch (current_context) {
      case VERTEX_CTX:
        return VERTEX_SHADER;
      case FRAGMENT_CTX:
        return FRAGMENT_SHADER;
      break;
      case GEOMETRY_CTX:
        return GEOMETRY_SHADER;
      default:
        throw Error(INVALID_SHADER_TYPE , "Parser State is corrupted!");
    }

    throw Error(INVALID_SHADER_TYPE , "Parser State is corrupted!");
  }

  Ref<AstStmt> ShaderParser::ParseDecl() {
    bool no_advance = false;

    /// if oshader parse special, otherwise don't do anything
    if (Match({ VERTEX_KW , FRAGMENT_KW , GEOMETRY_KW } , false) && start_context == VARIABLE_CTX) {
      if (Match({ VERTEX_KW })) {
        current_context = VERTEX_CTX;
      } else if (Match({ FRAGMENT_KW })) {
        current_context = FRAGMENT_CTX;
      } else if (Match({ GEOMETRY_KW })) {
        current_context = GEOMETRY_CTX;
      }

      return ParseShaderDecl();
    }
    
    if (MatchTypes(no_advance) || Match({ CONST_KW } , false)) {
      return ParseVarDecl();
    }

    if (Match({ UNIFORM_KW })) {
      return ParseUniformDecl();
    }

    if (Match({ IDENTIFIER } , false)) {
      return NewRef<ExprStmt>(ParseExpression());
    }

    if (Match({ LAYOUT_KW })) {
      flags.initializer_blocker.push(true);
      Ref<AstStmt> stmt = ParseLayoutDecl();
      flags.initializer_blocker.pop();
      return stmt;
    }

    if (Match({ STRUCT_KW })) {
      Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier! found {}" , Peek().value));
      return ParseStructDecl(identifier);
    }

    if (Match({ IN_KW , OUT_KW })) {
      Token in_out = Previous();
      if (Match({ IDENTIFIER })) {
        return ParseInOutBlock(in_out , Previous()); 
      }

      Token type = ConsumeType(fmtstr("Expected type keyword after {}! found {}" , Previous().value , Peek().value));
      Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier! found {}" , Peek().value));

      return NewRef<LayoutVarDecl>(in_out , type , identifier);
    }

    if (Match({ RETURN_KW })) {
      return ParseReturn(); 
    }

    if (Match({ IF_KW })) {
      return ParseIf();
    }

    if (Match({ FOR_KW })) {
      return ParseFor();
    }

    return ParseStatement();
  }
      
  Ref<AstStmt> ShaderParser::ParseShaderDecl() {
    std::vector<Ref<AstExpr>> attributes;

    if (Match({ OPEN_BRACKET })) {
      while (!Match({ CLOSE_BRACKET })) {
        attributes.push_back(ParseShaderAttribute());
        if (Match({ COMMA })) {
          /// just consume, handle trailing commas
        }
      }
    }

    Consume(OPEN_BRACE , "Expected opening brace for shader declaration"); 
    std::vector<Ref<AstStmt>> stmts;

    while (!Check(CLOSE_BRACE) && !AtEnd()) {
      Ref<AstStmt> stmt = ParseDecl();
      if (stmt != nullptr) {
        stmts.push_back(stmt);
      } else {
        throw Error(SYNTAX_ERROR , "Failed to parse block");
      }
      if (Match({ SEMICOLON })) {
        /// do nothing just consume
      }
    }

    Consume(CLOSE_BRACE , "Expected closing brace to close shader declaration");

    return NewRef<ShaderDecl>(GetCurrentShaderType() , attributes , stmts);
  }

  Ref<AstStmt> ShaderParser::ParseLayoutDecl() {
    Consume(OPEN_PAREN , fmtstr("Expected open parathesis after 'layout'! found {}" , Peek().value));

    /// fix this so that the order of the layout descriptors doesn't matter

    Token rules = Token({} , INVALID_TOKEN , "");
    std::vector<Ref<AstExpr>> descriptors;
    if (Match({ STDXXX_KW , TRIANGLES_KW , LINE_STRIP_KW })) {
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
    std::vector<FunctionParam> params;
    if (Match({ OPEN_PAREN })) {
      while (!Check(CLOSE_PAREN) && !AtEnd()) {
        Token type = ConsumeType(fmtstr("Expected type for function parameter! found {}" , Peek().value));
        Token param = Consume(IDENTIFIER , fmtstr("Expected identifier for function parameter! found {}" , Peek().value));
        params.push_back({ type , param });
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

  Ref<AstStmt> ShaderParser::ParseUniformDecl() {
    flags.initializer_blocker.push(true);
    Ref<AstStmt> var = ParseVarDecl();
    flags.initializer_blocker.pop();

    return NewRef<UniformDecl>(var);  
  }

  Ref<AstStmt> ShaderParser::ParseVarDecl() {
    bool is_const = false;
    if (Match({ CONST_KW })) {
      is_const = true;
    }

    Token type = ConsumeType(fmtstr("Expected type keyword! found {}" , Peek().value));
    Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier! found {}" , Peek().value));

    /// if initializers are not valid then functions and function calls are not either
    Ref<AstStmt> initializer;
    if (flags.initializer_blocker.empty()) {
      if (Match({ OPEN_PAREN } , false)) {
        return ParseFunctionDecl(type , identifier);
      } else if (Match({ EQUAL_OP })) {
        initializer = ParseStatement();
      }
    } else if (Match({ OPEN_BRACKET })) {
      return ParseArrayDecl(type , identifier , is_const);
    } 

    bool is_global = false;

    /// if declaring variable at global scope, have to consume semicolon
    if (flags.scoping.empty()) {
      is_global = true;
    }

    return NewRef<VarDecl>(type , identifier , initializer , is_const , is_global); 
  }

  Ref<AstStmt> ShaderParser::ParseArrayDecl(const Token& type , const Token& name , bool is_const) { 
    Token size = Token({} , INT_LIT , "");
    if (Match({ INT_LIT })) {
      size = Consume(INT_LIT , fmtstr("Expected integer literal for array size! found {}" , Peek().type));
    } 
    Consume(CLOSE_BRACKET , fmtstr("Expected close bracket after array size! found {}" , Peek().type));

    Ref<AstExpr> initializer = nullptr;
    if (flags.initializer_blocker.empty()) {
      if (Match({ EQUAL_OP })) {
        /// do nothing
      }

      initializer = ParseArrayExpr(size);
    } 

    return NewRef<ArrayDecl>(type , name , size , initializer , is_const);
  }
      
  Ref<AstStmt> ShaderParser::ParseBufferDecl() {
    Token buff_type = Token({} , INVALID_TOKEN , "");
    if (Match({ READONLY_KW })) {
      Token readonly = Previous();

      buff_type = Consume(BUFFER_KW , fmtstr("Can only make Shader Storage Buffers read only! found {}" , Peek().value));
      buff_type.value = readonly.value + " " + buff_type.value;
    } else {
      if (Match({ UNIFORM_KW , BUFFER_KW } , false)) {
        buff_type = Advance();
      } else {
        throw Error(SYNTAX_ERROR , fmtstr("Expected 'uniform' or 'buffer'! found {}" , Peek().value));
      }
    }

    Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier to name Shader Storage! found {}" , Peek().value));

    Ref<AstStmt> body = ParseBlock();
    // Consume(SEMICOLON , fmtstr("Must close shader storage block with ';'! found {}" , Peek().value));

    return NewRef<ShaderStorageStmt>(buff_type , identifier , body);
  }

  Ref<AstStmt> ShaderParser::ParseLayoutStmt() {
    if (Match({ IN_KW , OUT_KW })) {
      Token in_out = Previous();
      if (Match({ SEMICOLON })) {
        return NewRef<LayoutVarDecl>(in_out); 
      }

      Token type = ConsumeType(fmtstr("Expected type keyword after {}! found {}" , Previous().value , Peek().value));
      Token identifier = Consume(IDENTIFIER , fmtstr("Expected identifier! found {}" , Peek().value));

      return NewRef<LayoutVarDecl>(in_out , type , identifier);
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
    return stmt;
  }

  Ref<AstStmt> ShaderParser::ParseInOutBlock(const Token& type , const Token& tag) {
    bool is_array = false;

    flags.initializer_blocker.push(true);
    Ref<AstStmt> body = ParseBlock();
    flags.initializer_blocker.pop();

    Token name = Consume(IDENTIFIER , fmtstr("Expected identifier for {} block {}! found {}" , type.value , tag.value , Peek().value));

    if (Match({ OPEN_BRACKET })) {
      is_array = true;

      Consume(CLOSE_BRACKET , fmtstr("Unterminated array {} block found! Cant not define length of in/out block arrays! found {}" ,
                                      type.value , Peek().value));
    }
    
    // Consume(SEMICOLON , fmtstr("Expected semicolon after {} block {}! found {}" , type.value , tag.value , Peek().value));

    return NewRef<InOutBlockStmt>(type , tag , name , body , is_array);
  }

  Ref<AstStmt> ShaderParser::ParseBlock() {
    Consume(OPEN_BRACE , "Expected opening brace for function declaration"); 
    std::vector<Ref<AstStmt>> stmts;

    flags.scoping.push(true);

    while (!Check(CLOSE_BRACE) && !AtEnd()) {
      Ref<AstStmt> stmt = ParseDecl();
      if (stmt != nullptr) {
        stmts.push_back(stmt);
      } else {
        throw Error(SYNTAX_ERROR , "Failed to parse block");
      }
      if (Match({ SEMICOLON })) {
        /// do nothing just consume
      }
      // Consume(SEMICOLON , fmtstr("Expected ';' after statement in block! found {}" , Peek().value));
    }

    Consume(CLOSE_BRACE , "Expected closing brace");
    
    flags.scoping.pop();

    return NewRef<BlockStmt>(stmts);
  }

  Ref<AstStmt> ShaderParser::ParseIf() { 
    Consume(OPEN_PAREN , fmtstr("Expected '(' after 'if'! found {}" , Peek().value));

    Ref<AstExpr> expr = ParseExpression();
    
    Consume(CLOSE_PAREN , fmtstr("Expected ')' after if condition! found {}" , Peek().value));

    Ref<AstStmt> then_stmt = ParseBlock();

    Ref<AstStmt> else_stmt = nullptr;
    if (Match({ ELSE_KW })) {
      else_stmt = ParseDecl();
    }

    return NewRef<IfStmt>(expr , then_stmt , else_stmt);
  }

  Ref<AstStmt> ShaderParser::ParseWhile() { return nullptr; }

  Ref<AstStmt> ShaderParser::ParseFor() { 
    Consume(TokenType::OPEN_PAREN , "Expected opening parenthesis after 'for' keyword");

    Ref<AstStmt> initializer = nullptr;
    if (Match({ TokenType::SEMICOLON })) {
      // do nothing
    } else if (Match({ TokenType::IDENTIFIER } , false)) {
      initializer = ParseDecl();
    } else {
      Ref<AstExpr> expr = ParseExpression();
      initializer = NewRef<ExprStmt>(expr);
      Consume(TokenType::SEMICOLON , "Expected semicolon after initializer");
    }

    Ref<AstExpr> condition = nullptr;
    if (!Check(TokenType::SEMICOLON)) {
      condition = ParseExpression();
    }
    Consume(TokenType::SEMICOLON , "Expected semicolon after loop condition");

    Ref<AstExpr> increment = nullptr;
    if (!Check(TokenType::CLOSE_PAREN)) {
      increment = ParseExpression();
    }
    Consume(TokenType::CLOSE_PAREN , "Expected closing parenthesis after for loop");

    Ref<AstStmt> body = ParseBlock();

    if (increment != nullptr) {
      std::vector<Ref<AstStmt>> stmts{ body , NewRef<ExprStmt>(increment) };
      body = NewRef<BlockStmt>(stmts);
    }

    if (condition == nullptr) {
      condition = NewRef<LiteralExpr>(Token(Peek().location , TokenType::TRUE_KW , "true"));
    }

    body = NewRef<WhileStmt>(condition , body);

    if (initializer != nullptr) {
      std::vector<Ref<AstStmt>> stmts{ initializer , body };
      body = NewRef<BlockStmt>(stmts);
    }

    return body;
  }

  Ref<AstStmt> ShaderParser::ParseReturn() {
    if (Match({ SEMICOLON })) {
      return NewRef<ReturnStmt>();
    }

    Ref<ExprStmt> expr = NewRef<ExprStmt>(ParseExpression());
    Consume(SEMICOLON , fmtstr("Expected ';' after return statement! found {}" , Peek().value));

    return NewRef<ReturnStmt>(expr);
  }
      
  Ref<AstExpr> ShaderParser::ParseShaderAttribute() {
    if (Match({ MESH_KW })) {
      Token type = Previous();
      Consume(COLON , fmtstr("Expected ':' to define shader attribute! found {}" , Peek().value));
      Token value = Consume(IDENTIFIER , fmtstr("Expected identifier to define shader attribute! found {}" , Peek().value));
      return NewRef<ShaderAttribute>(type , value); 
    }

    throw Error(SYNTAX_ERROR , "Unknown shader attribute! found {}" , Peek().value);
  }
      
  Ref<AstExpr> ShaderParser::ParseLayoutDescriptor() {
    if (Match({ BINDING_KW , LOCATION_KW , MAX_VERTICES_KW })) {
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

    if (Match({ EQUAL_OP })) {
      /// Token('=') == Previous()
      Ref<AstExpr> right = ParseAssignment();
      if (expr->GetType() == VAR_EXPR) {
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

    /// here we assume ctor call 
    if (MatchTypes()) {
      Ref<AstExpr> var_expr = NewRef<VarExpr>(Previous());
      
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

    // Consume(SEMICOLON , fmtstr("Expected closing bracket for array literal! found {}" , Peek().type));

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
    }

    if (Match({ OPEN_BRACKET })) {
      index = ParseExpression();
      Consume(CLOSE_BRACKET , "Expected closing bracket for struct array access");

      assignment = nullptr;
      if (Match({ EQUAL_OP })) {
        assignment = ParseExpression();
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
      case SAMPLER2D_KW:
        return Advance();
      default:
        if (Peek().value == "DirectionLight" || Peek().value == "PointLight") {
          return Advance();
        }
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
      SAMPLER2D_KW ,
    }, advance);
  }

  void ShaderParser::AddTopLevelNode(const Ref<AstStmt>& node) {
    switch (current_context) {
      case VERTEX_CTX:
        result.vertex_nodes.push_back(node);
      break;
      case FRAGMENT_CTX:
        result.fragment_nodes.push_back(node);
      break;
      case GEOMETRY_CTX:
        result.geometry_nodes.push_back(node);
      break;
      default:
        throw Error(INVALID_SHADER_CTX , "Parser state corrupted");
    }
  }

} // namespace other
