#ifndef OTHER_ENGINE_SHADER_HPP
#define OTHER_ENGINE_SHADER_HPP

#include <cstdint>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include <glad/glad.h>

namespace other {

    enum ShaderType {
        VERTEX_SHADER = GL_VERTEX_SHADER ,
        FRAGMENT_SHADER = GL_FRAGMENT_SHADER ,
        GEOMETRY_SHADER = GL_GEOMETRY_SHADER ,
        TESS_CONTROL_SHADER = GL_TESS_CONTROL_SHADER ,
        TESS_EVALUATION_SHADER = GL_TESS_EVALUATION_SHADER ,
        COMPUTE_SHADER = GL_COMPUTE_SHADER ,
    };

    class Shader {
        uint32_t renderer_id;
    
        std::string vertex_path;
        std::string fragment_path;
    
        std::string vertex_src;
        std::string fragment_src;
    
        std::unordered_map<std::string , int32_t> uniform_locations;
    
        bool valid = false;
    
        bool CompileShader(uint32_t shader_piece , const char* src);
        bool LinkShader();
    
        uint32_t GetUniformLocation(const std::string& name);
    
        public:
            Shader(const std::string& vertex_path , const std::string& fragment_path);
            ~Shader();
    
            void Bind() const;
            void Unbind() const;
    
            inline const bool IsValid() const { return valid; }
    
            void SetUniform(const std::string& name , const int32_t& value);
            void SetUniform(const std::string& name , const float& value);
            void SetUniform(const std::string& name , const glm::vec2& value);
            void SetUniform(const std::string& name , const glm::vec3& value);
            void SetUniform(const std::string& name , const glm::vec4& value);
            void SetUniform(const std::string& name , const glm::mat2& value);
            void SetUniform(const std::string& name , const glm::mat3& value);
            void SetUniform(const std::string& name , const glm::mat4& value);
    };

} // namespace other 

#endif // !OTHER_ENGINE_SHADER_HPP
