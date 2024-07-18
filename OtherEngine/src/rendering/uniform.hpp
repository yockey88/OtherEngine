/**
 * \file rendering/uniform.hpp
 **/
#ifndef OTHER_ENGINE_UNIFORM_HPP
#define OTHER_ENGINE_UNIFORM_HPP

#include <string>

#include <glm/gtc/type_ptr.hpp>

#include "core/uuid.hpp"
#include "core/ref_counted.hpp"
#include "math/vecmath.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/shader.hpp"

namespace other {

  class UniformBuffer : public RefCounted {
    public:
      UniformBuffer(const std::string& name , const std::vector<Uniform> uniforms , uint32_t binding_point , 
                    BufferType type = BufferType::UNIFORM_BUFFER , BufferUsage usage = DYNAMIC_DRAW);
      ~UniformBuffer();

      const std::string& Name() const;

      void BindShader(const Ref<Shader>& shader);

      bool Bound() const;

      void BindBase();
      void BindRange(size_t offset , size_t size);

      void Bind();
      
      template <typename T>
      void SetUniform(const std::string& name , const T& value , uint32_t index = 0) {
        auto [u_data , success , offset] = TryFind(name , index);
        if (!success) {
          return;
        }

        Bind();
        if constexpr (glm_t<T>) {
          glBufferSubData(type , offset , u_data.size , glm::value_ptr(value));
        } else {
          glBufferSubData(type , offset , u_data.size , &value);
        }
        Unbind();

        CHECKGL();
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

      BufferType type;
      BufferUsage usage;

      uint32_t binding_point;
      uint32_t size = 0;
      uint32_t renderer_id = 0;

      uint32_t CalculateOffset(const UniformData& uniform , uint32_t index);
      std::tuple<UniformData , bool , uint32_t> TryFind(const std::string& name , uint32_t index);
      std::pair<UUID , UniformData> GetUniform(const std::string& name);
  };

} // namespace other

#endif // !OTHER_ENGINE_UNIFORM_HPP
