#ifndef OTHER_ENGINE_VERTEX_HPP
#define OTHER_ENGINE_VERTEX_HPP

#include <cstdint>
#include <vector>

#include <glad/glad.h>

#include "core/defines.hpp"
#include "math/vecmath.hpp"
#include "rendering/color.hpp"

namespace other {

  enum BufferUsage {
    STATIC_DRAW = GL_STATIC_DRAW ,
    DYNAMIC_DRAW = GL_DYNAMIC_DRAW ,
    STREAM_DRAW = GL_STREAM_DRAW ,
  };
  
  enum BufferType {
    ARRAY_BUFFER = GL_ARRAY_BUFFER ,
    ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER ,
  };

  enum DrawMode {
    TRIANGLES = GL_TRIANGLES ,
    TRIANGLE_STRIP = GL_TRIANGLE_STRIP ,
    TRIANGLE_FAN = GL_TRIANGLE_FAN ,
    LINES = GL_LINES ,
    LINE_STRIP = GL_LINE_STRIP ,
    LINE_LOOP = GL_LINE_LOOP ,
    POINTS = GL_POINTS ,
  };

  struct Vertex {
    Point position;
    RgbColor color;
    Opt<glm::vec3> normal = std::nullopt;
    Opt<glm::vec3> tangent = std::nullopt;
    Opt<glm::vec3> bitangent = std::nullopt;
    Opt<glm::vec2> uv_coord = std::nullopt;

    /// DO NOT FORGET TO CHANGE BOTH OF THESE FUNCTIONS IF YOU CHANGE ONE
    constexpr static std::vector<uint32_t> Layout() {
      return { 3 , 3 , 3 , 3 , 3 , 2 };
    }

    constexpr static uint32_t Stride() {
      return 17;
    }
  };

  class VertexBuffer { 
    public:
      VertexBuffer(BufferType type , size_t capacity);
      VertexBuffer(const void* data , uint32_t size , BufferUsage usage = STATIC_DRAW , BufferType type = ARRAY_BUFFER);
      ~VertexBuffer();

      size_t Size() const;
  
      void Bind() const;
      void Unbind() const;

      void BufferData(const void* data , uint32_t size , uint32_t offset = 0);
      void ClearBuffer();

    private:
      BufferType buffer_type;
      BufferUsage buffer_usage; 

      Opt<size_t> dynamic_capacity = std::nullopt;
      
      uint32_t renderer_id = 0;
      uint32_t buffer_size;
  };
  
  class VertexArray { 
    public:
      VertexArray();
      VertexArray(
        const std::vector<float>& vertices , 
        const std::vector<uint32_t>& indices = {} , 
        const std::vector<uint32_t>& layout = {}
      );
  
      ~VertexArray();
  
      void Bind() const;
      void Draw(DrawMode mode) const;
      void Unbind() const;

    private:
      uint32_t renderer_id;
      uint32_t vertex_count;
  
      std::vector<float> vertices;
      std::vector<uint32_t> indices;
      std::vector<uint32_t> layout;
  
      Scope<VertexBuffer> vertex_buffer = nullptr;
      Scope<VertexBuffer> index_buffer = nullptr;
  
      void SetLayout();
  };

} // namespace other

#endif // !OTHER_ENGINE_VERTEX_HPP
