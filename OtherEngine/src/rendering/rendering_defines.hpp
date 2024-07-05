/**
 * \file rendering/rendering_defines.hpp
 **/
#ifndef OTHER_ENGINE_RENDERING_DEFINES_HPP
#define OTHER_ENGINE_RENDERING_DEFINES_HPP

#include <string>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "core/logger.hpp"

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
    POINTS = GL_POINTS ,
    LINES = GL_LINES ,
    LINE_STRIP = GL_LINE_STRIP ,
    LINE_LOOP = GL_LINE_LOOP ,
    TRIANGLES = GL_TRIANGLES ,
    TRIANGLE_STRIP = GL_TRIANGLE_STRIP ,
    TRIANGLE_FAN = GL_TRIANGLE_FAN ,
  };

  enum DepthFunction {
    NEVER = GL_NEVER , 
    LESS = GL_LESS , 
    EQUAL = GL_EQUAL ,
    LESS_EQUAL = GL_LEQUAL ,
    GREATER = GL_GREATER ,
    NOT_EQUAL = GL_NOTEQUAL ,
    GREATER_EQUAL = GL_GEQUAL ,
    ALWAYS = GL_ALWAYS ,
  };
  
  struct VertexBufferElement {
    std::string name = "";
    ValueType type = ValueType::EMPTY;
    uint32_t size = 0;
    uint32_t offset = 0;

    uint32_t GetComponentCount();

    VertexBufferElement() {}
    VertexBufferElement(ValueType type , const std::string& name)
        : name(name) , type(type) , offset(0) {
      size = GetComponentCount();
    }
  };

  class Layout {
    public:
      Layout() {}
      Layout(const std::initializer_list<VertexBufferElement>& elements);

      uint32_t Stride() const;
      const std::vector<VertexBufferElement> Elements() const;
      uint32_t Count() const;

      [[nodiscard]] std::vector<VertexBufferElement>::iterator begin() { return elements.begin(); }
      [[nodiscard]] std::vector<VertexBufferElement>::iterator end() { return elements.end(); }
      [[nodiscard]] std::vector<VertexBufferElement>::const_iterator begin() const { return elements.begin(); }
      [[nodiscard]] std::vector<VertexBufferElement>::const_iterator end() const { return elements.end(); }

    private:
      uint32_t stride = 0;
      std::vector<VertexBufferElement> elements;

      void CalculateOffsetAndStride();
  };

  struct FramebufferSpec {
    DepthFunction depth_func;

    glm::vec4 clear_color = { 1.f , 0.f , 0.f , 1.f };
    glm::ivec2 size = { 0.f , 0.f };

    bool depth = true;
    bool color = true;
    bool stencil = true;
  };

#define LAYOUT_CLASS 0

  struct PipelineSpec {
    bool has_indices = false;
    uint32_t buffer_cap = 4096;

    DrawMode topology = DrawMode::TRIANGLES;
    bool back_face_culling = true;
    bool depth_test = true;
    bool depth_write = true;
    bool wire_frame = false;
    float line_width = 1.f;

    FramebufferSpec framebuffer_spec {};
    Layout vertex_layout;

    std::string debug_name;
  };

  struct SceneRenderSpec {
    std::vector<PipelineSpec> pipeline_descs;
  };
  
  static void CheckGlError(const char* file , int line) {
    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
      switch (err) {
        case GL_INVALID_ENUM: 
          OE_ERROR("OpenGL Error INVALID_ENUM : [{} | {}]" , file , line);
        break;
        case GL_INVALID_VALUE:
          OE_ERROR("OpenGL Error INVALID_VALUE : [{} | {}]" , file , line);
        break;
        case GL_INVALID_OPERATION:
          OE_ERROR("OpenGL Error INVALID_OPERATION : [{} | {}]" , file , line);
        break;
        case GL_STACK_OVERFLOW:
          OE_ERROR("OpenGL Error STACK_OVERFLOW : [{} | {}]" , file , line);
        break;
        case GL_STACK_UNDERFLOW:
          OE_ERROR("OpenGL Error STACK_UNDERFLOW : [{} | {}]" , file , line);
        break;
        case GL_OUT_OF_MEMORY:
          OE_ERROR("OpenGL Error OUT_OF_MEMORY : [{} | {}]" , file , line);
        break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
          OE_ERROR("OpenGL Error INVALID_FRAMEBUFFER_OPERATION : [{} | {}]" , file , line);
        break;
        case GL_CONTEXT_LOST:
          OE_ERROR("OpenGL Error CONTEXT_LOST : [{} | {}]" , file , line);
        break;
        case GL_TABLE_TOO_LARGE:
          OE_ERROR("OpenGL Error TABLE_TOO_LARGE : [{} | {}]" , file , line);
        break;
        default: break;
      }
      err = glGetError();
    }
  }
  
#define CHECKGL() do { other::CheckGlError(__FILE__ , __LINE__); } while (false)

} // namespace other

#endif // !OTHER_ENGINE_RENDERING_DEFINES_HPP
