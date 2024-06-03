/**
 * \file ecs/component.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_HPP
#define OTHER_ENGINE_COMPONENT_HPP

#include <string>
#include <type_traits>

namespace other {

  struct Component {
    virtual ~Component() = default;

    virtual std::string GetComponentName() const = 0;
  };

  template <typename T>
  concept component_type = std::is_base_of<Component, T>::value;

#define ECS_COMPONENT(name) \
  name() = default; \
  virtual ~ ##name() override {} \
  virtual std::string GetComponentName() const override { return "Component[" #name "]"; }

} // namespace other

#endif // !OTHER_ENGINE_COMPONENT_HPP
