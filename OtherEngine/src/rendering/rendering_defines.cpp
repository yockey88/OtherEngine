/**
 * \file rendering/rendering_defines.cpp
 **/
#include "rendering/rendering_defines.hpp"

namespace other {
  
  uint32_t VertexBufferElement::GetComponentCount() {
    switch (type) {
      case VEC2:
        return 2;
      case VEC3:
        return 3;
      case VEC4:
        return 4;
      
      case MAT3:
        return 3 * 3;
      case MAT4:
        return 4 * 4;

      case EMPTY:
        return 0;

      default:
        return 1;
    }
  }
      
  Layout::Layout(const std::initializer_list<VertexBufferElement>& elements)
      : elements(elements) {
    CalculateOffsetAndStride();
  }
      
  uint32_t Layout::Stride() const {
    return stride;
  }

  const std::vector<VertexBufferElement> Layout::Elements() const {
    return elements;
  }
  
  uint32_t Layout::Count() const {
    return elements.size();
  }
    
  void Layout::CalculateOffsetAndStride() {
    for (auto& e : elements) {
      stride += e.GetComponentCount();
    }
  }

} // namespace other
