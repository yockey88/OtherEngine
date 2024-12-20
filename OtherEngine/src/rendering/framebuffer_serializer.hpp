/**
 * \file rendering/framebuffer_serializer.hpp
 **/
#ifndef OTHER_ENGINE_FRAMEBUFFER_SERIALIZER_HPP
#define OTHER_ENGINE_FRAMEBUFFER_SERIALIZER_HPP

#include "core/ref.hpp"

#include "rendering/framebuffer.hpp"

#include "serialization/serializer.hpp"


namespace other {

  class FramebufferSerializer : public Serializer {
   public:
    FramebufferSerializer() = default;
    ~FramebufferSerializer() = default;

    void Serialize(const Ref<Framebuffer>& framebuffer);
    void Deserialize(Ref<Framebuffer>& framebuffer);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_FRAMEBUFFER_SERIALIZER_HPP