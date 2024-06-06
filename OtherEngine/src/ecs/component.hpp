/**
 * \file ecs/component.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_HPP
#define OTHER_ENGINE_COMPONENT_HPP

#include <string>
#include <type_traits>
#include <array>
#include <string_view>

namespace other {

  constexpr static size_t kBuiltInComponents = 4;

  using ComponentTagPair = std::pair<std::string_view , size_t>;
  constexpr static std::array<ComponentTagPair , kBuiltInComponents> kBuiltInComponentTags = {
    ComponentTagPair{ "TAG" , 0 } , 
    ComponentTagPair{ "TRANSFORM" , 1 } , 
    ComponentTagPair{ "RELATIONSHIP" , 2 } ,
    ComponentTagPair{ "MESH" , 3 } ,
  };

  struct Component {
    virtual ~Component() = default;

    virtual std::string GetComponentName() const = 0;
  };

  template <typename T>
  concept component_type = std::is_base_of<Component, T>::value;

#define ECS_COMPONENT_CUSTOM_DTOR(name) \
  name() = default; \
  virtual std::string GetComponentName() const override { return "Component[" #name "]"; }

#define ECS_COMPONENT(name) \
  virtual ~name() override {} \
  ECS_COMPONENT_CUSTOM_DTOR(name);


} // namespace other

#endif // !OTHER_ENGINE_COMPONENT_HPP
