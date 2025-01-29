/**
 * \file physics/3D/jolt/jolt_world.cpp
 **/
#include "physics/3D/jolt/jolt_world.hpp"

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include "scene/scene.hpp"

#include "physics/physics_defines.hpp"

namespace other {

  constexpr static JPH::uint kMaxBodies = 1024;
  constexpr static JPH::uint kNumMutexes = 0;
  constexpr static JPH::uint kMaxPairs = 1024;
  constexpr static JPH::uint kMaxContactConstraints = 1024;
  constexpr static JPH::uint kCollisionSteps = 1;

  static void JoltTrace(const char* in_msg, ...) {
    char buffer[1024];

    va_list args;
    va_start(args, in_msg);
    vprintf(buffer, args);
    va_end(args);

    std::string in_msg_str = in_msg;
    std::string msg = buffer;
    OE_TRACE("[Jolt] > {}", in_msg_str, msg);
  }

  static bool JoltAssert(const char* in_expr, const char* in_msg, const char* in_file, JPH::uint in_line) {
    std::string expr = "";
    std::string msg = "";
    std::string file = "";
    if (in_expr != nullptr) {
      expr = in_expr;
    }
    if (in_msg != nullptr) {
      msg = in_msg;
    }
    if (in_file != nullptr) {
      file = in_file;
    }

    OE_ASSERT(false, "Jolt Assert Failed: {}:{}: ({}) {}", file, in_line, expr, msg);
    // Breakpoint
    return true;
  };

  JoltWorld::JoltWorld(Scene* scene_ctx)
      : PhysicsWorld(scene_ctx) {
    // Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
    // This needs to be done before any other Jolt function is called.
    JPH::RegisterDefaultAllocator();

    // Install trace and assert callbacks
    JPH::Trace = JoltTrace;
    JPH::AssertFailed = JoltAssert;

    // Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
    // It is not directly used in this example but still required.
    JPH::Factory::sInstance = new JPH::Factory();

    // Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
    // If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
    // If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
    JPH::RegisterTypes();

    /// TODO: somehow choose the best amount of pre-allocated memory for the scene this world
    ///       is owned by ??? is there something we can calculate to know the optimal amount of memory per scene?
    ///       would this be part of mesh baking and scene compilation?
    temp_alloc = new JPH::TempAllocatorImpl(300 * 1024 * 1024);
    thread_pool = NewScope<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

    system = NewScope<JPH::PhysicsSystem>();
    system->Init(kMaxBodies, kNumMutexes, kMaxPairs, kMaxContactConstraints, broad_phase_layer_handler, broad_phase_layer_filter, obj_layer_filter);

    activation_listener = NewScope<ActivationListener>();
    system->SetBodyActivationListener(activation_listener.get());

    contact_listener = NewScope<ContactListener>();
    system->SetContactListener(contact_listener.get());
  }

  JoltWorld::~JoltWorld() {
    system = nullptr;
    thread_pool = nullptr;

    delete temp_alloc;
    temp_alloc = nullptr;
  }

  void JoltWorld::ResetSimulation(Scene* scene) {
    // system->Reset();
  }

  void JoltWorld::Simulate(float ts) {
    // JPH_PROFILE("JoltWorld::Simulate");
    system->OptimizeBroadPhase();

    /*
                if (m_Accumulator > m_FixedTimeStep)
                        m_Accumulator = 0.0f;

                m_Accumulator += ts;
                if (m_Accumulator < m_FixedTimeStep)
                {
                        m_CollisionSteps = 0;
                        return;
                }

                m_CollisionSteps = (uint32_t)(m_Accumulator / m_FixedTimeStep);
                m_Accumulator -= (float)m_CollisionSteps * m_FixedTimeStep;
    */

    system->Update(ts, kCollisionSteps, temp_alloc, thread_pool.get());
  }

  void JoltWorld::CreateBody(Entity& entity) {
    auto& rigid_body = entity.GetComponent<RigidBody>();
    auto& collider = entity.GetComponent<Collider>();
    auto& transform = entity.GetComponent<Transform>();
    auto& tag = entity.GetComponent<Tag>();

    if (collider.shape_idx >= PhysicsShape::Shape::NUM_PHYSICS_SHAPES) {
      OE_ERROR("Invalid collider shape type, defaulting to box");
      collider.shape_idx = PhysicsShape::Shape::BOX;
    }

    CreateColliders(collider, transform);
    JPH::Shape* shape = (JPH::Shape*)collider.shape->NativeShape();

    JPH::EMotionType motion_type = JPH::EMotionType::Static;
    switch (rigid_body.type) {
      case PhysicsBodyType::STATIC:
        motion_type = JPH::EMotionType::Static;
        break;
      case PhysicsBodyType::KINEMATIC:
        motion_type = JPH::EMotionType::Kinematic;
        break;
      case PhysicsBodyType::DYNAMIC:
        motion_type = JPH::EMotionType::Dynamic;
        break;
      default:
        OE_ASSERT(false, "Invalid rigid body type");
    }

    glm::quat rot = glm::normalize(transform.qrotation);
    // clang-format off
    JPH::BodyCreationSettings body_settings(shape, JPH::RVec3(transform.position.x, transform.position.y, transform.position.z), 
                                            JPH::Quat(rot.x, rot.y, rot.z, rot.w), motion_type, JPH::ObjectLayer(rigid_body.layer_id));
    // clang-format on
    body_settings.mIsSensor = rigid_body.is_trigger;
    body_settings.mAllowDynamicOrKinematic = rigid_body.enable_dynamic_type_change;
    body_settings.mAllowSleeping = true;

    JPH::BodyInterface& body_interface = system->GetBodyInterface();
    JPH::Body* body = body_interface.CreateBody(body_settings);
    OE_ASSERT(body != nullptr, "Failed to create dynamic body");

    body->SetUserData(tag.id.Get());
    rigid_body.physics_body = NewRef<JoltBody>(body_interface, body);

    auto [itr, res] = native_bodies.insert({ body, tag.id });
    OE_ASSERT(res, "Failed to insert body into native bodies map");

    auto [itr2, res2] = bodies.insert({ tag.id, rigid_body.physics_body });
    OE_ASSERT(res2, "Failed to insert body into bodies map");
  }

  void JoltWorld::DestroyBody(Entity& entity) {
    auto& rigid_body = entity.GetComponent<RigidBody>();
    auto& collider = entity.GetComponent<Collider>();

    rigid_body.physics_body = nullptr;
    collider.shape = nullptr;
  }

  Ref<PhysicsShape> JoltWorld::CreateBoxShape(const glm::vec3& half_extents) {
    JPH::Ref<JPH::BoxShapeSettings> shape_settings = new JPH::BoxShapeSettings(JPH::RVec3(half_extents.x, half_extents.y, half_extents.z));

    glm::vec3 scale = half_extents * 2.f;
    JPH::RotatedTranslatedShapeSettings rotated_shape_settings(JPH::RVec3(scale.x, scale.y, scale.z), JPH::Quat::sIdentity(), shape_settings);
    JPH::ShapeSettings::ShapeResult result = rotated_shape_settings.Create();
    if (result.HasError()) {
      OE_ERROR("Failed to create box shape : {}", result.GetError());
      return nullptr;
    }

    JPH::Ref<JPH::Shape> ref = result.Get();
    JPH::Ref<JPH::RotatedTranslatedShape> rt_shape = JPH::Ref<JPH::RotatedTranslatedShape>(static_cast<JPH::RotatedTranslatedShape*>(const_cast<JPH::Shape*>(ref.GetPtr())));
    return NewRef<JoltBoxShape>(shape_settings, rt_shape, this);
  }

  Ref<PhysicsShape> JoltWorld::CreateSphereShape(float radius) {
    return nullptr;
  }

  Ref<PhysicsShape> JoltWorld::CreateCapsuleShape(float radius, float height) {
    JPH::Ref<JPH::CapsuleShapeSettings> shape_settings = new JPH::CapsuleShapeSettings(radius, height);

    JPH::RotatedTranslatedShapeSettings rotated_shape_settings(JPH::RVec3(radius, height, radius), JPH::Quat::sIdentity(), shape_settings);
    JPH::ShapeSettings::ShapeResult result = rotated_shape_settings.Create();
    if (result.HasError()) {
      OE_ERROR("Failed to create capsule shape : {}", result.GetError());
      return nullptr;
    }

    JPH::Ref<JPH::Shape> ref = result.Get();
    JPH::Ref<JPH::RotatedTranslatedShape> rt_shape = JPH::Ref<JPH::RotatedTranslatedShape>(static_cast<JPH::RotatedTranslatedShape*>(const_cast<JPH::Shape*>(ref.GetPtr())));
    return NewRef<JoltCapsuleShape>(shape_settings, rt_shape, this);
  }

  Ref<PhysicsShape> JoltWorld::CreateConvexMeshShape(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, uint32_t num_faces) {
    return nullptr;
  }

  Ref<PhysicsShape> JoltWorld::CreateConcaveMeshShape(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, uint32_t num_faces) {
    return nullptr;
  }

  void JoltWorld::CreateColliders(Collider& collider, Transform& transform) {
    switch (collider.shape_idx) {
      case PhysicsShape::Shape::BOX: {
        glm::vec3 half_extents = transform.scale / 2.f;
        collider.shape = CreateBoxShape(half_extents);
      } break;
      case PhysicsShape::Shape::SPHERE: {
        collider.shape = CreateSphereShape(transform.scale.x / 2.f);
      } break;
      case PhysicsShape::Shape::CAPSULE: {
        collider.shape = CreateCapsuleShape(transform.scale.x / 2.f, transform.scale.y);
      } break;
      case PhysicsShape::Shape::CONVEX_MESH:
      case PhysicsShape::Shape::CONCAVE_MESH:
        OE_ASSERT(false, "Mesh shapes not implemented yet");
        break;
      default:
        OE_ASSERT(false, "Invalid collider shape type");
    }
    OE_ASSERT(collider.shape != nullptr, "Failed to create collider shape");
  }

}  // namespace other
