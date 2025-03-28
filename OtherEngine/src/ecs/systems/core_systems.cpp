/**
 * \file ecs/systems/core_systems.cpp
 **/
#include "ecs/systems/core_systems.hpp"

#include "application/app_state.hpp"

#include "ecs/component.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/entity_metatable.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/serialization_data.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/entity.hpp"

#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"

/**
 * \todo move the rest of this into scene
 **/

namespace other {

  /// TODO: references/pointers to components are invalidated after removing a component from an entity, this is not a problem right now but
  ///         has to be fixed in the future

  void OnConstructEntity(entt::registry& context, entt::entity entt) {
    auto& metatable = context.emplace<EntityMetatable>(entt);
    {
      auto& tag = context.emplace<Tag>(entt);
      tag.id = 0;
      tag.name = "[ Blank Entity ]";
      tag.handle = entt;
      metatable.components[TAG_COMPONENT_INDEX] = &tag;
    }
    {
      auto& transform = context.emplace<Transform>(entt);
      transform.position = { 0.f, 0.f, 0.f };
      transform.erotation = { 0.f, 0.f, 0.f };
      transform.scale = { 1.f, 1.f, 1.f };
      metatable.components[TRANSFORM_COMPONENT_INDEX] = &transform;
    }
    {
      auto& relationship = context.emplace<Relationship>(entt);
      metatable.components[RELATIONSHIP_COMPONENT_INDEX] = &relationship;
    }

    /* auto& serialization_data = */ context.emplace<SerializationData>(entt);
  }

  void OnDestroyEntity(entt::registry& context, entt::entity entt) {
    Entity e(context, entt);

    if (e.HasComponent<Script>()) {
      auto& script = e.GetComponent<Script>();
      script.ApiCall("OnStop");
      script.ApiCall("NativeStop");
      script.ApiCall("OnShutdown");
      script.ApiCall("NativeShutdown");
    }
  }

  void OnCameraAddition(entt::registry& context, entt::entity entt) {
    Entity ent(context, entt);
    auto& camera = ent.GetComponent<Camera>();
    camera.camera = NewRef<PerspectiveCamera>(Renderer::WindowSize());
    camera.camera_position = camera.camera->Position();
    camera.camera_direction = camera.camera->Direction();
    camera.camera_up = camera.camera->Up();
  }

  void OnAddScript(entt::registry& context, entt::entity entt) {
  }

  void OnAddModel(entt::registry& context, entt::entity entt) {
    Entity ent(context, entt);
    auto& mesh = ent.GetComponent<Mesh>();
  }

  void OnAddStaticModel(entt::registry& context, entt::entity entt) {
    Entity ent(context, entt);
    auto& mesh = ent.GetComponent<StaticMesh>();
  }

}  // namespace other
