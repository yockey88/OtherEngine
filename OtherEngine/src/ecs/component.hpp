/**
 * \file ecs/component.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_HPP
#define OTHER_ENGINE_COMPONENT_HPP

#include <cstdint>
#include <string>
#include <type_traits>
#include <array>
#include <string_view>
#include <map>

#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>

#include "core/uuid.hpp"

namespace other {

/**
 * FIXME: reduce the number of steps to implement a component
 *
 * When adding a new component these are the places where something must change
 *  - implement the component and its corresponding serializer
 *    - while implementing the serializer, add any necessary key/values to core/config_keys.hpp file
 *  - add the next index to the list below and increment kNumComponents by one
 *  - add the corresponding tag-pair to the array and note there can be no collisions between names
 *  - add a ctor function for the component serializer list in ecs/systems/entity_serialization.cpp
 *      (this must have the same index as specified below)
 *  - add a AddComponentButton in editor/entity_properties.hpp 
 *
 *  - (optional) implement any UI functionality in ecs/systems/component_gui and add the call to DrawComponent<>(name , func)
 *      in the editor/entity_properties.hpp
 **/

  constexpr static size_t kNumComponents = 11;

  constexpr static int32_t kTagIndex = 0;
  constexpr static int32_t kTransformIndex = 1;
  constexpr static int32_t kRelationshipIndex = 2;
  constexpr static int32_t kMeshIndex = 3;
  constexpr static int32_t kStaticMeshIndex = 4;
  constexpr static int32_t kScriptIndex = 5;
  constexpr static int32_t kCameraIndex = 6;
  constexpr static int32_t kRigidBody2DIndex = 7;
  constexpr static int32_t kCollider2DIndex = 8;
  constexpr static int32_t kRigidBodyIndex = 9;
  constexpr static int32_t kColliderIndex = 10;

  using ComponentTagPair = std::pair<std::string_view , size_t>;
  constexpr static std::array<ComponentTagPair , kNumComponents> kComponentTags = {
    ComponentTagPair{ "TAG" , kTagIndex } , 
    ComponentTagPair{ "TRANSFORM" , kTransformIndex } , 
    ComponentTagPair{ "RELATIONSHIP" , kRelationshipIndex } ,
    ComponentTagPair{ "MESH" , kMeshIndex } ,
    ComponentTagPair{ "STATIC-MESH" , kStaticMeshIndex } ,
    ComponentTagPair{ "SCRIPT" , kScriptIndex } ,
    ComponentTagPair{ "CAMERA" , kCameraIndex } ,
    ComponentTagPair{ "RIGID-BODY-2D" , kRigidBody2DIndex } ,
    ComponentTagPair{ "COLLIDER-2D" , kCollider2DIndex } ,
    ComponentTagPair{ "RIGID-BODY" , kRigidBodyIndex } ,
    ComponentTagPair{ "COLLIDER" , kColliderIndex } ,
  };

  class ComponentDataBase {
    public:
      static std::string GetComponentTagUc(size_t idx);
      static std::string GetComponentTagLc(size_t idx);
      static int32_t GetComponentIdxFromTag(const std::string_view tag);
  };

  struct Component {
    // Component() {}
    Component(int32_t idx) : component_idx(idx) {}
    virtual ~Component() = default;

    int32_t component_idx = -1;
    virtual std::string GetComponentName() const = 0;
  };

  template <typename T>
  concept component_type = std::is_base_of<Component, T>::value;

#define ECS_COMPONENT_CUSTOM_DTOR(name , idx) \
  name() : Component(idx) {} \
  virtual std::string GetComponentName() const override { return "Component[" #name "]"; }

#define ECS_COMPONENT(name , idx) \
  virtual ~name() override {} \
  ECS_COMPONENT_CUSTOM_DTOR(name , idx);


} // namespace other

#endif // !OTHER_ENGINE_COMPONENT_HPP
