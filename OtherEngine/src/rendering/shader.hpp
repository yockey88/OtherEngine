#ifndef OTHER_ENGINE_SHADER_HPP
#define OTHER_ENGINE_SHADER_HPP

#include <cstdint>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "core/defines.hpp"
#include "asset/asset.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/uniform.hpp"

namespace other {
    
  struct ShaderIr {
    MeshLayout layout;

    std::map<UUID , ShaderStorage> storages;
    std::map<UUID , Uniform> uniforms;

    std::string vert_source = ""; 
    std::string frag_source = "";
    Opt<std::string> geom_source = std::nullopt;
  };

  class Shader : public Asset {    
    public:
      OE_ASSET(SHADER);

      Shader(const ShaderIr& src);
      virtual ~Shader() override;

      uint32_t ID() const;
  
      void Bind() const;
      void Unbind() const;
  
      const bool IsValid() const;

      const bool HasGeometry() const;
    
      template <typename T> 
      void SetUniform(const std::string_view name , T&& value , uint32_t index = 0) {
        Bind();
        InnerSetUniform(name , std::forward<T>(value) , index); 
      }

    private:
      uint32_t renderer_id;

      ShaderIr ir;
  
      std::unordered_map<UUID , int32_t> uniform_locations;
  
      bool valid = false;

      bool Compile(const char* vsrc , const char* fsrc , const char* gsrc = nullptr);
  
      bool CompileShader(ShaderType type , uint32_t shader_piece , const char* src);
      bool LinkShader();
  
      uint32_t GetUniformLocation(const std::string_view name);
      
      void InnerSetUniform(const std::string_view name , const int32_t& value , uint32_t index = 0);
      void InnerSetUniform(const std::string_view name , const float& value , uint32_t index = 0);
      void InnerSetUniform(const std::string_view name , const glm::vec2& value , uint32_t index = 0);
      void InnerSetUniform(const std::string_view name , const glm::vec3& value , uint32_t index = 0);
      void InnerSetUniform(const std::string_view name , const glm::vec4& value , uint32_t index = 0);
      void InnerSetUniform(const std::string_view name , const glm::mat2& value , uint32_t index = 0);
      void InnerSetUniform(const std::string_view name , const glm::mat3& value , uint32_t index = 0);
      void InnerSetUniform(const std::string_view name , const glm::mat4& value , uint32_t index = 0);
  };

  Ref<Shader> BuildShader(const Path& path);
  Ref<Shader> BuildShader(const Path& vpath , const Path& fpath , const Opt<Path>& gpath = std::nullopt);

} // namespace other 

#endif // !OTHER_ENGINE_SHADER_HPP
