/**
 * \file parsing/shader_preprocessor.hpp
 **/
#ifndef OTHER_ENGINE_SHADER_PREPROCESSOR_HPP
#define OTHER_ENGINE_SHADER_PREPROCESSOR_HPP

#include <string>

#include "core/errors.hpp"

#include "parsing/parsing_defines.hpp"

namespace other {
  
  struct ProcessedFile {
    std::string src = "";
    std::vector<std::string> imports{};
  };

  class ShaderPreprocessor {
    public:
      ShaderPreprocessor(const std::string& src)
        : raw_src(src) {}

      ProcessedFile Process();

    private:
      SourceLocation loc;
      std::string raw_src;

      std::string current_command;

      ProcessedFile result;
    
      void Consume();
      void ConsumeCommand();
      void Add();
      void Add(char c);
      void Add(const std::string& s);
    
      bool Command();
      bool CanContinue();
      bool AtEnd();
      bool Check(char c);
      bool CheckNext(char c);
      char Peek();

      void Version();

      void FinishCommand(const std::string& expected);
    
      void FinishImport(const std::string& rest_of_str);
      void ReadImport();
    
      void If();
      
      template <typename... Args>
      ShaderException Error(ShaderError error , std::string_view message, Args&&... args) const {
        return ShaderException(fmtstr(message , std::forward<Args>(args)...) , error);
      }
  };

} // namespace other

#endif // !OTHER_ENGINE_SHADER_PREPROCESSOR_HPP

