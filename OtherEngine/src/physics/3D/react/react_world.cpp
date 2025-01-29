/**
 * \file physics/3D/react/react_world.cpp
 **/
#include "physics/3D/react/react_world.hpp"

#include <reactphysics3d/body/RigidBody.h>
#include <reactphysics3d/collision/VertexArray.h>
#include <reactphysics3d/mathematics/Vector3.h>
#include <reactphysics3d/utils/quickhull/QuickHull.h>

#include "profiling/profiling.hpp"

#include "core/filesystem.hpp"
#include "core/formatters.hpp"
#include "core/logger.hpp"

#include "asset/asset_manager.hpp"

#include "ecs/components/physics_component.hpp"
#include "scene/scene.hpp"

#include "physics/3D/physics_shape.hpp"
#include "physics/3D/react/react_body.hpp"
#include "physics/3D/react/react_shape.hpp"
#include "rendering/model.hpp"
#include "rendering/shader.hpp"

namespace other {

  class ReactLogger : public rp3d::Logger {
   public:
    virtual ~ReactLogger() override = default;

    void log(Level level, const std::string& physicsWorldName, Category category, const std::string& message, const char* filename, int lineNumber) override {
      switch (level) {
        case Level::Information:
          OE_TRACE("ReactPhysics3D: [{} : {}] {}", physicsWorldName, Logger::getCategoryName(category), message);
          break;
        case Level::Warning:
          OE_WARN("ReactPhysics3D: [{} : {}] {}", physicsWorldName, Logger::getCategoryName(category), message);
          break;
        case Level::Error:
          OE_ERROR("ReactPhysics3D: [{} : {}] {}", physicsWorldName, Logger::getCategoryName(category), message);
          break;
        default:
          OE_WARN("Invalid log level");
          OE_INFO("ReactPhysics3D: [{} : {}] {}", physicsWorldName, Logger::getCategoryName(category), message);
          break;
      }
    }
  };

  namespace {

    static ReactLogger logger;

  }  // anonymous namespace

  ReactWorld::ReactWorld(Scene* scene)
      : PhysicsWorld(scene) {
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

    physics_common.setLogger(&logger);
    physics_world = physics_common.createPhysicsWorld(settings);

    collision_listener = NewRef<ReactCollisionListener>(scene);
  }

  ReactWorld::~ReactWorld() {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");
    physics_common.destroyPhysicsWorld(physics_world);
  }

  void ReactWorld::ResetSimulation(Scene* scene) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    auto& scene_entities = scene->SceneEntities();
    for (const auto& [id, entity] : scene_entities) {
      if (entity == nullptr) {
        continue;
      }

      if (entity->HasComponent<RigidBody>()) {
        auto& body = entity->GetComponent<RigidBody>();
        auto& transform = entity->GetComponent<Transform>();
        body.physics_body->SetTransform(transform);
        body.physics_body->SetVelocity(glm::vec3(0.f));
        body.physics_body->SetAngularVelocity(glm::vec3(0.f));
      }
    }

    SetDebugRendering(debug_render_enabled);
    debug_data = {
      .shader = debug_data.shader,
      .physics_triangles_vao = nullptr,
      .physics_lines_vao = nullptr,
    };
    Simulate(0.00001f);
    prev_time = std::nullopt;
  }

  void ReactWorld::Simulate(float ts) {
    physics_world->update(ts);
    interpolate_physics = false;
    // if (!prev_time.has_value()) {
    //   prev_time = SteadyClock::now();

    //   physics_world->update(ts);
    // } else {
    //   interpolate_physics = true;
    //   current_time = SteadyClock::now();
    //   delta_time = current_time - *prev_time;
    //   prev_time = current_time;

    //   accumulator += delta_time.count();  /// add fixed physics time step

    //   while (accumulator >= ts) {
    //     physics_world->update(ts);
    //     accumulator -= ts;
    //   }

    //   alpha = accumulator / ts;
    // }
  }

  void ReactWorld::CreateBody(Entity& ent) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    Tag& tag = ent.GetComponent<Tag>();
    Transform& initial_transform = ent.GetComponent<Transform>();
    RigidBody& body = ent.GetComponent<RigidBody>();
    Collider& collider = ent.GetComponent<Collider>();

    initial_transform.CalcMatrix();
    rp3d::Vector3 pos(initial_transform.position.x, initial_transform.position.y, initial_transform.position.z);
    rp3d::Quaternion orientation(initial_transform.qrotation.x, initial_transform.qrotation.y, initial_transform.qrotation.z, initial_transform.qrotation.w);
    rp3d::Transform phys_transform(pos, orientation);
    rp3d::RigidBody* rp3d_body = physics_world->createRigidBody(phys_transform);

    if (debug_render_enabled) {
      rp3d_body->setIsDebugEnabled(true);
    }

    body.physics_body = NewRef<ReactBody>(rp3d_body, physics_world);
    body.physics_body->SetEntityID(tag.id);
    body.physics_body->SetType(body.type);

    auto error_remove = [&ent]() {
      ent.RemoveComponent<Collider>();
      ent.RemoveComponent<RigidBody>();
    };

    switch (collider.shape_idx) {
      case PhysicsShape::Shape::BOX: {
        glm::vec3 half_extents = initial_transform.scale / 2.f;
        collider.shape = CreateBoxShape(half_extents);
      } break;

      case PhysicsShape::Shape::SPHERE: {
        collider.shape = CreateSphereShape(initial_transform.scale.x / 2.f);
      } break;

      case PhysicsShape::Shape::CAPSULE: {
        collider.shape = CreateCapsuleShape(initial_transform.scale.x / 2.f, initial_transform.scale.y);
      } break;

      case PhysicsShape::Shape::CONVEX_MESH: {
        if (!ent.HasAnyComponent<StaticMesh, Mesh>()) {
          OE_ERROR("Can not create physics mesh without mesh!");
          error_remove();
          return;
        }

        if (ent.HasComponent<StaticMesh>()) {
          auto& mesh = ent.GetComponent<StaticMesh>();
          Ref<Model> model = AssetManager::GetAsset<StaticModel>(mesh.handle);
          if (model == nullptr) {
            OE_ERROR("Model asset is null");
            error_remove();
            return;
          }

          Ref<ModelSource> source = model->GetModelSource();
          if (source == nullptr) {
            OE_ERROR("Model source is null");
            error_remove();
            return;
          }

          const std::vector<Vertex>& vertices = source->Vertices();
          const std::vector<Index>& idxs = source->Indices();
          uint32_t num_faces = idxs.size();
          collider.shape = CreateConvexMeshShape(vertices, idxs, num_faces);

        } else if (ent.HasComponent<Mesh>()) {
          auto& mesh = ent.GetComponent<Mesh>();
          Ref<Model> model = AssetManager::GetAsset<Model>(mesh.handle);
          if (model == nullptr) {
            OE_ERROR("Model asset is null");
            error_remove();
            return;
          }

          Ref<ModelSource> source = model->GetModelSource();
          if (source == nullptr) {
            OE_ERROR("Model source is null");
            error_remove();
            return;
          }

          const std::vector<Vertex>& vertices = source->Vertices();
          const std::vector<Index>& idxs = source->Indices();
          uint32_t num_faces = idxs.size();
          collider.shape = CreateConvexMeshShape(vertices, idxs, num_faces);
        } else {
          OE_ASSERT(false, "Entity does not have mesh component");
        }
      } break;
      case PhysicsShape::Shape::CONCAVE_MESH: {
        OE_ERROR("Concave mesh collider not implemented yet");
        error_remove();
        return;
      } break;
      default:
        OE_ERROR("Unimplemented or invalid collider shape type : {}", collider.shape_idx);
        error_remove();
        return;
    }
    OE_ASSERT(collider.shape != nullptr, "Failed to create collider shape");

    OE_ASSERT(body.physics_body != nullptr, "Physics body is null");
    body.physics_body->AddCollider(collider.shape);
    body.physics_body->SetTransform(initial_transform);
    RegisterColliderShape(tag.id, collider.shape);

    OE_ASSERT(collider.shape != nullptr, "Collider shape is null");
    collider.shape->SetTransform(initial_transform);

    auto [itr, res] = native_bodies.insert({ body.physics_body->GetNativeBody<void*>(), tag.id });
    OE_ASSERT(res, "Failed to insert body into native bodies map");

    auto [itr2, res2] = bodies.insert({ tag.id, body.physics_body });
    OE_ASSERT(res2, "Failed to insert body into bodies map");
  }

  void ReactWorld::DestroyBody(Entity& ent) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    OE_ASSERT(ent.HasComponent<Collider>(), "Entity does not have collider component");
    OE_ASSERT(ent.HasComponent<RigidBody>(), "Entity does not have rigid body component");
    {
      RigidBody& body = ent.GetComponent<RigidBody>();
      Collider& collider = ent.GetComponent<Collider>();
      OE_ASSERT(body.physics_body != nullptr, "Physics body is null");
      OE_ASSERT(collider.shape != nullptr, "Collider shape is null");

      UnregisterColliderShape(body.physics_body, collider.shape);
      body.physics_body->RemoveCollider(collider.shape);

      rp3d::RigidBody* rp3d_body = body.physics_body->GetNativeBody<rp3d::RigidBody>();
      physics_world->destroyRigidBody(rp3d_body);
    }

    ent.RemoveComponent<Collider>();
    ent.RemoveComponent<RigidBody>();
  }

  Ref<PhysicsShape> ReactWorld::CreateBoxShape(const glm::vec3& half_extents) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");
    OE_TRACE("Creating box shape with half extents: {0}", half_extents);

    rp3d::Vector3 extents(half_extents.x, half_extents.y, half_extents.z);
    rp3d::BoxShape* shape = physics_common.createBoxShape(extents);
    OE_ASSERT(shape != nullptr, "Failed to create box shape");

    return NewRef<ReactBoxShape>(shape, this);
  }

  Ref<PhysicsShape> ReactWorld::CreateSphereShape(float radius) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");
    OE_TRACE("Creating sphere shape with radius: {0}", radius);

    rp3d::decimal r = radius;
    rp3d::SphereShape* shape = physics_common.createSphereShape(r);
    OE_ASSERT(shape != nullptr, "Failed to create sphere shape");

    return NewRef<ReactSphereShape>(shape, this);
  }

  Ref<PhysicsShape> ReactWorld::CreateCapsuleShape(float radius, float height) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");
    OE_TRACE("Creating capsule shape with radius: {0} and height: {1}", radius, height);

    rp3d::decimal r = radius;
    rp3d::decimal h = height;
    rp3d::CapsuleShape* shape = physics_common.createCapsuleShape(r, h);
    OE_ASSERT(shape != nullptr, "Failed to create capsule shape");

    return NewRef<ReactCapsuleShape>(shape, this);
  }

  Ref<PhysicsShape> ReactWorld::CreateConvexMeshShape(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, uint32_t num_faces) {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");
    PROFILE_SECTION("ReactWorld::CreateConvexMeshShape");

    std::vector<float> verts;
    {
      PROFILE_SECTION("ReactWorld::CreateConvexMeshShape--BuildVertexBuffer");
      verts.reserve(vertices.size() * 3);
      for (const Vertex& v : vertices) {
        verts.push_back(v.position.x);
        verts.push_back(v.position.y);
        verts.push_back(v.position.z);
      }
    }

    auto& mem_allocator = physics_world->getMemoryManager().getPoolAllocator();

    rp3d::VertexArray vert_array = rp3d::VertexArray(verts.data(), sizeof(float) * 3, vertices.size(), rp3d::VertexArray::DataType::VERTEX_FLOAT_TYPE);
    rp3d::PolygonVertexArray res_vert_array;
    rp3d::Array<float> res_verts(mem_allocator);
    rp3d::Array<unsigned int> res_indices(mem_allocator);
    rp3d::Array<rp3d::PolygonVertexArray::PolygonFace> res_faces(mem_allocator);
    std::vector<rp3d::Message> err_msgs;
    bool success = false;
    {
      PROFILE_SECTION("ReactWorld::CreateConvexMeshShape--ComputeConvexHull");
      success = rp3d::QuickHull::computeConvexHull(vert_array, res_vert_array, res_verts, res_indices, res_faces, mem_allocator, err_msgs);
    }
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
    if (!success) {
      OE_ERROR("Failed to compute convex hull");
      return nullptr;
    }

    err_msgs.clear();
    rp3d::ConvexMesh* mesh = nullptr;
    {
      PROFILE_SECTION("ReactWorld::CreateConvexMeshShape--CreateConvexMesh");
      mesh = physics_common.createConvexMesh(res_vert_array, err_msgs);
    }
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
      OE_ERROR("Failed to create convex mesh");
      return nullptr;
    }
    rp3d::ConvexMeshShape* shape = nullptr;
    {
      PROFILE_SECTION("ReactWorld::CreateConvexMeshShape--CreateConvexMeshShape");
      shape = physics_common.createConvexMeshShape(mesh);
    }
    if (shape == nullptr) {
      OE_ERROR("Failed to create convex mesh shape");
      return nullptr;
    }

    return NewRef<ReactConvexMeshShape>(shape, this);
  }

  Ref<PhysicsShape> ReactWorld::CreateConcaveMeshShape(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, uint32_t num_faces) {
    OE_ASSERT(false, "Concave mesh shapes are not implemented yet");
    return nullptr;
  }

  Ref<PhysicsShape> ReactWorld::CreateCompoundShape(const std::vector<Ref<PhysicsShape>>& shapes) {
    return NewRef<ReactCompoundShape>(this);
  }

  void ReactWorld::SetDebugRendering(bool debug) {
    debug_render_enabled = debug;
    physics_world->setIsDebugRenderingEnabled(debug_render_enabled);

    for (uint32_t i = 0; i < physics_world->getNbRigidBodies(); i++) {
      rp3d::RigidBody* body = physics_world->getRigidBody(i);
      body->setIsDebugEnabled(debug_render_enabled);
    }

    rp3d::DebugRenderer& debug_renderer = physics_world->getDebugRenderer();
    debug_renderer.setContactNormalLength(5.0f);
    debug_renderer.setContactPointSphereRadius(0.3f);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLIDER_AABB, debug);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLISION_SHAPE, debug);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::CONTACT_POINT, debug);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::CONTACT_NORMAL, debug);
    debug_renderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLIDER_BROADPHASE_AABB, debug);

    if (!debug) {
      debug_data = {
        .shader = debug_data.shader,
        .physics_triangles_vao = nullptr,
        .physics_lines_vao = nullptr,
      };
      Simulate(0.00001f);
    }
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
          if (debug_data.physics_lines_vao == nullptr || debug_data.physics_triangles_vao == nullptr) {
            return;
          }

          debug_data.shader->Bind();
          debug_data.physics_lines_vao->Draw(DrawMode::LINES);
          debug_data.physics_triangles_vao->Draw(DrawMode::TRIANGLES);
          debug_data.shader->Unbind();
        },
      }
    );
  }

  void ReactWorld::RegisterCallbacks() {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");
    OE_ASSERT(collision_listener != nullptr, "Collision listener is null");
    physics_world->setEventListener(&collision_listener->listener);
  }

}  // namespace other