
/**
 * \file rendering/layout.hpp
 **/
#ifndef OTHER_ENGINE_LAYOUT_HPP
#define OTHER_ENGINE_LAYOUT_HPP

#include <initializer_list>

#include "core/defines.hpp"
#include "core/writer_reader.hpp"

#include "rendering/uniform.hpp"

namespace other {

  struct VertexBufferElement {
    std::string name = "";
    ValueType type = ValueType::EMPTY_TYPE;
    uint32_t size = 0;
    uint32_t offset = 0;

    uint32_t GetComponentCount();

    VertexBufferElement() {}
    VertexBufferElement(ValueType type, const std::string& name)
        : name(name), type(type) {
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
    std::vector<uint32_t> GetRawLayout() const;
    uint32_t Count() const;

    [[nodiscard]] std::vector<VertexBufferElement>::iterator begin();
    [[nodiscard]] std::vector<VertexBufferElement>::iterator end();
    [[nodiscard]] std::vector<VertexBufferElement>::const_iterator begin() const;
    [[nodiscard]] std::vector<VertexBufferElement>::const_iterator end() const;

   private:
    friend struct Writer<Layout>;
    friend struct Reader<Layout>;

    uint32_t stride = 0;
    std::vector<VertexBufferElement> elements;

    void CalculateOffsetAndStride();
  };

  template <>
  struct Writer<VertexBufferElement> {
    std::ostream& operator()(std::ostream& os, const VertexBufferElement& data) {
      std::string type_str = fmtstr("{}", data.type) |
        std::views::transform([](char c) { return std::tolower(c); }) |
        std::ranges::to<std::string>();
      os << type_str << ":" << data.name;
      return os;
    }
  };

  template <>
  struct Reader<VertexBufferElement> {
    VertexBufferElement operator()(std::istream& is) {
      ValueType type = Reader<ValueType>{}(is);
      std::string name = Reader<std::string>{}(is);

      return VertexBufferElement{ type, name };
    }
  };

  template <>
  struct Writer<Layout> {
    std::ostream& operator()(std::ostream& os, const Layout& data) {
      Writer<std::vector<VertexBufferElement>>{}(os, data.elements);
      return os;
    }
  };

  template <>
  struct Reader<Layout> {
    Layout operator()(std::istream& is) {
      Layout data;
      BeginReadList(is);
      while (is.peek() != '}') {
        data.elements.push_back(Reader<VertexBufferElement>{}(is));

        ClearWhitespace(is);
        if (is.peek() == ',') {
          is.ignore();
        }
      }
      EndReadList(is);
      return data;
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_LAYOUT_HPP
