/**
 * \file physics/3D/react/react_world.cpp
 **/
#include "physics/3D/react/react_world.hpp"

#include <reactphysics3d/body/RigidBody.h>
#include <reactphysics3d/mathematics/Vector3.h>

#include "core/directory.hpp"
#include "core/filesystem.hpp"
#include "core/formatters.hpp"

#include "asset/asset_manager.hpp"

#include "scene/scene.hpp"

#include "physics/3D/react/react_body.hpp"
#include "physics/3D/react/react_shape.hpp"
#include "rendering/shader.hpp"

namespace other {

  ReactWorld::ReactWorld(/* Ref<Scene> scene */) {
    Path path = Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders" / "physics_debug.oshader";
    Ref<Shader> shader = BuildShader(path);
    if (shader == nullptr) {
      OE_ERROR("Failed to build physics debug shader");
      return;
    }
    debug_data = {
      .shader = shader,
      .physics_triangles_vao = nullptr,
      .physics_lines_vao = nullptr,
    };

    // PhysicsSpec spec = scene->GetPhysicsSpec();

    rp3d::PhysicsWorld::WorldSettings settings;
    settings.worldName = "OtherEngine-PhysicsScene";
    settings.gravity = rp3d::Vector3(0.f, -9.81f, 0.f);
    // settings.persistentContactDistanceThreshold = 0.3f;
    // settings.defaultFrictionCoefficient = 0.3f;
    // settings.defaultBounciness = 0.3f;
    // settings.restitutionVelocityThreshold = 0.5f;
    // settings.isSleepingEnabled = false;
    // settings.defaultVelocitySolverNbIterations = 32;
    // settings.defaultPositionSolverNbIterations = 32;
    // settings.defaultTimeBeforeSleep = true;
    // settings.defaultSleepLinearVelocity = 0.02f;
    // settings.defaultSleepAngularVelocity = 0.0523599f;
    // settings.cosAngleSimilarContactManifold = 0.95f;

    physics_world = physics_common.createPhysicsWorld(settings);
  }

  ReactWorld::~ReactWorld() {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");
    physics_common.destroyPhysicsWorld(physics_world);
  }

  void ReactWorld::ResetSimulation() {
    // uint32_t num_bodies = physics_world->getNbRigidBodies();
    // for (uint32_t i = 0; i < num_bodies; i++) {
    //   glm::vec3 pos = initial_positions[i];
    //   glm::quat rot = initial_rotations[i];

    //   rp3d::RigidBody* body = physics_world->getRigidBody(i);
    //   body->setLinearVelocity(rp3d::Vector3(0.f, 0.f, 0.f));
    //   body->setAngularVelocity(rp3d::Vector3(0.f, 0.f, 0.f));

    //   rp3d::Transform transform(rp3d::Vector3(pos.x, pos.y, pos.z), rp3d::Quaternion(rot.w, rot.x, rot.y, rot.z));
    //   body->setTransform(transform);

    //   body->setIsActive(true);
    // }

    // initial_positions.clear();
    // initial_rotations.clear();

    prev_time = std::nullopt;
  }

  void ReactWorld::Simulate(float ts) {
    if (!prev_time.has_value()) {
      prev_time = SteadyClock::now();

      uint32_t num_bodies = physics_world->getNbRigidBodies();
      for (uint32_t i = 0; i < num_bodies; i++) {
        rp3d::RigidBody* body = physics_world->getRigidBody(i);
        initial_positions.push_back(glm::vec3(body->getTransform().getPosition().x, body->getTransform().getPosition().y, body->getTransform().getPosition().z));
        initial_rotations.push_back(glm::quat(body->getTransform().getOrientation().w, body->getTransform().getOrientation().x, body->getTransform().getOrientation().y, body->getTransform().getOrientation().z));
      }

      physics_world->update(ts);
    } else {
      interpolate_physics = true;
      current_time = SteadyClock::now();
      delta_time = current_time - *prev_time;
      prev_time = current_time;

      accumulator += delta_time.count();  /// add fixed physics time step

      while (accumulator >= ts) {
        physics_world->update(ts);
        accumulator -= ts;
      }

      alpha = accumulator / ts;
    }
  }

  Ref<PhysicsBody> ReactWorld::CreateBody(Transform& initial_transform) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    initial_transform.CalcMatrix();
    rp3d::Vector3 pos(initial_transform.position.x, initial_transform.position.y, initial_transform.position.z);
    rp3d::Quaternion orientation(initial_transform.qrotation.x, initial_transform.qrotation.y, initial_transform.qrotation.z, initial_transform.qrotation.w);
    rp3d::Transform phys_transform(pos, orientation);
    rp3d::RigidBody* body = physics_world->createRigidBody(phys_transform);

    if (debug_render_enabled) {
      body->setIsDebugEnabled(true);
    }

    return NewRef<ReactBody>(body);
  }

  Ref<PhysicsShape> ReactWorld::CreateBoxShape(const glm::vec3& half_extents) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    rp3d::Vector3 extents(half_extents.x, half_extents.y, half_extents.z);
    rp3d::BoxShape* shape = physics_common.createBoxShape(extents);
    return NewRef<ReactBoxShape>(shape);
  }

  Ref<PhysicsShape> ReactWorld::CreateSphereShape(float radius) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    rp3d::SphereShape* shape = physics_common.createSphereShape(radius);
    return NewRef<ReactSphereShape>(shape);
  }

  Ref<PhysicsShape> ReactWorld::CreateCapsuleShape(float radius, float height) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    rp3d::CapsuleShape* shape = physics_common.createCapsuleShape(radius, height);
    return NewRef<ReactCapsuleShape>(shape);
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
    return NewRef<ReactConvexMeshShape>(shape);
  }

  Ref<PhysicsShape> ReactWorld::CreateConcaveMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) {
    OE_ASSERT(false, "Concave mesh shapes are not implemented yet");
    return nullptr;
  }

  void ReactWorld::SetDebugRendering(bool debug) {
    debug_render_enabled = debug;
    physics_world->setIsDebugRenderingEnabled(debug_render_enabled);

    for (uint32_t i = 0; i < physics_world->getNbRigidBodies(); i++) {
      rp3d::RigidBody* body = physics_world->getRigidBody(i);
      body->setIsDebugEnabled(debug_render_enabled);
    }

    if (!debug_render_enabled) {
      OE_DEBUG("Disabling debug rendering");
      return;
    }
    OE_DEBUG("Enabling debug rendering");

    rp3d::DebugRenderer& debug_renderer = physics_world->getDebugRenderer();
    debug_renderer.setContactNormalLength(5.0f);
    debug_renderer.setContactPointSphereRadius(0.3f);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLIDER_AABB, true);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLISION_SHAPE, true);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::CONTACT_POINT, true);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::CONTACT_NORMAL, true);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLIDER_BROADPHASE_AABB, true);
  }

  void ReactWorld::SubmitDebugRender(Ref<SceneRenderer> renderer) {
    if (!debug_render_enabled) {
      return;
    }

    rp3d::DebugRenderer& debug_renderer = physics_world->getDebugRenderer();

    uint32_t nb_lines = debug_renderer.getNbLines();
    uint32_t nb_triangles = debug_renderer.getNbTriangles();

    const rp3d::Array<rp3d::DebugRenderer::DebugLine>& lines = debug_renderer.getLines();
    const rp3d::Array<rp3d::DebugRenderer::DebugTriangle>& triangles = debug_renderer.getTriangles();

    std::vector<float> line_vertices;
    for (uint32_t i = 0; i < nb_lines; i++) {
      const rp3d::DebugRenderer::DebugLine& line = lines[i];

      glm::vec4 color = {
        (line.color1 >> 16) & 0xFF,
        (line.color1 >> 8) & 0xFF,
        (line.color1) & 0xFF,
        1.f
      };

      glm::vec4 color2 = {
        (line.color2 >> 16) & 0xFF,
        (line.color2 >> 8) & 0xFF,
        (line.color2) & 0xFF,
        1.f
      };

      line_vertices.push_back(line.point1.x);
      line_vertices.push_back(line.point1.y);
      line_vertices.push_back(line.point1.z);

      line_vertices.push_back(color.r);
      line_vertices.push_back(color.g);
      line_vertices.push_back(color.b);

      line_vertices.push_back(line.point2.x);
      line_vertices.push_back(line.point2.y);
      line_vertices.push_back(line.point2.z);

      line_vertices.push_back(color2.r);
      line_vertices.push_back(color2.g);
      line_vertices.push_back(color2.b);
    }

    std::vector<float> triangle_vertices;
    for (uint32_t i = 0; i < nb_triangles; ++i) {
      const rp3d::DebugRenderer::DebugTriangle& triangle = triangles[i];

      glm::vec4 color = {
        (triangle.color1 >> 16) & 0xFF,
        (triangle.color1 >> 8) & 0xFF,
        (triangle.color1) & 0xFF,
        1.f
      };

      glm::vec4 color2 = {
        (triangle.color2 >> 16) & 0xFF,
        (triangle.color2 >> 8) & 0xFF,
        (triangle.color2) & 0xFF,
        1.f
      };

      glm::vec4 color3 = {
        (triangle.color3 >> 16) & 0xFF,
        (triangle.color3 >> 8) & 0xFF,
        (triangle.color3) & 0xFF,
        1.f
      };

      triangle_vertices.push_back(triangle.point1.x);
      triangle_vertices.push_back(triangle.point1.y);
      triangle_vertices.push_back(triangle.point1.z);

      triangle_vertices.push_back(color.r);
      triangle_vertices.push_back(color.g);
      triangle_vertices.push_back(color.b);

      triangle_vertices.push_back(triangle.point2.x);
      triangle_vertices.push_back(triangle.point2.y);
      triangle_vertices.push_back(triangle.point2.z);

      triangle_vertices.push_back(color2.r);
      triangle_vertices.push_back(color2.g);
      triangle_vertices.push_back(color2.b);

      triangle_vertices.push_back(triangle.point3.x);
      triangle_vertices.push_back(triangle.point3.y);
      triangle_vertices.push_back(triangle.point3.z);

      triangle_vertices.push_back(color3.r);
      triangle_vertices.push_back(color3.g);
      triangle_vertices.push_back(color3.b);
    }

    if (debug_data.physics_lines_vao == nullptr) {
      debug_data.physics_lines_vao = NewRef<VertexArray>(line_vertices, std::vector<uint32_t>{}, std::vector<uint32_t>{ 3, 3 }, BufferUsage::DYNAMIC_DRAW);
    } else {
      debug_data.physics_lines_vao->SetVertices(line_vertices);
    }

    if (debug_data.physics_triangles_vao == nullptr) {
      debug_data.physics_triangles_vao = NewRef<VertexArray>(triangle_vertices, std::vector<uint32_t>{}, std::vector<uint32_t>{ 3, 3 }, BufferUsage::DYNAMIC_DRAW);
    } else {
      debug_data.physics_triangles_vao->SetVertices(triangle_vertices);
    }

    renderer->SubmitDebugDrawCommands(
      "Geometry",
      {
        [&]() {
          debug_data.shader->Bind();
          debug_data.physics_lines_vao->Draw(DrawMode::LINES);
          debug_data.physics_triangles_vao->Draw(DrawMode::TRIANGLES);
          debug_data.shader->Unbind();
        },
      }
    );
  }

}  // namespace other