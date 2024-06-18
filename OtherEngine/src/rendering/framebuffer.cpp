/**
 * \file rendering/framebuffer.cpp
 **/
#include "rendering/framebuffer.hpp"

#include <glad/glad.h>

#include "core/logger.hpp"
#include "rendering/renderer.hpp"

namespace other {

  Framebuffer::Framebuffer() {
    size = Renderer::WindowSize();

    glGenFramebuffers(1 , &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER , fbo);

    glGenTextures(1 , &color_attachment);
    glBindTexture(GL_TEXTURE_2D , color_attachment);
    CreateFramebuffer();
  }
  
  Framebuffer::~Framebuffer() {
    glDeleteTextures(1 , &color_attachment);
    DestroyFramebuffer();
  }

  uint32_t Framebuffer::Texture() const {
    return color_attachment;
  }

  bool Framebuffer::Valid() const {
    return fb_complete;
  }
      
  void Framebuffer::Resize(const glm::vec2& sz) {
    size = sz;
    DestroyFramebuffer();
    
    glGenFramebuffers(1 , &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER , fbo);

    glBindTexture(GL_TEXTURE_2D , color_attachment);
    CreateFramebuffer();
  }
      
  void Framebuffer::BindFrame() {
    if (!fb_complete) {
      return;
    }

    auto clear_color = Renderer::GetWindow()->ClearColor();
    auto clear_flags = Renderer::GetWindow()->ClearFlags();
    glBindFramebuffer(GL_FRAMEBUFFER , fbo);
    glViewport(0 , 0 , static_cast<uint32_t>(size.x) , static_cast<uint32_t>(size.y));
    glClearColor(clear_color.x , clear_color.y , clear_color.z , clear_color.w);
    glClear(clear_flags);
    glEnable(GL_DEPTH_TEST);
  }

  void Framebuffer::UnbindFrame() {
    if (!fb_complete) {
      return;
    }

    auto size = Renderer::GetWindow()->Size();
    auto clear_color = Renderer::GetWindow()->ClearColor();
    auto clear_flags = Renderer::GetWindow()->ClearFlags();
    glBindFramebuffer(GL_READ_FRAMEBUFFER , fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER , intermediate_fbo);
    glBlitFramebuffer(0 , 0 , size.x , size.y , 0 , 0 , size.x , size.y , clear_flags , GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER , 0);
    glClearColor(clear_color.x , clear_color.y , clear_color.z , clear_color.w);
    glClear(clear_flags);
    glDisable(GL_DEPTH_TEST);
  }

  void Framebuffer::Draw() const {
    /// draw 
  }

  void Framebuffer::CreateFramebuffer() {

    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGB , size.x , size.y , 0 , GL_RGB , GL_UNSIGNED_BYTE , nullptr);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR); 
    glBindTexture(GL_TEXTURE_2D , 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , color_attachment , 0);
    
    CHECKGL();

    glGenRenderbuffers(1 , &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER , rbo);
    glRenderbufferStorage(GL_RENDERBUFFER , GL_DEPTH24_STENCIL8 , size.x , size.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_DEPTH_STENCIL_ATTACHMENT , GL_RENDERBUFFER , rbo);
    
    CHECKGL();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      OE_ERROR("Framebuffer is not complete!");
      DestroyFramebuffer();
    } else {
      fb_complete = true;
    }

    glBindFramebuffer(GL_FRAMEBUFFER , 0);
  }

  void Framebuffer::DestroyFramebuffer() {
    glDeleteRenderbuffers(1 , &rbo);
    glDeleteFramebuffers(1 , &fbo);
    glDeleteFramebuffers(1 , &intermediate_fbo);
  }

} // namespace other 
