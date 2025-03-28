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

#include "serialization/object_serializer.hpp"

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

  /// integers signed because of 'invisible components'
  ///   default components attached to all entities that are for internal engine use
  enum ComponentIndex : int32_t {
    /// implicit components, always present
    TAG_COMPONENT_INDEX = 0,
    TRANSFORM_COMPONENT_INDEX,
    RELATIONSHIP_COMPONENT_INDEX,

    /// explicit components
    MESH_COMPONENT_INDEX,
    STATICMESH_COMPONENT_INDEX,
    SCRIPT_COMPONENT_INDEX,
    CAMERA_COMPONENT_INDEX,
    RIGIDBODY2D_COMPONENT_INDEX,
    COLLIDER2D_COMPONENT_INDEX,
    RIGIDBODY_COMPONENT_INDEX,
    COLLIDER_COMPONENT_INDEX,
    PHYSICS_OBJECT_COMPONENT_INDEX,
    LIGHTSOURCE_COMPONENT_INDEX,
    TERRAIN_COMPONENT_INDEX,

    NUM_COMPONENTS,
    INVALID_COMPONENT = NUM_COMPONENTS
  };
  /** invisible components
   *                       kSerializationData = -1
   *                       kSceneComponent = -2
   *                       kEntityMetaTable = -3
   *                       kNullComponent = -999
   **/

  constexpr static size_t kNumComponents = NUM_COMPONENTS;
  struct ComponentTag {
    std::string_view name;
    int32_t idx;

    constexpr ComponentTag(const std::string_view name, int32_t idx)
        : name(name), idx(idx) {}

    constexpr auto operator<=>(const ComponentTag& other) const = default;
  };
  constexpr static std::array<ComponentTag, NUM_COMPONENTS> kComponentTags = {
    ComponentTag{ "tag", TAG_COMPONENT_INDEX },
    ComponentTag{ "transform", TRANSFORM_COMPONENT_INDEX },
    ComponentTag{ "relationship", RELATIONSHIP_COMPONENT_INDEX },
    ComponentTag{ "mesh", MESH_COMPONENT_INDEX },
    ComponentTag{ "static-mesh", STATICMESH_COMPONENT_INDEX },
    ComponentTag{ "script", SCRIPT_COMPONENT_INDEX },
    ComponentTag{ "camera", CAMERA_COMPONENT_INDEX },
    ComponentTag{ "rigid-body-2d", RIGIDBODY2D_COMPONENT_INDEX },
    ComponentTag{ "collider-2d", COLLIDER2D_COMPONENT_INDEX },
    ComponentTag{ "rigid-body", RIGIDBODY_COMPONENT_INDEX },
    ComponentTag{ "collider", COLLIDER_COMPONENT_INDEX },
    ComponentTag{ "physics-object", PHYSICS_OBJECT_COMPONENT_INDEX },
    ComponentTag{ "light-source", LIGHTSOURCE_COMPONENT_INDEX },
    ComponentTag{ "terrain", TERRAIN_COMPONENT_INDEX }
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
    virtual void Serialize(ByteBuffer& buffer) = 0;
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
  }                                                                                     \
  virtual void Serialize(ByteBuffer& buffer) override {                                 \
    n##Snapshotter serializer;                                                          \
    serializer.Write(buffer, *this);                                                    \
  }

#define INVISIBLE_ECS_COMPONENT(n, idx)                                                 \
  ECHO_REFLECT();                                                                       \
  n() : Component(idx) {}                                                               \
  virtual ~n() override {}                                                              \
  virtual std::string GetComponentName() const override { return "Component[" #n "]"; } \
  virtual std::string GetRawComponentName() const override {                            \
    if (component_idx < 0) {                                                            \
      return "anonymous-component";                                                     \
    }                                                                                   \
    return std::string{ kComponentTags[component_idx].name };                           \
  }                                                                                     \
  virtual void Serialize(ByteBuffer& buffer) override {}

  struct NullComponent : public Component {
    INVISIBLE_ECS_COMPONENT(NullComponent, -999);
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
      // comp.material;
    };

  template <typename C>
  concept DebugRenderableComp =
    ComponentType<C> &&
    (RenderableComp<C> ||
     requires(C comp) {
       comp.handle;  /// asset handle
                     //  comp.material;
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
