/**
 * \file parsing/parsing_defines.hpp
 **/
#ifndef OTHER_ENGINE_PARSING_DEFINES_HPP
#define OTHER_ENGINE_PARSING_DEFINES_HPP

#include <string_view>
#include <array>
#include <algorithm>
#include <utility>

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
    
    /// glsl shader exprs
    VERSION_EXPR ,
    SHADER_ATTRIBUTE_EXPR ,
    LAYOUT_DESCRIPTOR ,

    /// glsl shader stmts
    LAYOUT_DECL_STMT ,
    LAYOUT_VAR_DECL_STMT ,
    SHADER_STORAGE_STMT ,
    IN_OUT_BLOCK_STMT,
    UNIFORM_DECL_STMT ,

    /// oshader exprs
    

    /// oshader stmts
    SHADER_DECL_STMT ,

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
    PLUS_EQUAL ,
    MINUS,
    MINUS_EQUAL ,
    STAR,
    STAR_EQUAL ,
    F_SLASH,
    F_SLASH_EQUAL ,
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
    ELSE_KW ,
    WHILE_KW ,
    FOR_KW ,
    RETURN_KW , 
    STRUCT_KW ,
    CONST_KW ,

    CORE_KW ,
    LAYOUT_KW , 
    LOCATION_KW , 
    STDXXX_KW , 
    TRIANGLES_KW ,
    LINE_STRIP_KW ,
    MAX_VERTICES_KW ,
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
    SAMPLER2D_KW ,

    /// oshader keywords
    VERTEX_KW ,
    FRAGMENT_KW ,
    GEOMETRY_KW ,
    
    MESH_KW ,

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
    
    "VERSION_EXPR" ,
    "SHADER_ATTRIBUTE_EXPR"
    "LAYOUT_DESCRIPTOR" ,

    "LAYOUT_DECL_STMT" ,
    "LAYOUT_VAR_DECL_STMT" ,
    "SHADER_STORAGE_STMT" ,
    "IN_OUT_BLOCK_STMT" ,
    
    "SHADER_DECL_STMT" ,
    
    "INVALID_STMT" ,
  };
  
  constexpr static uint32_t kNumTokens = NUM_TOKENS + 1;
  constexpr static std::array<std::string_view , kNumTokens> kTokenStrings = {
    "START_SRC = 0" ,

    // identifiers
    "IDENTIFIER" ,

    // literals 
    "INT_LIT" , 
    "FLOAT_LIT" ,

    // operators
    "OPEN_PAREN" ,
    "CLOSE_PAREN" ,
    "OPEN_BRACE" ,
    "CLOSE_BRACE" ,
    "OPEN_BRACKET" ,
    "CLOSE_BRACKET" ,
    "PLUS" ,
    "PLUS_EQUAL" ,
    "MINUS" ,
    "MINUS_EQUAL" ,
    "STAR" ,
    "STAR_EQUAL" ,
    "F_SLASH" ,
    "F_SLASH_EQUAL" ,
    "EQUAL_OP" ,
    "EQUAL_EQUAL" ,
    "GREATER_OP" ,
    "GREATER_EQUAL_OP" ,
    "LESS_OP" ,
    "LESS_EQUAL_OP" ,
    "BANG" ,
    "BANG_EQUAL" ,
    "LOGICAL_AND" ,
    "LOGICAL_OR" ,
    "SEMICOLON" ,
    "COLON" ,
    "COMMA" ,
    "QUOTE" ,
    "DQUOTE" ,
    "DOT" ,
    "HASH" ,

    "VOID_KW" , 
    "IF_KW" ,
    "ELSE_KW" ,
    "WHILE_KW" ,
    "FOR_KW" ,
    "RETURN_KW" , 
    "STRUCT_KW" ,
    "CONST_KW" ,

    "CORE_KW" ,
    "LAYOUT_KW" , 
    "LOCATION_KW" , 
    "STDXXX_KW" , 
    "TRIANGLES_KW" ,
    "LINE_STRIP_KW" ,
    "MAX_VERTICES_KW" ,
    "BINDING_KW" , 
    "UNIFORM_KW" , 
    "READONLY_KW" , 
    "BUFFER_KW" ,

    "IN_KW" ,
    "OUT_KW" ,

    "INT_KW" , 
    "FLOAT_KW" , 
    "VEC2_KW" , 
    "VEC3_KW" , 
    "VEC4_KW" , 
    "MAT2_KW" , 
    "MAT3_KW" , 
    "MAT4_KW" ,
    "SAMPLER2D_KW" ,

    /// oshader keywords
    "VERTEX_KW" ,
    "FRAGMENT_KW" ,
    "GEOMETRY_KW" ,

    "MESH_KW" ,

    "END_SRC" ,

    "INVALID_KEYWORD" ,
  };

  constexpr static uint32_t kNumKeywords = END_SRC - VOID_KW;
  constexpr static uint32_t kNumOperators = HASH - OPEN_BRACE - 3;

  using KeyWordPair = std::pair<uint64_t , TokenType>;

  constexpr static std::array<KeyWordPair , kNumKeywords> kKeywordMap {
    KeyWordPair{ FNV("void")         , VOID_KW         } , 
    KeyWordPair{ FNV("if")           , IF_KW           } ,
    KeyWordPair{ FNV("else")         , ELSE_KW         } ,
    KeyWordPair{ FNV("while")        , WHILE_KW        } ,
    KeyWordPair{ FNV("for")          , FOR_KW          } ,
    KeyWordPair{ FNV("return")       , RETURN_KW       } ,
    KeyWordPair{ FNV("struct")       , STRUCT_KW       } ,
    KeyWordPair{ FNV("const")        , CONST_KW        } ,

    KeyWordPair{ FNV("core")         , CORE_KW         } ,
    KeyWordPair{ FNV("layout")       , LAYOUT_KW       } ,
    KeyWordPair{ FNV("location")     , LOCATION_KW     } ,
    KeyWordPair{ FNV("std")          , STDXXX_KW       } ,
    KeyWordPair{ FNV("triangles")    , TRIANGLES_KW    } , 
    KeyWordPair{ FNV("line_strip")   , LINE_STRIP_KW   } ,
    KeyWordPair{ FNV("max_vertices") , MAX_VERTICES_KW },
    KeyWordPair{ FNV("binding")      , BINDING_KW      } ,
    KeyWordPair{ FNV("uniform")      , UNIFORM_KW      } ,
    KeyWordPair{ FNV("readonly")     , READONLY_KW     } ,
    KeyWordPair{ FNV("buffer")       , BUFFER_KW       } ,
    KeyWordPair{ FNV("in")           , IN_KW           } ,
    KeyWordPair{ FNV("out")          , OUT_KW          } ,

    KeyWordPair{ FNV("int")          , INT_KW          } ,
    KeyWordPair{ FNV("float")        , FLOAT_KW        } ,
    KeyWordPair{ FNV("vec2")         , VEC2_KW         } ,
    KeyWordPair{ FNV("vec3")         , VEC3_KW         } ,
    KeyWordPair{ FNV("vec4")         , VEC4_KW         } ,
    KeyWordPair{ FNV("mat2")         , MAT2_KW         } ,
    KeyWordPair{ FNV("mat3")         , MAT3_KW         } ,
    KeyWordPair{ FNV("mat4")         , MAT4_KW         } ,
    KeyWordPair{ FNV("sampler2D")    , SAMPLER2D_KW    } ,

    KeyWordPair{ FNV("vertex")       , VERTEX_KW       } ,
    KeyWordPair{ FNV("fragment")     , FRAGMENT_KW     } ,
    KeyWordPair{ FNV("geometry")     , GEOMETRY_KW     } ,

    KeyWordPair{ FNV("mesh")         , MESH_KW         } ,
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
    return std::find_if(kKeywordMap.begin() , kKeywordMap.end() , [&str](const KeyWordPair& kw) -> bool { 
      return FNV(str) == kw.first;
    }) != kKeywordMap.end();
  }

  static inline bool IsOperator(char c) {
    return std::find(kOperators.begin() , kOperators.end() , c) != kOperators.end();
  }

} // namespace other

#endif // !OTHER_ENGINE_PARSING_DEFINES_HPP
