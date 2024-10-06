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
#include "ecs/components/mesh.hpp"

#include "physics/physics_defines.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"

#include "physics/phyics_engine.hpp"
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>

namespace other {

  CORE_SYSTEM(OnConstructEntity) {
    Entity e(context , entt);
    auto& tag = e.AddComponent<Tag>();
    tag.id = 0;
    tag.name = "[ Blank Entity ]";
    tag.handle = entt;
    
    auto& transform = e.AddComponent<Transform>();
    transform.position = { 0.f , 0.f , 0.f };
    transform.erotation = { 0.f , 0.f , 0.f };
    transform.scale = { 1.f , 1.f , 1.f };

    /* auto& relationship = */ e.AddComponent<Relationship>();
    
    /* auto& serialization_data = */ e.AddComponent<SerializationData>();
  }
  
  CORE_SYSTEM(OnDestroyEntity) {
    Entity e(context , entt);

    if (e.HasComponent<Script>()) {
      auto& script = e.GetComponent<Script>();
      script.ApiCall("OnStop");
      script.ApiCall("NativeStop");
      script.ApiCall("OnShutdown");
      script.ApiCall("NativeShutdown");
      script.Clear();
    }
  }

  CORE_SYSTEM(OnCameraAddition) {
    Entity ent(context , entt);
    auto& camera = ent.GetComponent<Camera>();
    camera.camera = NewRef<PerspectiveCamera>(Renderer::WindowSize());
  }
  
  CORE_SYSTEM(OnAddModel) {
    Entity ent(context , entt);
    auto& mesh = ent.GetComponent<Mesh>();
    mesh.material = Material {
      .color = { 0.5f , 0.6f , 0.7f , 1.f } ,
      .shininess = 16.f ,
    };
  }

  CORE_SYSTEM(OnAddStaticModel) {
    Entity ent(context , entt);
    auto& mesh = ent.GetComponent<StaticMesh>();
    mesh.material = Material {
      .color = { 0.5f , 0.6f , 0.7f , 1.f } ,
      .shininess = 16.f ,
    };
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

    body.mass_data = body.physics_body->GetMassData();
    body.mass_data.mass = body.mass;

    body.physics_body->SetMassData(&body.mass_data);
  }
  
  void Initialize2DCollider(Ref<PhysicsWorld2D>& world , RigidBody2D& body , Collider2D& collider , const Transform& transform) {
    b2PolygonShape shape;
    shape.SetAsBox(transform.scale.x * collider.size.x , transform.scale.y * collider.size.y);

    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.density = collider.density;
    fixture_def.friction = collider.friction;

    collider.fixture = body.physics_body->CreateFixture(&fixture_def);
  }

  CORE_SYSTEM(OnRigidBody2DUpdate) {
    Ref<Scene> scene = PhysicsEngine::GetSceneContext();
    OE_ASSERT(scene != nullptr , "Somehow updated a rigidy body 2D component without and active scene context");
    
    auto physics_world = scene->Get2DPhysicsWorld();
    OE_ASSERT(physics_world != nullptr , "Somehow updated a rigid body 2D component without active 2D physics");

    Entity ent(context , entt);
    auto& body = ent.GetComponent<RigidBody2D>(); 

    auto& tag = ent.GetComponent<Tag>();
    auto& transform = ent.GetComponent<Transform>();
    
    Initialize2DRigidBody(physics_world , body , tag , transform); 
  }
  

  CORE_SYSTEM(OnCollider2DUpdate) {
    Ref<Scene> scene = PhysicsEngine::GetSceneContext();
    OE_ASSERT(scene != nullptr , "Somehow updated a collider 2D component without an active scene context");
    
    auto physics_world = scene->Get2DPhysicsWorld();
    OE_ASSERT(physics_world != nullptr , "Somehow updated a collider 2D component without active 2D physics");

    Entity ent(context , entt);
    if (!ent.HasComponent<RigidBody2D>()) {
      ent.AddComponent<RigidBody2D>();
    }

    auto& body = ent.GetComponent<RigidBody2D>(); 
    auto& collider = ent.GetComponent<Collider2D>();
    auto& transform = ent.GetComponent<Transform>();

    Initialize2DCollider(physics_world , body , collider , transform);
  }
  
  void InitializeRigidBody(Ref<PhysicsWorld>& world , RigidBody& body , const Tag& tag , const Transform& transform) {
  }

  void InitializeCollider(Ref<PhysicsWorld>& world , RigidBody& body , Collider& collider , const Transform& transform) {
  }
  
  CORE_SYSTEM(OnRigidBodyUpdate) {
    Ref<Scene> scene = PhysicsEngine::GetSceneContext();
    OE_ASSERT(scene != nullptr , "Somehow updated a rigid body component without an active scene context");
    
    auto physics_world = scene->GetPhysicsWorld();
    OE_ASSERT(physics_world != nullptr , "Somehow updated a rigid body component without active 3D physics");

    Entity ent(context , entt);
    auto& body = ent.GetComponent<RigidBody>();

    auto& tag = ent.GetComponent<Tag>();
    auto& transform = ent.GetComponent<Transform>();

    InitializeRigidBody(physics_world , body , tag , transform);
  }

  CORE_SYSTEM(OnColliderUpdate) {
    Ref<Scene> scene = PhysicsEngine::GetSceneContext();
    OE_ASSERT(scene != nullptr , "Somehow updated a collider component without an active scene context");
    
    auto physics_world = scene->GetPhysicsWorld();
    OE_ASSERT(physics_world != nullptr , "Somehow updated a collider component without active 3D physics");

    Entity ent(context , entt);
    if (!ent.HasComponent<RigidBody>()) {
      ent.AddComponent<RigidBody>();
    }

    auto& body = ent.GetComponent<RigidBody>(); 
    auto& collider = ent.GetComponent<Collider>();
    auto& transform = ent.GetComponent<Transform>();

    InitializeCollider(physics_world , body , collider , transform);
  }

} // namespace other
