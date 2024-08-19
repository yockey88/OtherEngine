/**
 * \file rendering/uniform.hpp
 **/
#ifndef OTHER_ENGINE_UNIFORM_HPP
#define OTHER_ENGINE_UNIFORM_HPP

#include <rendering/point_light.hpp>
#include <string>

#include <glm/gtc/type_ptr.hpp>

#include "core/uuid.hpp"
#include "core/ref_counted.hpp"
#include "core/buffer.hpp"
#include "math/vecmath.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/point_light.hpp"
#include "rendering/direction_light.hpp"

namespace other {
  
  enum ShaderStorageType {
    STD140 , STD430 ,
  };
  
  struct Uniform {
    std::string name = ""; 
    ValueType type;
    uint32_t arr_length = 1;
    Opt<size_t> size = std::nullopt; /// must be set for user types
  };
  
  struct ShaderStorage {
    ShaderStorageType type; 
    uint32_t binding_point;
    std::string name;
    std::map<UUID , Uniform> uniforms;
  };

  class UniformBuffer : public RefCounted {
    public:
      UniformBuffer(const std::string& name , const std::vector<Uniform> uniforms , uint32_t binding_point , 
                    BufferType type = BufferType::UNIFORM_BUFFER , BufferUsage usage = DYNAMIC_DRAW);
      ~UniformBuffer();

      const std::string& Name() const;

      bool Bound() const;

      void BindBase();
      void BindRange(size_t offset  = 0 , size_t size = 0);

      void Bind();

      void LoadFromBuffer(const Buffer& buffer);
      
      template <typename T>
      void SetUniform(const std::string_view name , const T& value , uint32_t index = 0) {
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
        CHECKGL();
        Unbind();

        CHECKGL();
      }

      void Unbind();

      void Clear();

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
      std::tuple<UniformData , bool , uint32_t> TryFind(const std::string_view name , uint32_t index);
      std::pair<UUID , UniformData> GetUniform(const std::string_view name);
  };

} // namespace other

#endif // !OTHER_ENGINE_UNIFORM_HPP
