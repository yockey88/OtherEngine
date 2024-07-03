#ifndef OTHER_ENGINE_VERTEX_HPP
#define OTHER_ENGINE_VERTEX_HPP

#include <cstdint>
#include <vector>

#include "core/defines.hpp"
#include "core/ref_counted.hpp"

#include "math/vecmath.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/color.hpp"

namespace other {

  struct Vertex {
    Point position{ 0.f , 0.f , 0.f };
    RgbColor color{ 0.f , 0.f , 0.f };
    glm::vec3 normal{ 0.f , 0.f , 0.f };
    glm::vec3 tangent{ 0.f , 0.f , 0.f };
    glm::vec3 bitangent{ 0.f , 0.f , 0.f };
    glm::vec2 uv_coord{ 0.f , 0.f };

    /// DO NOT FORGET TO CHANGE BOTH OF THESE FUNCTIONS IF YOU CHANGE ONE
    constexpr static std::vector<uint32_t> Layout() {
      return { 3 , 3 , 3 , 3 , 3 , 2 };
    }

    constexpr static uint32_t Stride() {
      return 17;
    }
  };

  class VertexBuffer : public RefCounted { 
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
