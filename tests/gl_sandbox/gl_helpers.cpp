/**
 * \file gl_helpers.cpp
 **/
#include "gl_helpers.hpp"

#include <glad/glad.h>

#include "core/logger.hpp"

namespace other {

  uint32_t GetShader(const char* vsrc , const char* fsrc) {
    int success;

    int32_t vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vsrc, nullptr);
    
    glCompileShader(vert);
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetShaderInfoLog(vert, 512, nullptr, info);
      OE_ERROR("Failed to compile vertex shader vert 1: {0}", info);
    }

    int32_t frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fsrc, nullptr);

    glCompileShader(frag);
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[511];
      glGetShaderInfoLog(frag, 512, nullptr, info);
      OE_ERROR("Failed to compile fragment shader frag 1: {0}", info);
    }
    
    CHECK();
    
    uint32_t shader = 0;
    shader = glCreateProgram();
    glAttachShader(shader, vert);
    glAttachShader(shader, frag);
    glLinkProgram(shader);
    
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetProgramInfoLog(shader, 512, nullptr, info);
      OE_ERROR("Failed to link shader program shader 1: {0}", info);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    
    CHECK();

    return shader;
  }

  void CheckGlError(int line) {
    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
      switch (err) {
        case GL_INVALID_ENUM: 
          OE_ERROR("OpenGL Error INVALID_ENUM : {}" , line);
        break;
        case GL_INVALID_VALUE:
          OE_ERROR("OpenGL Error INVALID_VALUE : {}" , line);
        break;
        case GL_INVALID_OPERATION:
          OE_ERROR("OpenGL Error INVALID_OPERATION : {}" , line);
        break;
        case GL_STACK_OVERFLOW:
          OE_ERROR("OpenGL Error STACK_OVERFLOW : {}" , line);
        break;
        case GL_STACK_UNDERFLOW:
          OE_ERROR("OpenGL Error STACK_UNDERFLOW : {}" , line);
        break;
        case GL_OUT_OF_MEMORY:
          OE_ERROR("OpenGL Error OUT_OF_MEMORY : {}" , line);
        break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
          OE_ERROR("OpenGL Error INVALID_FRAMEBUFFER_OPERATION : {}" , line);
        break;
        case GL_CONTEXT_LOST:
          OE_ERROR("OpenGL Error CONTEXT_LOST : {}" , line);
        break;
        case GL_TABLE_TOO_LARGE:
          OE_ERROR("OpenGL Error TABLE_TOO_LARGE : {}" , line);
        break;
        default: break;
      }
      err = glGetError();
    }
  }

} // namespace other
