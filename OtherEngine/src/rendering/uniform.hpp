/**
 * \file rendering/uniform.hpp
 **/
#ifndef OTHER_ENGINE_UNIFORM_HPP
#define OTHER_ENGINE_UNIFORM_HPP

#include <ranges>
#include <string>

#include <glm/gtc/type_ptr.hpp>
#include <rendering/point_light.hpp>

#include "core/buffer.hpp"
#include "core/defines.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"
#include "core/writer_reader.hpp"
#include "math/vecmath.hpp"

#include "rendering/rendering_defines.hpp"

namespace other {

  enum ShaderStorageType {
    STD140,
    STD430,
  };

  enum InOutType {
    INPUT,
    OUTPUT,
  };

  struct Uniform {
    std::string name = "";
    ValueType type;
    uint32_t arr_length = 1;
    Opt<size_t> size = std::nullopt;  /// must be set for user types
  };

  template <typename T>
  struct UniformVariable {};

  struct ShaderStorage {
    ShaderStorageType type;
    uint32_t binding_point;
    std::string name;
    std::map<UUID, Uniform> uniforms;
  };

  struct InOutVar {
    InOutType in_out;

    std::string name;
    ValueType type;
  };

  struct InOutBlock {
    InOutType in_out;

    std::string name;
    std::map<UUID, InOutVar> vars;
  };

  struct DescriptorSet {
    std::vector<Uniform> uniforms;
    std::vector<InOutBlock> in_out_blocks;
  };

  class UniformBuffer : public RefCounted {
   public:
    UniformBuffer(const std::string& name, const std::vector<Uniform> uniforms, uint32_t binding_point, BufferType type = BufferType::UNIFORM_BUFFER, BufferUsage usage = DYNAMIC_DRAW);
    ~UniformBuffer();

    const std::string& Name() const;

    bool Bound() const;

    void BindBase();
    void BindRange(size_t offset = 0, size_t size = 0);

    void Bind();

    void LoadFromBuffer(const Buffer& buffer);

    template <typename T>
    void SetUniform(const std::string_view name, const T& value, uint32_t index = 0) {
      auto [u_data, success, offset] = TryFind(name, index);
      if (!success) {
        return;
      }
      OE_ASSERT(offset >= 0, "Uniform buffer not initialized");
      OE_ASSERT(u_data.size > 0, "Uniform buffer not initialized");
      OE_ASSERT((offset + u_data.size) <= size, "Uniform buffer overflow");

      Bind();
      if constexpr (glm_t<T>) {
        glBufferSubData(type, offset, u_data.size, glm::value_ptr(value));
      } else {
        glBufferSubData(type, offset, u_data.size, &value);
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

    std::map<UUID, UniformData> uniforms;

    const std::string name;
    bool bound = false;

    BufferType type;
    BufferUsage usage;

    uint32_t binding_point;
    uint32_t size = 0;
    uint32_t renderer_id = 0;

    uint32_t CalculateOffset(const UniformData& uniform, uint32_t index);
    std::tuple<UniformData, bool, uint32_t> TryFind(const std::string_view name, uint32_t index);
    std::pair<UUID, UniformData> GetUniform(const std::string_view name);
  };

  template <>
  struct Writer<Uniform> {
    std::ostream& operator()(std::ostream& os, const Uniform& data) {
      std::string type_str = fmtstr("{}", data.type) |
        std::views::transform([](char c) { return std::tolower(c); }) |
        std::ranges::to<std::string>();
      os << type_str << ":" << data.name;
      if (data.arr_length > 1) {
        os << ":" << data.arr_length;
      }
      return os;
    }
  };

  template <>
  struct Reader<Uniform> {
    Uniform operator()(std::istream& is) {
      ValueType type = Reader<ValueType>{}(is);
      std::string name = Reader<std::string>{}(is);

      uint32_t arr_length = 1;
      ClearWhitespace(is);
      if (is.peek() == ':') {
        is.ignore();
        arr_length = Reader<uint32_t>{}(is);
      }

      return Uniform{
        .name = name,
        .type = type,
        .arr_length = arr_length,
        .size = GetValueSize(type),
      };
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_UNIFORM_HPP
