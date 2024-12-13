/**
 * \file rendering/gbuffer.hpp
 **/
#ifndef GBUFFER_HPP
#define GBUFFER_HPP

#include <glm/glm.hpp>

#include "core/ref.hpp"

#include "rendering/shader.hpp"

namespace other {

  class GBuffer : public RefCounted {
   public:
    enum TexIndex {
      POSITION = 0,
      NORMAL,
      ALBEDO,
      SPECULAR,

      NUM_TEX_IDXS,
      INVALID_TEX_IDX = NUM_TEX_IDXS
    };

    uint32_t render_buffer = 0;
    uint32_t textures[NUM_TEX_IDXS] = {
      0, 0, 0, 0
    };

    GBuffer(const glm::ivec2& size);
    virtual ~GBuffer() override;

    bool Valid() const;

    void Bind() const;
    void Unbind() const;

   private:
    Ref<Shader> shader = nullptr;

    uint32_t gbuffer_id = 0;

    bool valid = false;
  };

}  // namespace other

#endif  // !GBUFFER_HPP
