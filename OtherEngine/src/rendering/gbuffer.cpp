/**
 * \file rendering/gbuffer.cpp
 **/
#include "rendering/gbuffer.hpp"

#include <algorithm>

#include <glad/glad.h>

#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "rendering/shader.hpp"

namespace other {

  GBuffer::GBuffer(const glm::ivec2& size) {
    std::ranges::fill(textures , 0u);
    glGenFramebuffers(1 , &gbuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER , gbuffer_id);
    glGenTextures(3 , textures.data());
    
    glBindTexture(GL_TEXTURE_2D , textures[0]);
    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGBA16F , size.x , size.y , 0 , GL_RGBA , GL_FLOAT , nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0] , 0);

    glBindTexture(GL_TEXTURE_2D , textures[1]);
    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGBA16F , size.x , size.y , 0 , GL_RGBA , GL_FLOAT , nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D, textures[1] , 0);
    
    glBindTexture(GL_TEXTURE_2D , textures[2]);
    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGBA , size.x , size.y , 0 , GL_RGBA , GL_UNSIGNED_BYTE , nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, textures[2] , 0);
    
    uint32_t attachments[3] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3 , attachments);

    uint32_t render_buffer = 0;
    glGenRenderbuffers(1 , &render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER , render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER , GL_DEPTH_COMPONENT , size.x , size.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_RENDERBUFFER , render_buffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      OE_ERROR("G-Buffer not complete!");
    } else {
      valid = true;
    }

    glBindFramebuffer(GL_FRAMEBUFFER , 0);

    Path shader_dir = Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders";
    Path gbuffer_path = shader_dir / "gbuffer.oshader";

    shader = BuildShader(gbuffer_path);
  }

  GBuffer::~GBuffer() {
    std::ranges::fill(textures , 0u);
    glDeleteTextures(3 , textures.data());
    glDeleteFramebuffers(1 , &gbuffer_id);
  }

  bool GBuffer::Valid() const {
    return valid;
  }
      
  void GBuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER , gbuffer_id);
    // shader->Bind();
  }

  void GBuffer::Unbind() const {
    // shader->Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER , 0);
  }

} // namespace other
