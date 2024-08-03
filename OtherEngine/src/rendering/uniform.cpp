/**
 * \file rendering/uniform.cpp
 **/
#include "rendering/uniform.hpp"

#include <glad/glad.h>

#include "rendering/rendering_defines.hpp"

namespace other {

  UniformBuffer::UniformBuffer(const std::string& name , const std::vector<Uniform> unis , uint32_t binding_point ,
                               BufferType type , BufferUsage usage) 
      : name(name) , type(type) , usage(usage) , binding_point(binding_point)  { 
    glGenBuffers(1 , &renderer_id);
    glBindBuffer(type , renderer_id);
    
    uint32_t offset = 0;
    for (const auto& u : unis) {
      size += GetValueSize(u.type) * u.arr_length;

      UUID hash = FNV(u.name);
      auto& u_data = uniforms[hash] = UniformData{
        .hash = hash , 
        .offset = offset ,
        .size = GetValueSize(u.type) ,
      };

      offset += u_data.size * u.arr_length;
      
    }

    OE_DEBUG("Allocating Uniform Buffer {} = {}" , name , size);

    glBufferData(type , size , nullptr , usage);
    glBindBuffer(type , 0);
  } 

  UniformBuffer::~UniformBuffer() {
    glBufferData(type , size , nullptr , usage);
    glDeleteBuffers(1 , &renderer_id);
  }

  const std::string& UniformBuffer::Name() const {
    return name;
  }

  void UniformBuffer::BindShader(const Ref<Shader>& shader) {
    switch (type) {
      case UNIFORM_BUFFER: {
        uint32_t idx = glGetUniformBlockIndex(shader->ID() , name.c_str());
        glUniformBlockBinding(shader->ID() , idx , binding_point);
        break;
      } case SHADER_STORAGE: {
        uint32_t idx = glGetProgramResourceIndex(shader->ID() , GL_SHADER_STORAGE_BLOCK , name.c_str());
        glShaderStorageBlockBinding(shader->ID() , idx , binding_point);
      } break;
      default:
        break;
    }
  }

  bool UniformBuffer::Bound() const {
    return bound;
  }
      
  void UniformBuffer::BindBase() {
    glBindBufferBase(type , binding_point , renderer_id);
  }

  void UniformBuffer::BindRange(size_t offset , size_t size) {
    OE_ASSERT(false , "unimplemented");
  }

  void UniformBuffer::Bind() {
    glBindBuffer(type , renderer_id);
  }
  
  void UniformBuffer::Unbind() {
    glBindBuffer(type , 0);
  }
      
  uint32_t UniformBuffer::CalculateOffset(const UniformData& uniform , uint32_t index) {
    return uniform.offset + index * uniform.size;
  }
      
  std::tuple<UniformBuffer::UniformData , bool , uint32_t> UniformBuffer::TryFind(const std::string_view name , uint32_t index) {
    auto [id , u_data] = GetUniform(name);
    if (id.Get() == 0) {
      return { {} , false  , 0 };
    }
    
    uint32_t offset = CalculateOffset(u_data , index); 
    return { u_data , true , offset };
  }
      
  std::pair<UUID , UniformBuffer::UniformData> UniformBuffer::GetUniform(const std::string_view name) {
    static const auto null_uniform = std::pair<UUID , UniformBuffer::UniformData>{ 0 , {} };

    UUID hash = FNV(name);
    auto itr = uniforms.find(hash);
    if (itr == uniforms.end()) {
      OE_ERROR("Attempting to set invalid uniform {}" , name);
      return null_uniform;
    }

    return *uniforms.find(hash);
  }

} // namespace other
