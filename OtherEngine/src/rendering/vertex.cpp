#include "rendering/vertex.hpp"

#include <iostream>

namespace other {

  VertexBuffer::VertexBuffer(
    const void* data , uint32_t size , 
    BufferUsage usage , 
    BufferType type
  ) : buffer_type(type) , buffer_usage(usage) , 
      renderer_id(0) , buffer_size(size)  ,
      buffer_data(data) {
    glGenBuffers(1 , &renderer_id);
    glBindBuffer(type , renderer_id);
    glBufferData(type , size , data , usage);
  }
  
  VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1 , &renderer_id);
  }
  
  void VertexBuffer::Bind() const {
    glBindBuffer(buffer_type , renderer_id);
  }
  
  void VertexBuffer::Unbind() const {
    glBindBuffer(buffer_type , 0);
  }
  
  void VertexArray::SetLayout() {
    if (layout.size() == 0) {
      layout = { 3 };
    }
  
    uint32_t stride = 0;
    for (uint32_t i = 0; i < layout.size(); i++) {
      stride += layout[i];
    }

    vertex_count = vertices.size() / stride;
    stride *= sizeof(float);
  
    uint32_t offset = 0;
    for (uint32_t i = 0; i < layout.size(); i++) {
      glEnableVertexAttribArray(i);
      glVertexAttribPointer(i , layout[i] , GL_FLOAT , GL_FALSE , stride , (void*)(offset * sizeof(float)));
  
      offset += layout[i];
    }
  }
  
  VertexArray::VertexArray(const std::vector<float>& vertices , const std::vector<uint32_t>& indices , const std::vector<uint32_t>& layout)
      : vertices(vertices) , indices(indices) , layout(layout) {
    glGenVertexArrays(1 , &renderer_id);
    glBindVertexArray(renderer_id);
  
    vertex_buffer = std::make_unique<VertexBuffer>(
      vertices.data() , vertices.size() * sizeof(float)
    );
    
    if (indices.size() == 0) {
      SetLayout();
    } else {
      index_buffer = std::make_unique<VertexBuffer>(
        indices.data() , indices.size() * sizeof(uint32_t) , 
        STATIC_DRAW , ELEMENT_ARRAY_BUFFER
      );
  
      SetLayout();
    }
  }
  
  VertexArray::~VertexArray() {
    glDeleteVertexArrays(1 , &renderer_id);
  }
  
  void VertexArray::Bind() const {
    glBindVertexArray(renderer_id);
  }
  
  void VertexArray::Draw(DrawMode mode) const {
    Bind();
    if (index_buffer != nullptr) {
      glDrawElements(mode , indices.size() , GL_UNSIGNED_INT , 0);
    } else {
      glDrawArrays(mode , 0 , vertex_count);
    }
    Unbind();
  }
  
  void VertexArray::Unbind() const {
    glBindVertexArray(0);
  }

} // namespace other 
