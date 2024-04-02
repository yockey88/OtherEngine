/**
 * \file ecs/component.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_HPP
#define OTHER_ENGINE_COMPONENT_HPP

#include <type_traits>

namespace other {

  struct Component {
    virtual ~Component() = default;
  };

  template <typename T>
  concept component_type = std::is_base_of<Component, T>::value;

} // namespace other

#endif // !OTHER_ENGINE_COMPONENT_HPP
