/**
 * \file ecs/systems/core_systems.cpp
 **/
#include "ecs/systems/core_systems.hpp"

#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/EActivation.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>

#include "application/app_state.hpp"

#include "ecs/components/camera.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/serialization_data.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/entity.hpp"

#include "physics/physics_defines.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"

namespace other {

  void OnConstructEntity(entt::registry& context, entt::entity entt) {
    auto& tag = context.emplace<Tag>(entt);
    tag.id = 0;
    tag.name = "[ Blank Entity ]";
    tag.handle = entt;

    auto& transform = context.emplace<Transform>(entt);
    transform.position = { 0.f, 0.f, 0.f };
    transform.erotation = { 0.f, 0.f, 0.f };
    transform.scale = { 1.f, 1.f, 1.f };

    /* auto& relationship = */ context.emplace<Relationship>(entt);

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
