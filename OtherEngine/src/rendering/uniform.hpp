/**
 * \file rendering/uniform.hpp
 **/
#ifndef OTHER_ENGINE_UNIFORM_HPP
#define OTHER_ENGINE_UNIFORM_HPP

#include <string>

#include <glm/gtc/type_ptr.hpp>

#include "core/uuid.hpp"
#include "core/ref_counted.hpp"

#include "rendering/rendering_defines.hpp"

namespace other {

  struct Uniform {
    std::string name = ""; 
    ValueType type;
  };

  class UniformBuffer : public RefCounted {
    public:
      UniformBuffer(const std::string& name , const std::vector<Uniform> uniforms , uint32_t binding_point , BufferUsage usage = STATIC_DRAW);
      ~UniformBuffer();

      const std::string& Name() const;
      uint32_t BindingPoint() const;

      bool Bound() const;

      void BindBase();
      void BindRange(size_t offset , size_t size);

      void Bind();
      
      template <typename T>
      void SetUniform(const std::string& name , const T& value) {
        auto [id , u_data] = GetUniform(name , sizeof(T));

        if (id.Get() == 0) {
          OE_ERROR("Failed to find uniform {}" , name);
          return;
        }

        Bind();
        glBufferSubData(GL_UNIFORM_BUFFER , u_data.offset , u_data.size , &value);
        Unbind();
      }

      template <> void SetUniform<glm::mat4>(const std::string& name , const glm::mat4& value) {
        auto [id , u_data] = GetUniform(name , sizeof(glm::mat4));
        if (id.Get() == 0) {
          OE_ERROR("Failed to find uniform {}" , name);
          return;
        }

        Bind();
        glBufferSubData(GL_UNIFORM_BUFFER , u_data.offset , u_data.size , glm::value_ptr(value));
        Unbind();
      }

      void Unbind();

    private:
      struct UniformData {
        Uniform uniform;
        UUID hash;
        size_t offset;
        size_t size;
      };

      std::map<UUID , UniformData> uniforms;

      const std::string name;
      bool bound = false;

      BufferUsage usage;
      uint32_t binding_point;
      uint32_t size = 0;
      uint32_t renderer_id = 0;

      std::pair<UUID , UniformData> GetUniform(const std::string& name , size_t val_size);
  };

} // namespace other

#endif // !OTHER_ENGINE_UNIFORM_HPP
