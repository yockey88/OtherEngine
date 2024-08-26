#include "rendering/shader.hpp"

#include <glad/glad.h>
#include <string>
#include <string>

#include <glm/gtc/type_ptr.hpp>

#include "core/logger.hpp"
#include "core/rand.hpp"
#include "core/filesystem.hpp"

#include "parsing/shader_compiler.hpp"

namespace other {
      
  const bool Shader::IsValid() const {
    return valid;
  }
      
  const bool Shader::HasGeometry() const {
    return ir.geom_source.has_value();
  }
      
  Shader::Shader(const ShaderIr& src) 
      : ir(src) {
    handle = Random::GenerateUUID();

    const char* vsrc_ptr = ir.vert_source.c_str();
    const char* fsrc_ptr = ir.frag_source.c_str();
    const char* gsrc_ptr = nullptr;
    if (HasGeometry()) {
      gsrc_ptr = ir.geom_source.value().c_str();
    }

    if (!Compile(vsrc_ptr , fsrc_ptr , gsrc_ptr)) {
      OE_ERROR("Failed to compile shader!");
      valid = false;
    }
  }
  
  Shader::~Shader() {
   glDeleteProgram(renderer_id);
  }

  uint32_t Shader::ID() const {
    return renderer_id;
  }
  
  void Shader::Bind() const {
    if (!valid) {
      return;
    }
    glUseProgram(renderer_id);
  }
  
  void Shader::Unbind() const {
    glUseProgram(0);
  }
    
  void Shader::InnerSetUniform(const std::string_view name , const int32_t& value , uint32_t index) {
    uint32_t loc = GetUniformLocation(name);
    glUniform1i(loc , value);
  }
  
  void Shader::InnerSetUniform(const std::string_view name , const float& value , uint32_t index) {
    uint32_t loc = GetUniformLocation(name);
    glUniform1f(loc , value);
  }
  
  void Shader::InnerSetUniform(const std::string_view name , const glm::vec2& value , uint32_t index) {
    uint32_t loc = GetUniformLocation(name);
    glUniform2f(loc , value.x , value.y);
  }
  
  void Shader::InnerSetUniform(const std::string_view name , const glm::vec3& value , uint32_t index) {
    uint32_t loc = GetUniformLocation(name);
    glUniform3f(loc , value.x , value.y , value.z);
  }
  
  void Shader::InnerSetUniform(const std::string_view name , const glm::vec4& value , uint32_t index) {
    uint32_t loc = GetUniformLocation(name);
    glUniform4f(loc , value.x , value.y , value.z , value.w);
  }
  
  void Shader::InnerSetUniform(const std::string_view name , const glm::mat2& value , uint32_t index) {
    uint32_t loc = GetUniformLocation(name);
    glUniformMatrix2fv(loc , 1 , GL_FALSE , glm::value_ptr(value));
  }
  
  void Shader::InnerSetUniform(const std::string_view name , const glm::mat3& value , uint32_t index) {
    uint32_t loc = GetUniformLocation(name);
    glUniformMatrix3fv(loc , 1 , GL_FALSE , glm::value_ptr(value));
  }
  
  void Shader::InnerSetUniform(const std::string_view name , const glm::mat4& value , uint32_t index) {
    uint32_t loc = GetUniformLocation(name);
    glUniformMatrix4fv(loc , 1 , GL_FALSE , glm::value_ptr(value));
  }
      
  bool Shader::Compile(const char* vsrc , const char* fsrc , const char* gsrc) {
    uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    Opt<uint32_t> geometry_shader = std::nullopt;
    if (HasGeometry()) {
      geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    }
  
    glShaderSource(vertex_shader , 1 , &vsrc , nullptr);
    glShaderSource(fragment_shader , 1 , &fsrc , nullptr);
    if (HasGeometry()) {
      glShaderSource(geometry_shader.value() , 1 , &gsrc , nullptr);
    }
  
    if (!CompileShader(VERTEX_SHADER , vertex_shader , vsrc)) {
      return false;
    }
  
    if (!CompileShader(FRAGMENT_SHADER , fragment_shader , fsrc)) {
      return false;
    }

    if (HasGeometry() && !CompileShader(GEOMETRY_SHADER , geometry_shader.value() , gsrc)) {
      return false;
    }
  
    renderer_id = glCreateProgram();
    glAttachShader(renderer_id , vertex_shader);
    glAttachShader(renderer_id , fragment_shader);
    if (HasGeometry()) {
      glAttachShader(renderer_id , geometry_shader.value());
    }
  
    if (!LinkShader()) {
      return false;
    }
  
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    if (HasGeometry()) {
      glDeleteShader(geometry_shader.value());
    }
  
    valid = true;

    return true;
  }
  
  bool Shader::CompileShader(ShaderType type , uint32_t shader_piece , const char* src) {
    int32_t check = 0;
    glCompileShader(shader_piece);
    glGetShaderiv(shader_piece , GL_COMPILE_STATUS , &check);
    if (check != GL_TRUE) {
      int32_t length = 0;
      glGetShaderiv(shader_piece , GL_INFO_LOG_LENGTH , &length);
      std::vector<char> message(length);
      glGetShaderInfoLog(shader_piece , length , &length , message.data());
  
      std::string error(message.begin() , message.end());
      std::string type_str = "UNKNOWN";
      switch (type) {
        case VERTEX_SHADER: type_str = "vertex"; break;
        case FRAGMENT_SHADER: type_str = "fragment"; break;
        case GEOMETRY_SHADER: type_str = "geometry"; break;
        default:
          break;
      }
      OE_ERROR("Failed to compile {} shader: {}" , type_str , error);
  
      return false;
    }
  
    return true;
  }
  
  bool Shader::LinkShader() {
    int32_t check = 0;
    glLinkProgram(renderer_id);
    glGetProgramiv(renderer_id , GL_LINK_STATUS , &check);
    if (check != GL_TRUE) {
      int32_t length = 0;
      glGetProgramiv(renderer_id , GL_INFO_LOG_LENGTH , &length);
      std::vector<char> message(length);
      glGetProgramInfoLog(renderer_id , length , &length , message.data());
  
      std::string error(message.begin() , message.end());
      OE_ERROR("Failed to link shader program: {}" , error);
  
      return false;
    }
  
    return true;
  }
  
  uint32_t Shader::GetUniformLocation(const std::string_view name) {
    auto hash = FNV(name);
    if (uniform_locations.find(hash) != uniform_locations.end()) {
      return uniform_locations[hash];
    }
  
    std::string name_str = std::string{ name };
    int32_t location = glGetUniformLocation(renderer_id , name_str.c_str());
    uniform_locations[hash] = location;
  
    return location;
  }
  
  Ref<Shader> BuildShader(const Path& path) {
    std::string src = Filesystem::ReadFile(path);
    if (src.empty()) {
      OE_ERROR("Failed to read shader file {}" , path);
      return nullptr;
    }

    ShaderIr ir = ShaderCompiler::Compile(src);

#if 0
    std::cout << "Transpile vertex source : \n\n" << ir.vert_source << "\n------------\n";
    std::cout << "Transpile fragment source : \n\n" << ir.frag_source << "\n------------\n";

    if (ir.geom_source.has_value()) {
      std::cout << "Transpile geometry source : \n\n" << ir.geom_source.value() << "\n------------\n";
    }
#endif

    return NewRef<Shader>(ir);
  }
  
  Ref<Shader> BuildShader(const Path& vpath , const Path& fpath , const Opt<Path>& gpath) {
    OE_ASSERT(false , "UNIMPLEMENTED");
    // std::string vsrc = Filesystem::ReadFile(vpath);
    // std::string fsrc = Filesystem::ReadFile(fpath);
    // Opt<std::string> gsrc  = std::nullopt;

    // if (gpath.has_value()) {
    //   gsrc = Filesystem::ReadFile(gpath.value());
    // }

    // ShaderIr vert_ir = ShaderCompiler::Compile(VERTEX_SHADER , vsrc);
    // ShaderIr frag_ir = ShaderCompiler::Compile(FRAGMENT_SHADER , fsrc);
    // Opt<ShaderIr> geom_ir = std::nullopt;

    // if (gsrc.has_value()) {
    //   geom_ir = ShaderCompiler::Compile(GEOMETRY_SHADER , gsrc.value());
    // }

    // ShaderIr real_ir;
    // real_ir.vert_source = vert_ir.vert_source;
    // real_ir.frag_source = vert_ir.frag_source;

    // if (geom_ir.has_value()) {
    //   real_ir.geom_source = geom_ir.value().geom_source;
    // }

    // return NewRef<Shader>(real_ir);
  }

} // namespace yockcraft
