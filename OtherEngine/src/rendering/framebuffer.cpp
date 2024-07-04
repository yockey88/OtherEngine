/**
 * \file rendering/framebuffer.cpp
 **/
#include "rendering/framebuffer.hpp"

#include <glad/glad.h>

#include "core/logger.hpp"

namespace other {

  Framebuffer::Framebuffer(const FramebufferSpec& spec) 
      : spec(spec) {
    CreateFramebuffer();
  }
  
  Framebuffer::~Framebuffer() {
    DestroyFramebuffer();
  }

  bool Framebuffer::Valid() const {
    return fb_complete;
  }
      
  void Framebuffer::Resize(const glm::vec2& sz) {
    spec.size = sz;
    DestroyKeepTextures();
    CreateWithOldTextures();
  }
      
  void Framebuffer::BindFrame() {
    if (!fb_complete) {
      return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER , fbo);
    glClearColor(spec.clear_color.x , spec.clear_color.y , spec.clear_color.z , spec.clear_color.w);
    glClear(clear_flags);

    if (spec.depth) {
      glEnable(GL_DEPTH_TEST);
    }
  }

  void Framebuffer::UnbindFrame() {
    if (!fb_complete) {
      return;
    }
    
    if (spec.depth) {
      glDisable(GL_DEPTH_TEST);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER , fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER , intermediate_fbo);
    glBlitFramebuffer(0 , 0 , spec.size.x , spec.size.y , 0 , 0 , spec.size.x , spec.size.y , clear_flags , GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER , 0);
  }

  void Framebuffer::Draw() const {
    /// draw 
  }

  void Framebuffer::CreateFramebuffer() {
    glGenFramebuffers(1 , &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER , fbo);

    if (spec.depth) {
      glGenTextures(1 , &depth_attachment);
      glBindTexture(GL_TEXTURE_2D , depth_attachment);
      
      glTexImage2D(GL_TEXTURE_2D , 0 , GL_DEPTH_COMPONENT , spec.size.x , spec.size.y , 0 , GL_DEPTH_COMPONENT , GL_UNSIGNED_BYTE , nullptr);
      glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_REPEAT); 
      glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_REPEAT); 
      glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR); 
      glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR); 

      glBindTexture(GL_TEXTURE_2D , 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_TEXTURE_2D , depth_attachment , 0);
      
      clear_flags |= GL_DEPTH_BUFFER_BIT;
    }
    
    CHECKGL();

    if (spec.color) {
      glGenTextures(1 , &color_attachment);
      glBindTexture(GL_TEXTURE_2D , color_attachment);

      glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGB , spec.size.x , spec.size.y , 0 , GL_RGB , GL_UNSIGNED_BYTE , nullptr);
      glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_REPEAT); 
      glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_REPEAT); 
      glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR); 
      glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR); 
  
      glBindTexture(GL_TEXTURE_2D , 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , color_attachment , 0);
      
      clear_flags |= GL_COLOR_BUFFER_BIT;
    }
    
    CHECKGL();

    if (spec.stencil) {
      glGenRenderbuffers(1 , &rbo);
      glBindRenderbuffer(GL_RENDERBUFFER , rbo);
      glRenderbufferStorage(GL_RENDERBUFFER , GL_DEPTH24_STENCIL8 , spec.size.x , spec.size.y);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_DEPTH_STENCIL_ATTACHMENT , GL_RENDERBUFFER , rbo);

      clear_flags |= GL_STENCIL_BUFFER_BIT;
    }
    
    CHECKGL();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      OE_ERROR("Framebuffer is not complete!");
      DestroyFramebuffer();
      glBindFramebuffer(GL_FRAMEBUFFER , 0);
      return;
    } 

    glBindFramebuffer(GL_FRAMEBUFFER , 0);

    CHECKGL();

    glGenFramebuffers(1 , &intermediate_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER , intermediate_fbo);

    glGenTextures(1 , &texture);
    glBindTexture(GL_TEXTURE_2D , texture);

    CHECKGL();

    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGB , spec.size.x , spec.size.y , 0 , GL_RGB , GL_UNSIGNED_BYTE , nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, texture, 0);

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
    if (spec.depth) {
      glDeleteTextures(1 , &depth_attachment);
    }

    if (spec.color) {
      glDeleteTextures(1 , &color_attachment);
    }

    if (spec.stencil) {
      glDeleteRenderbuffers(1 , &rbo);
    }

    glDeleteTextures(1 , &texture);
    glDeleteFramebuffers(1 , &fbo);
    glDeleteFramebuffers(1 , &intermediate_fbo);
  }
      
  void Framebuffer::CreateWithOldTextures() {
    glGenFramebuffers(1 , &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER , fbo);

    CHECKGL();

    if (spec.depth) {
      glBindTexture(GL_TEXTURE_2D , depth_attachment);
      glTexImage2D(GL_TEXTURE_2D , 0 , GL_DEPTH_COMPONENT , spec.size.x , spec.size.y , 0 , GL_DEPTH_COMPONENT , GL_UNSIGNED_BYTE , nullptr);
      glBindTexture(GL_TEXTURE_2D , 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_TEXTURE_2D , depth_attachment , 0);
    }
    
    CHECKGL();

    if (spec.color) {
      glBindTexture(GL_TEXTURE_2D , color_attachment);
      glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGB , spec.size.x , spec.size.y , 0 , GL_RGB , GL_UNSIGNED_BYTE , nullptr);
      glBindTexture(GL_TEXTURE_2D , 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , color_attachment , 0);
    }
    
    CHECKGL();

    if (spec.stencil) {
      glGenRenderbuffers(1 , &rbo);
      glBindRenderbuffer(GL_RENDERBUFFER , rbo);
      glRenderbufferStorage(GL_RENDERBUFFER , GL_DEPTH24_STENCIL8 , spec.size.x , spec.size.y);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_DEPTH_STENCIL_ATTACHMENT , GL_RENDERBUFFER , rbo);
    }
    
    CHECKGL();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      OE_ERROR("Framebuffer is not complete!");
      DestroyFramebuffer();
      glBindFramebuffer(GL_FRAMEBUFFER , 0);
      return;
    } 

    glBindFramebuffer(GL_FRAMEBUFFER , 0);

    CHECKGL();

    glGenFramebuffers(1 , &intermediate_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER , intermediate_fbo);

    CHECKGL();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, texture, 0);

    CHECKGL();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      OE_ERROR("Framebuffer is not complete!");
      DestroyFramebuffer();
    } else {
      fb_complete = true;
    }

    glBindFramebuffer(GL_FRAMEBUFFER , 0);
  }

  void Framebuffer::DestroyKeepTextures() {
    if (spec.stencil) {
      glDeleteRenderbuffers(1 , &rbo);
    }

    glDeleteFramebuffers(1 , &fbo);
    glDeleteFramebuffers(1 , &intermediate_fbo);
  }

} // namespace other 
