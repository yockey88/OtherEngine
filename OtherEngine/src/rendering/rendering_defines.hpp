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
#include "core/writer_reader.hpp"

namespace other {

  enum BufferUsage {
    STATIC_DRAW = GL_STATIC_DRAW,
    DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
    STREAM_DRAW = GL_STREAM_DRAW,
  };

  enum BufferType {
    ARRAY_BUFFER = GL_ARRAY_BUFFER,
    ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
    UNIFORM_BUFFER = GL_UNIFORM_BUFFER,
    SHADER_STORAGE = GL_SHADER_STORAGE_BUFFER,
  };

  enum RenderState {
    FILL = GL_FILL,
    WIREFRAME = GL_LINE,
    POINT = GL_POINT,
  };

  enum DrawMode {
    POINTS = GL_POINTS,
    LINES = GL_LINES,
    LINE_STRIP = GL_LINE_STRIP,
    LINE_LOOP = GL_LINE_LOOP,
    TRIANGLES = GL_TRIANGLES,
    TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
    TRIANGLE_FAN = GL_TRIANGLE_FAN,
  };

  enum DepthFunction {
    NEVER = GL_NEVER,
    LESS = GL_LESS,
    EQUAL = GL_EQUAL,
    LESS_EQUAL = GL_LEQUAL,
    GREATER = GL_GREATER,
    NOT_EQUAL = GL_NOTEQUAL,
    GREATER_EQUAL = GL_GEQUAL,
    ALWAYS = GL_ALWAYS,
  };

  enum ShaderType {
    OTHER_SHADER = 0,
    VERTEX_SHADER = GL_VERTEX_SHADER,
    FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
    GEOMETRY_SHADER = GL_GEOMETRY_SHADER,
    TESS_CONTROL_SHADER = GL_TESS_CONTROL_SHADER,
    TESS_EVALUATION_SHADER = GL_TESS_EVALUATION_SHADER,
    COMPUTE_SHADER = GL_COMPUTE_SHADER,
  };

  enum FilterType {
    NEAREST = GL_NEAREST,
    LINEAR = GL_LINEAR
  };

  enum TextureType {
    DIFFUSE,
    SPECULAR,
    NORMAL,
    HEIGHT
  };

  enum ChannelType {
    RED = GL_RED,
    RGB = GL_RGB,
    RGBA = GL_RGBA
  };

  enum TargetType {
    TEX_1D = GL_TEXTURE_1D,
    TEX_2D = GL_TEXTURE_2D,
    TEX_3D = GL_TEXTURE_3D,
    TEX_1D_ARR = GL_TEXTURE_1D_ARRAY,
    TEX_2D_ARR = GL_TEXTURE_2D_ARRAY,
    TEX_RECT = GL_TEXTURE_RECTANGLE,
    TEX_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
    TEX_CUBE_MAP_ARR = GL_TEXTURE_CUBE_MAP_ARRAY,
    TEX_BUFFER = GL_TEXTURE_BUFFER,
    TEX_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE,
    TEX_2D_MULTISAMPLE_ARR = GL_TEXTURE_2D_MULTISAMPLE_ARRAY
  };

  enum TextureWrap {
    CLAMP_EDGE = GL_CLAMP_TO_EDGE,
    CLAMP_BORDER = GL_CLAMP_TO_BORDER,
    REPEAT = GL_REPEAT,
    MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
  };

  struct FramebufferSpec {
    DepthFunction depth_func = DepthFunction::LESS;

    glm::vec4 clear_color = { 0.f, 0.f, 0.f, 1.f };
    glm::ivec2 size = { 800, 600 };

    bool depth = true;
    bool color = true;
    bool stencil = true;

    std::string framebuffer_name = "Framebuffer";
  };

  struct MeshAttr {
    std::string attr_name = "";
    uint32_t idx = 0;
    uint32_t size = 0;
  };

  struct MeshLayout {
    bool override = false;

    uint32_t curr_idx = 0;
    std::string layout_name = "";
    uint32_t stride = 0;
    std::vector<MeshAttr> attrs;
  };

  static void CheckGlError(const char* file, int line) {
    PROFILE_SECTION("Other--CheckGLError");
    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
      switch (err) {
        case GL_INVALID_ENUM:
          OE_ERROR("OpenGL Error INVALID_ENUM : [{} | {}]", file, line);
          break;
        case GL_INVALID_VALUE:
          OE_ERROR("OpenGL Error INVALID_VALUE : [{} | {}]", file, line);
          break;
        case GL_INVALID_OPERATION:
          OE_ERROR("OpenGL Error INVALID_OPERATION : [{} | {}]", file, line);
          break;
        case GL_STACK_OVERFLOW:
          OE_ERROR("OpenGL Error STACK_OVERFLOW : [{} | {}]", file, line);
          break;
        case GL_STACK_UNDERFLOW:
          OE_ERROR("OpenGL Error STACK_UNDERFLOW : [{} | {}]", file, line);
          break;
        case GL_OUT_OF_MEMORY:
          OE_ERROR("OpenGL Error OUT_OF_MEMORY : [{} | {}]", file, line);
          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
          OE_ERROR("OpenGL Error INVALID_FRAMEBUFFER_OPERATION : [{} | {}]", file, line);
          break;
        case GL_CONTEXT_LOST:
          OE_ERROR("OpenGL Error CONTEXT_LOST : [{} | {}]", file, line);
          break;
        case GL_TABLE_TOO_LARGE:
          OE_ERROR("OpenGL Error TABLE_TOO_LARGE : [{} | {}]", file, line);
          break;
        default: break;
      }
      err = glGetError();
    }
  }

  template <>
  struct Writer<DepthFunction> {
    std::ostream& operator()(std::ostream& os, DepthFunction func) {
      switch (func) {
        case DepthFunction::NEVER: os << "NEVER"; break;
        case DepthFunction::LESS: os << "LESS"; break;
        case DepthFunction::EQUAL: os << "EQUAL"; break;
        case DepthFunction::LESS_EQUAL: os << "LESS_EQUAL"; break;
        case DepthFunction::GREATER: os << "GREATER"; break;
        case DepthFunction::NOT_EQUAL: os << "NOT_EQUAL"; break;
        case DepthFunction::GREATER_EQUAL: os << "GREATER_EQUAL"; break;
        case DepthFunction::ALWAYS: os << "ALWAYS"; break;
        default: break;
      }
      return os;
    }
  };

  template <>
  struct Reader<DepthFunction> {
    DepthFunction operator()(std::istream& stream) {
      std::string func_str = Reader<std::string>{}(stream);
      if (func_str == "NEVER") {
        return DepthFunction::NEVER;
      }
      if (func_str == "LESS") {
        return DepthFunction::LESS;
      }
      if (func_str == "EQUAL") {
        return DepthFunction::EQUAL;
      }
      if (func_str == "LESS_EQUAL") {
        return DepthFunction::LESS_EQUAL;
      }
      if (func_str == "GREATER") {
        return DepthFunction::GREATER;
      }
      if (func_str == "NOT_EQUAL") {
        return DepthFunction::NOT_EQUAL;
      }
      if (func_str == "GREATER_EQUAL") {
        return DepthFunction::GREATER_EQUAL;
      }
      if (func_str == "ALWAYS") {
        return DepthFunction::ALWAYS;
      }
      OE_ERROR("Invalid DepthFunction value");
      return DepthFunction::LESS;
    }
  };

  template <>
  struct Writer<DrawMode> {
    std::ostream& operator()(std::ostream& os, DrawMode mode) {
      switch (mode) {
        case DrawMode::POINTS: os << "POINTS"; break;
        case DrawMode::LINES: os << "LINES"; break;
        case DrawMode::LINE_STRIP: os << "LINE_STRIP"; break;
        case DrawMode::LINE_LOOP: os << "LINE_LOOP"; break;
        case DrawMode::TRIANGLES: os << "TRIANGLES"; break;
        case DrawMode::TRIANGLE_STRIP: os << "TRIANGLE_STRIP"; break;
        case DrawMode::TRIANGLE_FAN: os << "TRIANGLE_FAN"; break;
        default: break;
      }
      return os;
    }
  };

  template <>
  struct Reader<DrawMode> {
    DrawMode operator()(std::istream& stream) {
      std::string mode_str = Reader<std::string>{}(stream);
      if (mode_str == "POINTS") {
        return DrawMode::POINTS;
      }
      if (mode_str == "LINES") {
        return DrawMode::LINES;
      }
      if (mode_str == "LINE_STRIP" || mode_str == "LINE-STRIP") {
        return DrawMode::LINE_STRIP;
      }
      if (mode_str == "LINE_LOOP" || mode_str == "LINE-LOOP") {
        return DrawMode::LINE_LOOP;
      }
      if (mode_str == "TRIANGLES") {
        return DrawMode::TRIANGLES;
      }
      if (mode_str == "TRIANGLE_STRIP" || mode_str == "TRIANGLE-STRIP") {
        return DrawMode::TRIANGLE_STRIP;
      }
      if (mode_str == "TRIANGLE_FAN" || mode_str == "TRIANGLE-FAN") {
        return DrawMode::TRIANGLE_FAN;
      }
      OE_ERROR("Invalid DrawMode value");
      return DrawMode::TRIANGLES;
    }
  };

}  // namespace other

#ifdef OTHER_DEBUG_BUILD
  #define CHECKGL()                            \
    do {                                       \
      other::CheckGlError(__FILE__, __LINE__); \
    } while (false)
#else
  #define CHECKGL()
#endif

#endif  // !OTHER_ENGINE_RENDERING_DEFINES_HPP
