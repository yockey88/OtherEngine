/**
 * \file parsing/shader_preprocessor.cpp
 **/
#include "parsing/shader_preprocessor.hpp"
#include "parsing/parsing_defines.hpp"


namespace other {

 ProcessedFile ShaderPreprocessor::Process() {
  while (CanContinue()) {
    if (Command()) {
      ConsumeCommand();
      continue;
    }

    Add();
  }

  return result;
 }

 void ShaderPreprocessor::Consume() {
    loc.index++;
  }
  
  void ShaderPreprocessor::ConsumeCommand() {
    // consume '#'
    Consume();
    
    if (Check('i')) {
      current_command += "i";
      Consume();

      if (Check('n')) { FinishImport("clude"); }
      if (Check('m')) { FinishImport("port"); }
      if (Check('f')) { If(); } 

      Consume();
    }

    if (Check('v')) {
      current_command += "v";
      Consume();
      
      if (Check('e')) { Version(); }
    }
  }

  void ShaderPreprocessor::Add() {
    result.src += raw_src[loc.index];
    Consume();
  }
  
  void ShaderPreprocessor::Add(char c) {
    result.src += c;
  }
  
  void ShaderPreprocessor::Add(const std::string& s) {
    for (size_t i = 0; i < s.size(); i++) {
      Add(s[i]);
    }
  }

  bool ShaderPreprocessor::Command() {
    return Check('#');
  }

  bool ShaderPreprocessor::CanContinue() {
    return loc.index < raw_src.size();
  }
  
  bool ShaderPreprocessor::AtEnd() {
    return loc.index >= raw_src.size();
  }

  bool ShaderPreprocessor::Check(char c) {
    if (AtEnd()) {
      return false;
    }
    return raw_src[loc.index] == c;
  }

  bool ShaderPreprocessor::CheckNext(char c) {
    if (AtEnd() || loc.index + 1 >= raw_src.size()) {
      return false;
    }
    return raw_src[loc.index + 1] == c;
  }

  char ShaderPreprocessor::Peek() {
    if (AtEnd()) {
      return '\0';
    }
    return raw_src[loc.index];
  }

  void ShaderPreprocessor::Version() {
    while (!IsWhitespace(raw_src[loc.index])) {
      Consume();
    } 

    result.src += "#version";
  }
      
  void ShaderPreprocessor::FinishCommand(const std::string& expected) {
    for (size_t i = 0; i < expected.size(); i++) {
      if (!Check(expected[i])) {
        while (!IsWhitespace(Peek())) {
          current_command += Peek();
          Consume();
        }
        throw Error(INVALID_SHADER_DIRECTIVE , fmtstr("Unknown preprocessor directive {}" , current_command));
      }
      current_command += Peek();
      Consume();
    }

    current_command.clear();
  }
  
  void ShaderPreprocessor::FinishImport(const std::string& rest_of_str) {
    current_command += Peek();
    Consume();
    
    FinishCommand(rest_of_str);


    while (IsWhitespace(raw_src[loc.index])) {
      Consume();
    }
    
    ReadImport();
  }

  void ShaderPreprocessor::ReadImport() {
    std::string path; 
    
    char c = ';';
    if (Check('<') || Check('"')) {
      c = raw_src[loc.index];
      c = (c == '<') ? 
          '>' : '"';
      Consume();
    }

    std::string import;
    while (!Check(c)) {
      import += raw_src[loc.index];
      Consume();
    }

    Consume();
    if (c == '<' || c == '"') {
      if (!Check(';')) {
        throw Error(SYNTAX_ERROR , "Expected ';' after import");
      }
      // semicolon after import
      Consume();
    }

    result.imports.push_back(import);
  }

  void ShaderPreprocessor::If() {
    current_command += Peek();
    Consume();

    while (IsWhitespace(raw_src[loc.index])) {
      Consume();
    }

    std::string condition;
    while (!IsWhitespace(Peek())) {
      condition += Peek();
      Consume();
    }
  }      

} // namespace other
