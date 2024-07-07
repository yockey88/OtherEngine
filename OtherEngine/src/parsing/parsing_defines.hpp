/**
 * \file parsing/parsing_defines.hpp
 **/
#ifndef OTHER_ENGINE_PARSING_DEFINES_HPP
#define OTHER_ENGINE_PARSING_DEFINES_HPP

#include <string_view>
#include <array>
#include <map>

#include "core/defines.hpp"

namespace other {

  enum AstNodeType {
    LITERAL_EXPR ,
    UNARY_EXPR , 
    BINARY_EXPR ,
    CALL_EXPR , 
    GROUPING_EXPR , 
    VAR_EXPR ,
    ASSIGN_EXPR , 
    ARRAY_EXPR , 
    ARRAY_ACCESS_EXPR , 
    OBJ_ACCESS_EXPR ,

    EXPR_STMT ,
    VAR_DECL_STMT , 
    ARRAY_DECL_STMT ,
    BLOCK_STMT ,
    IF_STMT , 
    WHILE_STMT , 
    RETURN_STMT , 
    FUNCTION_STMT , 
    STRUCT_STMT ,
    
    /// shader exprs
    LAYOUT_DESCRIPTOR ,
    VERSION_EXPR ,

    /// shader stmts
    LAYOUT_DECL_STMT ,
    LAYOUT_STMT ,
    SHADER_STORAGE_STMT ,
    IN_OUT_BLOCK ,

    NUM_AST_NODES ,
    INVALID_AST_NODES = NUM_AST_NODES ,
  };
  
  enum TokenType {
    START_SRC = 0 ,

    // identifiers
    IDENTIFIER ,
    
    // literals 
    INT_LIT , FLOAT_LIT,

    // operators
    OPEN_PAREN,
    CLOSE_PAREN,
    OPEN_BRACE,
    CLOSE_BRACE,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    PLUS,
    MINUS,
    STAR,
    F_SLASH,
    EQUAL_OP ,
    EQUAL_EQUAL ,
    GREATER_OP ,
    GREATER_EQUAL_OP ,
    LESS_OP ,
    LESS_EQUAL_OP ,
    BANG,
    BANG_EQUAL,
    LOGICAL_AND,
    LOGICAL_OR,
    SEMICOLON,
    COLON,
    COMMA,
    QUOTE,
    DQUOTE,
    DOT,
    HASH ,

    /// keywords
    VOID_KW , 
    IF_KW ,
    WHILE_KW ,
    FOR_KW ,
    RETURN_KW , 
    STRUCT_KW ,

    CORE_KW ,
    LAYOUT_KW , 
    LOCATION_KW , 
    STDXXX_KW , 
    BINDING_KW , 
    UNIFORM_KW , 
    READONLY_KW , 
    BUFFER_KW ,

    IN_KW ,
    OUT_KW ,
    // sfloat for shader float, becuase ValueType has A FLOAT
    INT_KW , 
    FLOAT_KW , 
    VEC2_KW , 
    VEC3_KW , 
    VEC4_KW , 
    MAT2_KW , 
    MAT3_KW , 
    MAT4_KW ,

    END_SRC ,

    NUM_TOKENS , 
    INVALID_TOKEN = NUM_TOKENS ,
  };

  constexpr static uint32_t kNumAstTypes = NUM_AST_NODES + 1;
  constexpr static std::array<std::string_view , kNumAstTypes> kAstNodeStrings = {
    "LITERAL_EXPR" ,
    "UNARY_EXPR" , 
    "BINARY_EXPR" ,
    "CALL_EXPR" , 
    "GROUPING_EXPR" , 
    "VAR_EXPR" ,
    "ASSIGN_EXPR" , 
    "ARRAY_EXPR" , 
    "ARRAY_ACCESS_EXPR" , 
    "OBJ_ACCESS_EXPR" ,
    
    "EXPR_STMT" ,
    "VAR_DECL_STMNT" , 
    "ARRAY_DECL_STMNT" ,
    "BLOCK_STMT" ,
    "IF_STMT" , 
    "WHILE_STMT" , 
    "RETURN_STMT" , 
    "FUNCTION_STMT" , 
    "STRUCT_STMT" ,
    
    "LAYOUT_DESCRIPTOR" ,
    "VERSION_EXPR" ,

    "LAYOUT_DECL_STMT" ,
    "LAYOUT_STMT" ,

    "SHADER_STORAGE_STMT" ,
    
    "IN_OUT_BLOCK" ,
    
    "INVALID_STMT" ,
  };
  
  constexpr static uint32_t kNumTokens = NUM_TOKENS + 1;
  constexpr static std::array<std::string_view , kNumTokens> kTokenStrings = {
    "START_SRC" ,

    "IDENTIFIER" ,

    "INT_LIT" , 
    "FLOAT_LIT" ,

    "OPEN_BRACE" ,
    "CLOSE_BRACE" ,
    "OPEN_PAREN" ,
    "CLOSE_PAREN" ,
    "OPEN_BRACKET" ,
    "CLOSE_BRACKET" ,
    "PLUS" ,
    "MINUS" ,
    "STAR" ,
    "F_SLASH" ,
    "EQUAL" ,
    "EQUAL_EQUAL" ,
    "LESS" ,
    "LESS_EQUAL" ,
    "GREATER" ,
    "GREATER_EQUAL" ,
    "BANG" ,
    "BANG_EQUAL" ,
    "LOGICAL_AND" ,
    "LOGICAL_OR" ,
    "SEMICOLON" ,
    "COLON" ,
    "COMMA" ,
    "DOT" ,
    "HASH" ,

    "VOID " , 
    "IF" ,
    "WHILE" ,
    "FOR" ,
    "RETURN" , 
    "STRUCT" ,

    "CORE" ,
    "LAYOUT" , 
    "LOCATION" , 
    "STDXXX" , 
    "BINDING" ,
    "UNIFORM" , 
    "READONLY" , 
    "BUFFER" , 
    "IN_KW" ,
    "OUT_KW" ,

    "INT" , 
    "FLOAT" , 
    "VEC2" , 
    "VEC3" , 
    "VEC4" ,
    "MAT2" , 
    "MAT3" , 
    "MAT4" , 
    
    "END_SRC" ,

    "INVALID_KEYWORD" ,
  };

  constexpr static uint32_t kNumKeywords = END_SRC - VOID_KW;
  constexpr static uint32_t kNumOperators = HASH - OPEN_BRACE - 3;
  const static std::map<uint64_t , TokenType> kKeywordMap {
    { FNV("void")     , VOID_KW     } , 
    { FNV("if")       , IF_KW       } ,
    { FNV("while")    , WHILE_KW    } ,
    { FNV("for")      , FOR_KW      } ,
    { FNV("return")   , RETURN_KW   } ,
    { FNV("struct")   , STRUCT_KW   } ,

    { FNV("core")     , CORE_KW     } ,
    { FNV("layout")   , LAYOUT_KW   } ,
    { FNV("location") , LOCATION_KW } ,
    { FNV("std")      , STDXXX_KW   } ,
    { FNV("binding")  , BINDING_KW  } ,
    { FNV("uniform")  , UNIFORM_KW  } ,
    { FNV("readonly") , READONLY_KW } ,
    { FNV("buffer")   , BUFFER_KW   } ,
    { FNV("in")       , IN_KW       } ,
    { FNV("out")      , OUT_KW      } ,

    { FNV("int")      , INT_KW      } ,
    { FNV("float")    , FLOAT_KW    } ,
    { FNV("vec2")     , VEC2_KW     } ,
    { FNV("vec3")     , VEC3_KW     } ,
    { FNV("vec4")     , VEC4_KW     } ,
    { FNV("mat2")     , MAT2_KW     } ,
    { FNV("mat3")     , MAT3_KW     } ,
    { FNV("mat4")     , MAT4_KW     } ,
  };  

  constexpr std::array<char, kNumOperators> kOperators = {
    '{' , '}' , 
    '(' , ')' , 
    '[' , ']' , 
    '+', '-', '*', '/' , 
    '=' , '<' , '>' , '!' ,
    '&', '|', 
    ';', ':' , ',' , '.' , 
    '#' ,
  };

  struct SourceLocation {
    uint32_t index = 0;
    uint32_t line = 1;
    uint32_t column = 1;
  };

  struct Token {
    SourceLocation location;
    std::string value;
    uint64_t hash;
    TokenType type;

    Token(SourceLocation loc , TokenType type , const std::string& value)
      : location(loc) , value(value) , hash(FNV(value)) , type(type) {}
  };  

  static inline bool IsNumeric(char c) {
    return c >= '0' && c <= '9';
  }

  static inline bool IsAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }

  static inline bool IsAlphaNumeric(char c) {
    return IsAlpha(c) || IsNumeric(c);
  }

  static inline bool IsWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
  }

  static inline bool IsKeyword(const std::string& str) {
    return kKeywordMap.find(FNV(str)) != kKeywordMap.end();
  }

  static inline bool IsOperator(char c) {
    return std::find(kOperators.begin() , kOperators.end() , c) != kOperators.end();
  }

} // namespace other

#endif // !OTHER_ENGINE_PARSING_DEFINES_HPP
