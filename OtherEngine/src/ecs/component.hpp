/**
 * \file ecs/component.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_HPP
#define OTHER_ENGINE_COMPONENT_HPP

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#include <entt/core/type_info.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/group.hpp>
#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>

#include <reflection/echo_defines.hpp>
#include <reflection/reflected_object.hpp>

#include "core/uuid.hpp"

namespace echo = dotother::echo;

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

  constexpr static size_t kNumComponents = 12;

  /// integers signed because of 'invisible components'
  ///   default components attached to all entities that are for internal engine use
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
  constexpr static int32_t kLightSourceIndex = 11;
  /** invisible components
   *                       kSerializationData = -1
   *                       kNullComponent = -999
   **/

  struct ComponentTag {
    std::string_view name;
    int32_t idx;

    constexpr ComponentTag(const std::string_view name, int32_t idx)
        : name(name), idx(idx) {}

    constexpr auto operator<=>(const ComponentTag& other) const = default;
  };
  //= std::pair<std::string_view, size_t>;
  constexpr static std::array<ComponentTag, kNumComponents> kComponentTags = {
    ComponentTag{ "tag", kTagIndex },
    ComponentTag{ "transform", kTransformIndex },
    ComponentTag{ "relationship", kRelationshipIndex },
    ComponentTag{ "mesh", kMeshIndex },
    ComponentTag{ "static-mesh", kStaticMeshIndex },
    ComponentTag{ "script", kScriptIndex },
    ComponentTag{ "camera", kCameraIndex },
    ComponentTag{ "rigid-body-2d", kRigidBody2DIndex },
    ComponentTag{ "collider-2d", kCollider2DIndex },
    ComponentTag{ "rigid-body", kRigidBodyIndex },
    ComponentTag{ "collider", kColliderIndex },
    ComponentTag{ "light-source", kLightSourceIndex },
  };

  class Entity;

  struct Component : public echo::reflectable {
    Component(int32_t idx) : component_idx(idx) {}
    virtual ~Component() = default;

    UUID parent_uuid = 0;
    entt::entity parent_id = entt::null;
    Entity* parent_handle = nullptr;

    int32_t component_idx = -1;

    uint64_t GetId() const { return parent_uuid.Get(); }

    virtual std::string GetComponentName() const = 0;
    virtual std::string GetRawComponentName() const = 0;
  };

#define ECS_COMPONENT(n, idx)                                                           \
  ECHO_REFLECT();                                                                       \
  n() : Component(idx) {}                                                               \
  virtual ~n() override {}                                                              \
  virtual std::string GetComponentName() const override { return "Component[" #n "]"; } \
  virtual std::string GetRawComponentName() const override {                            \
    if (component_idx < 0) {                                                            \
      return "anonymous-component";                                                     \
    }                                                                                   \
    return std::string{ kComponentTags[component_idx].name };                           \
  }

  struct NullComponent : public Component {
    ECS_COMPONENT(NullComponent, -999);
  };

  /// Useful concepts

  template <typename T>
  concept ComponentType = std::is_base_of<Component, T>::value;

  template <typename T>
  concept NullComponentType = ComponentType<T> && std::same_as<T, NullComponent>;

  template <typename C>
  concept RenderableComp =
    ComponentType<C> &&
    requires(C comp) {
      comp.handle;  /// asset handle
      comp.material;
    };

  template <typename C>
  concept DebugRenderableComp =
    ComponentType<C> &&
    (RenderableComp<C> ||
     requires(C comp) {
       comp.handle;  /// asset handle
       comp.material;
     });

  template <ComponentType RC, ComponentType TC = NullComponent, ComponentType EC = NullComponent>
  using SystemGroup = entt::basic_group<
    entt::owned_t<entt::registry::storage_for_type<RC>>,
    entt::get_t<entt::registry::storage_for_type<TC>>,
    entt::exclude_t<entt::registry::storage_for_type<EC>>>;

}  // namespace other

ECHO_TYPE(
  type(other::Component, refl::attr::bases<dotother::echo::reflectable>),
  func(GetId, property("id")),
  func(GetComponentName, property("name")),
  func(GetRawComponentName, property("raw_name")),
  field(parent_handle),
  field(component_idx)
);

ECHO_TYPE(
  type(other::NullComponent, refl::attr::bases<other::Component>)
);

#endif  // !OTHER_ENGINE_COMPONENT_HPP
