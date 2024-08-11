
/**
 * \file rendering/layout.hpp
 **/
#ifndef OTHER_ENGINE_LAYOUT_HPP
#define OTHER_ENGINE_LAYOUT_HPP

#include <initializer_list>

#include "core/defines.hpp"

namespace other {
  
  struct VertexBufferElement {
    std::string name = "";
    ValueType type = ValueType::EMPTY;
    uint32_t size = 0;
    uint32_t offset = 0;

    uint32_t GetComponentCount();

    VertexBufferElement() {}
    VertexBufferElement(ValueType type , const std::string& name)
        : name(name) , type(type) {
      size = GetComponentCount();
      offset = GetComponentCount();
    }
  };

  class Layout {
    public:
      Layout() {}
      Layout(const std::initializer_list<VertexBufferElement>& elements);

      uint32_t Stride() const;
      const std::vector<VertexBufferElement> Elements() const;
      uint32_t Count() const;

      [[nodiscard]] std::vector<VertexBufferElement>::iterator begin();
      [[nodiscard]] std::vector<VertexBufferElement>::iterator end();
      [[nodiscard]] std::vector<VertexBufferElement>::const_iterator begin() const;
      [[nodiscard]] std::vector<VertexBufferElement>::const_iterator end() const;

    private:
      uint32_t stride = 0;
      std::vector<VertexBufferElement> elements;

      void CalculateOffsetAndStride();
  };

} // namespace other

#endif // !OTHER_ENGINE_LAYOUT_HPP
