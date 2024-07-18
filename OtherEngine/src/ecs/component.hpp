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

  constexpr static size_t kNumComponents = 9;

  constexpr static int32_t kTagIndex = 0;
  constexpr static int32_t kTransformIndex = 1;
  constexpr static int32_t kRelationshipIndex = 2;
  constexpr static int32_t kMeshIndex = 3;
  constexpr static int32_t kStaticMeshIndex = 4;
  constexpr static int32_t kScriptIndex = 5;
  constexpr static int32_t kCameraIndex = 6;
  constexpr static int32_t kRigidBody2DIndex = 7;
  constexpr static int32_t kCollider2DIndex = 8;

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
  };

  class ComponentDataBase {
    public:
      static std::string GetComponentTagUc(size_t idx);
      static std::string GetComponentTagLc(size_t idx);
      static int32_t GetComponentIdxFromTag(const std::string_view tag);

    private:
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
