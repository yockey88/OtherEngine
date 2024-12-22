/**
 * \file physics/3D/react/react_world.cpp
 **/
#include "physics/3D/react/react_world.hpp"

#include <reactphysics3d/body/RigidBody.h>
#include <reactphysics3d/mathematics/Vector3.h>

#include "application/app_state.hpp"

#include "scene/scene.hpp"

#include "physics/3D/react/react_body.hpp"
#include "physics/3D/react/react_shape.hpp"

namespace other {

  ReactWorld::ReactWorld() {}

  ReactWorld::~ReactWorld() {}

  void ReactWorld::Simulate(float ts) {
    float time_step = AppState::TargetTimeStep();
    accumulator += time_step;

    while (accumulator >= time_step) {
      physics_world->update(time_step);
      accumulator -= time_step;
    }

    alpha = accumulator / time_step;
  }

  Ref<PhysicsBody> ReactWorld::CreateBody(const Transform& initial_transform) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");
    glm::quat qrotation = glm::quat(initial_transform.erotation);

    rp3d::Vector3 position(initial_transform.position.x, initial_transform.position.y, initial_transform.position.z);
    rp3d::Quaternion orientation(qrotation.x, qrotation.y, qrotation.z, qrotation.w);
    rp3d::Transform transform(position, orientation);

    rp3d::RigidBody* react_body = physics_world->createRigidBody(transform);
    return NewRef<ReactBody>(react_body);
  }

  Ref<PhysicsShape> ReactWorld::CreateBoxShape(const glm::vec3& half_extents) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    rp3d::Vector3 extents(half_extents.x, half_extents.y, half_extents.z);
    rp3d::BoxShape* shape = physics_common.createBoxShape(extents);
    return NewRef<ReactShape>(shape);
  }

  Ref<PhysicsShape> ReactWorld::CreateSphereShape(float radius) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    rp3d::SphereShape* shape = physics_common.createSphereShape(radius);
    return NewRef<ReactShape>(shape);
  }

  Ref<PhysicsShape> ReactWorld::CreateCapsuleShape(float radius, float height) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    rp3d::CapsuleShape* shape = physics_common.createCapsuleShape(radius, height);
    return NewRef<ReactShape>(shape);
  }

  Ref<PhysicsShape> ReactWorld::CreateCylinderShape(float radius, float height) {
    return nullptr;
  }

  Ref<PhysicsShape> ReactWorld::CreateConeShape(float radius, float height) {
    return nullptr;
  }

  Ref<PhysicsShape> ReactWorld::CreateConvexMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    std::vector<float> vertices_data;
    for (const glm::vec3& vertex : vertices) {
      vertices_data.push_back(vertex.x);
      vertices_data.push_back(vertex.y);
      vertices_data.push_back(vertex.z);
    }

    rp3d::PolygonVertexArray::PolygonFace* faces = new rp3d::PolygonVertexArray::PolygonFace[num_faces];
    for (uint32_t i = 0; i < num_faces; i++) {
      faces[i].nbVertices = 3;
      faces[i].indexBase = i * 3;
    }

    // clang-format off
    rp3d::PolygonVertexArray polygon_vertex_array(vertices.size(), vertices_data.data(), sizeof(float) * 3, indices.data(), sizeof(uint32_t), num_faces, faces, 
                                                  rp3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE, rp3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
    // clang-format on

    std::vector<rp3d::Message> err_msgs;
    rp3d::ConvexMesh* mesh = physics_common.createConvexMesh(polygon_vertex_array, err_msgs);
    for (const rp3d::Message& msg : err_msgs) {
      switch (msg.type) {
        case rp3d::Message::Type::Error:
          OE_ERROR("ReactPhysics3D: {0}", msg.text);
          break;
        case rp3d::Message::Type::Warning:
          OE_WARN("ReactPhysics3D: {0}", msg.text);
          break;
        case rp3d::Message::Type::Information:
          OE_INFO("ReactPhysics3D: {0}", msg.text);
          break;
        default:
          break;
      }
    }

    if (mesh == nullptr) {
      return nullptr;
    }

    rp3d::ConvexMeshShape* shape = physics_common.createConvexMeshShape(mesh);
    return NewRef<ReactShape>(shape);
  }

  void ReactWorld::SetSceneContext(const Ref<Scene>& scene) {
    // PhysicsSpec spec = scene->GetPhysicsSpec();

    if (physics_world != nullptr) {
      physics_common.destroyPhysicsWorld(physics_world);
    }

    rp3d::PhysicsWorld::WorldSettings settings;
    /// TODO: set settings from spec
    settings.worldName = scene->Name();
    settings.gravity = rp3d::Vector3(0.f, -9.81f, 0.f);
    settings.gravity = reactphysics3d::Vector3(0.f, -9.81f, 0.f);
    settings.persistentContactDistanceThreshold = 0.3f;
    settings.defaultFrictionCoefficient = 0.3f;
    settings.defaultBounciness = 0.3f;
    settings.restitutionVelocityThreshold = 0.5f;
    settings.isSleepingEnabled = false;
    settings.defaultVelocitySolverNbIterations = 32;
    settings.defaultPositionSolverNbIterations = 32;
    settings.defaultTimeBeforeSleep = true;
    settings.defaultSleepLinearVelocity = 0.02f;
    settings.defaultSleepAngularVelocity = 0.0523599f;
    settings.cosAngleSimilarContactManifold = 0.95f;

    physics_world = physics_common.createPhysicsWorld(settings);

    // alpha = 0.f;

    physics_world->setIsDebugRenderingEnabled(true);

    rp3d::DebugRenderer& debug_renderer = physics_world->getDebugRenderer();
    debug_renderer.setContactNormalLength(1.5f);
    debug_renderer.setContactPointSphereRadius(1.f);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLIDER_AABB, true);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLISION_SHAPE, true);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::CONTACT_POINT, true);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::CONTACT_NORMAL, true);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLIDER_BROADPHASE_AABB, true);
  }

}  // namespace other