/**
 * \file rendering/uniform.cpp
 **/
#include "rendering/uniform.hpp"

#include <glad/glad.h>

#include "core/defines.hpp"
#include "rendering/rendering_defines.hpp"

namespace other {

  UniformBuffer::UniformBuffer(const std::string& name , const std::vector<Uniform> unis , uint32_t binding_point ,
                               BufferType type , BufferUsage usage) 
      : name(name) , type(type) , usage(usage) , binding_point(binding_point)  { 
    glGenBuffers(1 , &renderer_id);
    glBindBuffer(type , renderer_id);
    
    uint32_t offset = 0;
    for (const auto& u : unis) {
      size_t type_size = GetValueSize(u.type);
      if (type_size == 0) {
        if (!u.size.has_value()) {
          OE_ERROR("Can not set uniform size for user defined type, failed on uniform '{}'" , u.name);
          glBindBuffer(type , 0);
          glDeleteBuffers(1 , &renderer_id);
          return;
        } else {
          type_size = *u.size;
        }
      } 

      size += type_size * u.arr_length;

      UUID hash = FNV(u.name);
      auto& u_data = uniforms[hash] = UniformData{
        .hash = hash , 
        .offset = offset ,
        .size = type_size ,
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
      
  void UniformBuffer::LoadFromBuffer(const Buffer& buffer) {
    glBindBuffer(type , renderer_id);
    CHECKGL();
    glBufferSubData(type , 0 , buffer.Size() , buffer.ReadBytes());
    CHECKGL();
    glBindBuffer(type , 0);
    CHECKGL();
  }
  
  void UniformBuffer::Unbind() {
    glBindBuffer(type , 0);
  }
      
  void UniformBuffer::Clear() {
    glBindBuffer(type , renderer_id);
    CHECKGL();
    glBufferSubData(type , 0 , size , nullptr); 
    CHECKGL();
    glBindBuffer(type , 0);
    CHECKGL();
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
