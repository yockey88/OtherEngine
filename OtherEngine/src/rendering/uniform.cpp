/**
 * \file rendering/uniform.cpp
 **/
#include "rendering/uniform.hpp"

#include <glad/glad.h>

namespace other {

  UniformBuffer::UniformBuffer(const std::string& name , const std::vector<Uniform> unis , uint32_t binding_point , BufferUsage usage) 
      : name(name) , usage(usage) , binding_point(binding_point)  { 
    glGenBuffers(1 , &renderer_id);
    glBindBuffer(GL_UNIFORM_BUFFER , renderer_id);
    
    uint32_t offset = 0;
    for (const auto& u : unis) {
      size += GetValueSize(u.type);

      UUID hash = FNV(u.name);
      auto& u_data = uniforms[hash] = UniformData{
        .hash = hash , 
        .offset = offset ,
        .size = GetValueSize(u.type) ,
      };

      offset += u_data.size;
      
    }

    glBufferData(GL_UNIFORM_BUFFER , size , nullptr , usage);
    glBindBuffer(GL_UNIFORM_BUFFER , 0);
  } 

  UniformBuffer::~UniformBuffer() {
    glBufferData(GL_UNIFORM_BUFFER , size , nullptr , usage);
    glDeleteBuffers(1 , &renderer_id);
  }

  const std::string& UniformBuffer::Name() const {
    return name;
  }

  uint32_t UniformBuffer::BindingPoint() const {
    return binding_point;
  }

  bool UniformBuffer::Bound() const {
    return bound;
  }
      
  void UniformBuffer::BindBase() {
    glBindBufferBase(GL_UNIFORM_BUFFER , binding_point , renderer_id);
  }

  void UniformBuffer::BindRange(size_t offset , size_t size) {
    OE_ASSERT(false , "unimplemented");
    glBindBufferBase(GL_UNIFORM_BUFFER , binding_point , renderer_id);
  }

  void UniformBuffer::Bind() {
    glBindBuffer(GL_UNIFORM_BUFFER , renderer_id);
  }
  
  void UniformBuffer::Unbind() {
    glBindBuffer(GL_UNIFORM_BUFFER , 0);
  }
      
  std::pair<UUID , UniformBuffer::UniformData> UniformBuffer::GetUniform(const std::string& name , size_t val_size) {
    static const auto null_uniform = std::pair<UUID , UniformBuffer::UniformData>{ 0 , {} };

    UUID hash = FNV(name);
    auto itr = uniforms.find(hash);
    if (itr == uniforms.end()) {
      OE_WARN("Attempting to set invalid uniform {}" , name);
      return null_uniform;
    }

    if (val_size != itr->second.size) {
      OE_ERROR("Found uniform named {} with but size is incorrect (input = {} \\ real = {})" , name , val_size , itr->second.size);
      return null_uniform;
    }

    return *uniforms.find(hash);
  }

} // namespace other
