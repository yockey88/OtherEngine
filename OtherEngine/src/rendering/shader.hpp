#ifndef OTHER_ENGINE_SHADER_HPP
#define OTHER_ENGINE_SHADER_HPP

#include <cstdint>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "core/defines.hpp"
#include "asset/asset.hpp"

namespace other {

  enum ShaderType {
    VERTEX_SHADER = GL_VERTEX_SHADER ,
    FRAGMENT_SHADER = GL_FRAGMENT_SHADER ,
    GEOMETRY_SHADER = GL_GEOMETRY_SHADER ,
    TESS_CONTROL_SHADER = GL_TESS_CONTROL_SHADER ,
    TESS_EVALUATION_SHADER = GL_TESS_EVALUATION_SHADER ,
    COMPUTE_SHADER = GL_COMPUTE_SHADER ,
  };

  class Shader : public Asset {    
    public:
      OE_ASSET(SHADER);

      Shader(const std::vector<std::string>& src);
      Shader(const Path& vertex_path , const Path& fragment_path , const Opt<Path>& geometry_shader = std::nullopt);
      virtual ~Shader() override;
  
      void Bind() const;
      void Unbind() const;
  
      const bool IsValid() const;

      const bool HasGeometry() const;

      void BindToBlock(const std::string& name , uint32_t binding_point);
 
      void SetUniform(const std::string& name , const int32_t& value);
      void SetUniform(const std::string& name , const float& value);
      void SetUniform(const std::string& name , const glm::vec2& value);
      void SetUniform(const std::string& name , const glm::vec3& value);
      void SetUniform(const std::string& name , const glm::vec4& value);
      void SetUniform(const std::string& name , const glm::mat2& value);
      void SetUniform(const std::string& name , const glm::mat3& value);
      void SetUniform(const std::string& name , const glm::mat4& value);

    private:
      uint32_t renderer_id;
  
      Path vertex_path;
      Path fragment_path;
      Opt<Path> geometry_path;
  
      std::string vertex_src;
      std::string fragment_src;
      Opt<std::string> geometry_src;
  
      std::unordered_map<std::string , int32_t> uniform_locations;
  
      bool valid = false;

      bool Compile(const char* vsrc , const char* fsrc , const char* gsrc = nullptr);
  
      bool CompileShader(uint32_t shader_piece , const char* src);
      bool LinkShader();
  
      uint32_t GetUniformLocation(const std::string& name);
  };

} // namespace other 

#endif // !OTHER_ENGINE_SHADER_HPP
