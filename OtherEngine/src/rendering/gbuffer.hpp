/**
 * \file rendering/gbuffer.hpp
 **/
#ifndef GBUFFER_HPP
#define GBUFFER_HPP

#include <array>

#include <glm/glm.hpp>

#include "core/ref.hpp"

#include "rendering/shader.hpp"

namespace other {

  class GBuffer {
    public:
      enum TexIndex {
        POSITION = 0 ,
        NORMAL       ,
        ALBEDO       ,

        NUM_TEX_IDXS ,
        INVALID_TEX_IDX = NUM_TEX_IDXS
      };

      // std::array<uint32_t , NUM_TEX_IDXS> textures;
      uint32_t textures[NUM_TEX_IDXS] = {
        0 , 0 , 0 
      };

      GBuffer(const glm::ivec2& size);
      ~GBuffer();

      bool Valid() const;

      void Bind() const;
      void Unbind() const;

    private:
      Ref<Shader> shader = nullptr;

      uint32_t render_buffer = 0;
      uint32_t gbuffer_id = 0;

      bool valid = false;
  };

} // namespace other

#endif // !GBUFFER_HPP
