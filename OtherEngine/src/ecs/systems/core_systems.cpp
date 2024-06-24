/**
 * \file ecs/systems/core_systems.cpp
 **/
#include "ecs/systems/core_systems.hpp"

#include "ecs/components/serialization_data.hpp"
#include "ecs/entity.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/rigid_body_2d.hpp"

#include "physics/physics_defines.hpp"
#include "rendering/perspective_camera.hpp"

#include "physics/phyics_engine.hpp"

namespace other {

  CORE_SYSTEM(OnConstructEntity) {
    Entity e(context , entt);
    auto& tag = e.AddComponent<Tag>();
    tag.id = 0;
    tag.name = "[ Blank Entity ]";
    
    auto& transform = e.AddComponent<Transform>();
    transform.position = { 0.f , 0.f , 0.f };
    transform.erotation = { 0.f , 0.f , 0.f };
    transform.scale = { 1.f , 1.f , 1.f };

    /* auto& relationship = */ e.AddComponent<Relationship>();
    
    /* auto& serialization_data = */ e.AddComponent<SerializationData>();
  }
  
  CORE_SYSTEM(OnDestroyEntity) {
    context.view<Script>().each([](Script& script) {
      for (auto& [id , script] : script.scripts) {
        if (script->IsInitialized()) {
          script->Shutdown();
        }
      }
      script.scripts.clear();
    });
  }

  CORE_SYSTEM(OnCameraAddition) {
    Entity ent(context , entt);
    auto& camera = ent.GetComponent<Camera>();
    camera.camera = Ref<PerspectiveCamera>::Create();
  }

  void Initialize2DRigidBody(Ref<PhysicsWorld2D>& world , RigidBody2D& body , const Tag& tag ,const Transform& transform) {
    body.body_def = b2BodyDef {};
    body.body_def.position.x = transform.position.x;
    body.body_def.position.y = transform.position.y;
    body.body_def.angle = transform.position.z;
    body.body_def.linearDamping = body.linear_drag;
    body.body_def.angularDamping = body.angular_drag;
    body.body_def.gravityScale = body.gravity_scale;
    body.body_def.fixedRotation = body.fixed_rotation;
    body.body_def.bullet = body.bullet;
    body.body_def.userData.pointer = (uintptr_t)tag.id.Get();

    switch (body.type) {
      case STATIC:
        body.body_def.type = b2BodyType::b2_staticBody;
      break;
      case KINEMATIC:
        body.body_def.type = b2BodyType::b2_kinematicBody;
      break;
      case DYNAMIC:
        body.body_def.type = b2BodyType::b2_dynamicBody;
      break;
      default:
        OE_ERROR("Invalid Rigid Body 2D type, can not add to physics scene!");
        return;
    }

    body.physics_body = world->CreateBody(&body.body_def);
  }

  CORE_SYSTEM(OnRigidBody2DUpdate) {
    Ref<Scene> scene = PhysicsEngine::GetSceneContext();
    OE_ASSERT(scene != nullptr , "Somehow created a rigidy body 2D component without and active scene context");
    
    auto physics_world = scene->Get2DPhysicsWorld();
    OE_ASSERT(physics_world != nullptr , "Somehow create a rigid body 2D component without active 2D physics");

    Entity ent(context , entt);
    auto& body = ent.GetComponent<RigidBody2D>(); 

    auto& tag = ent.GetComponent<Tag>();
    auto& transform = ent.GetComponent<Transform>();
    
    Initialize2DRigidBody(physics_world , body , tag , transform); 
  }

} // namespace other
