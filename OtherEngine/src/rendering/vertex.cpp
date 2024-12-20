#include "rendering/vertex.hpp"

#include <glad/glad.h>

namespace other {

  VertexBuffer::VertexBuffer(BufferType type, size_t capacity)
      : buffer_type(type), buffer_usage(BufferUsage::DYNAMIC_DRAW),
        dynamic_capacity(capacity) {
    glGenBuffers(1, &renderer_id);
    glBindBuffer(type, renderer_id);
    glBufferData(type, dynamic_capacity.value(), nullptr, buffer_usage);
  }

  VertexBuffer::VertexBuffer(const void* data, uint32_t size, BufferUsage usage, BufferType type)
      : buffer_type(type), buffer_usage(usage), buffer_size(size) {
    glGenBuffers(1, &renderer_id);
    glBindBuffer(type, renderer_id);
    glBufferData(type, size, data, buffer_usage);
  }

  VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &renderer_id);
  }

  size_t VertexBuffer::Size() const {
    return buffer_size;
  }

  void VertexBuffer::Bind() const {
    glBindBuffer(buffer_type, renderer_id);
  }

  void VertexBuffer::Unbind() const {
    glBindBuffer(buffer_type, 0);
  }

  void VertexBuffer::BufferData(const void* data, uint32_t size, uint32_t offset) {
    if (buffer_usage != BufferUsage::DYNAMIC_DRAW) {
      return;
    }

    if (size > dynamic_capacity.value()) {
      return;
    }

    Bind();
    glBufferSubData(buffer_type, offset, size, data);
    Unbind();

    buffer_size += offset;
  }

  void VertexBuffer::ClearBuffer() {
    if (buffer_usage != BufferUsage::DYNAMIC_DRAW) {
      return;
    }

    Bind();
    glBufferSubData(buffer_type, 0, dynamic_capacity.value(), nullptr);
    Unbind();
  }

  VertexArray::VertexArray(const std::vector<float>& vertices, const std::vector<uint32_t>& indices, const std::vector<uint32_t>& layout)
      : vertices(vertices), indices(indices), layout(layout) {
    glGenVertexArrays(1, &renderer_id);
    Bind();

    vertex_buffer = NewRef<VertexBuffer>(vertices.data(), vertices.size() * sizeof(float));
    if (indices.size() != 0) {
      index_buffer = NewRef<VertexBuffer>(indices.data(), indices.size() * sizeof(uint32_t), STATIC_DRAW, ELEMENT_ARRAY_BUFFER);
    }

    SetLayout();

    Unbind();
  }

  VertexArray::VertexArray(const Ref<VertexArray>& other) {
    renderer_id = other->renderer_id;
    vertex_count = other->vertex_count;
    vertices = other->vertices;
    indices = other->indices;
    layout = other->layout;
    vertex_buffer = other->vertex_buffer;
    index_buffer = other->index_buffer;
  }

  VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &renderer_id);
  }

  void VertexArray::Bind() const {
    glBindVertexArray(renderer_id);
  }

  void VertexArray::Draw(DrawMode mode) const {
    Bind();
    if (index_buffer != nullptr) {
      glDrawElements(mode, indices.size(), GL_UNSIGNED_INT, 0);
    } else {
      glDrawArrays(mode, 0, vertex_count);
    }
    Unbind();
  }

  void VertexArray::Unbind() const {
    glBindVertexArray(0);
  }

  uint32_t VertexArray::RendererId() const {
    return renderer_id;
  }

  uint32_t VertexArray::NumElements() const {
    return indices.size();
  }

  DrawMode VertexArray::DrawModeFromFaceIndexCount(uint32_t num_indices) {
    switch (num_indices) {
      case 1:
        return DrawMode::POINTS;
      case 2:
        return DrawMode::LINES;
        return DrawMode::LINE_LOOP;
        return DrawMode::LINE_STRIP;

      case 3:
        return DrawMode::TRIANGLES;
        return DrawMode::TRIANGLE_STRIP;
        return DrawMode::TRIANGLE_FAN;

      default:
        OE_ASSERT(false, "Invalid number of indices for face!");
    }
  }

  void VertexArray::SetLayout() {
    if (layout.size() == 0) {
      layout = Vertex::RawLayout();
    }

    uint32_t stride = 0;
    for (uint32_t i = 0; i < layout.size(); i++) {
      stride += layout[i];
    }
    vertex_count = vertices.size() / stride;

    uint32_t offset = 0;
    for (uint32_t i = 0; i < layout.size(); i++) {
      glEnableVertexAttribArray(i);
      glVertexAttribPointer(i, layout[i], GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(offset * sizeof(float)));

      offset += layout[i];
    }
  }

}  // namespace other
