/**
 * \file rendering/model_factory.cpp
 **/
#include "rendering/model_factory.hpp"

#include <glm/fwd.hpp>

#include "math/math.hpp"

#include "asset/asset_manager.hpp"

#include "rendering/model.hpp"

namespace other {

  Opt<AssetHandle> ModelFactory::framebuffer_mesh_handle = std::nullopt;

  Opt<AssetHandle> ModelFactory::line_handle = std::nullopt;

  Opt<AssetHandle> ModelFactory::triangle_handle = std::nullopt;
  Opt<AssetHandle> ModelFactory::rect_handle = std::nullopt;

  Opt<AssetHandle> ModelFactory::box_handle = std::nullopt;
  Opt<AssetHandle> ModelFactory::box_wireframe_handle;

  Opt<AssetHandle> ModelFactory::sphere_handle = std::nullopt;
  Opt<AssetHandle> ModelFactory::capsule_handle = std::nullopt;

  Opt<std::vector<Vertex>> ModelFactory::box_vertices = std::nullopt;

  AssetHandle ModelFactory::CreateFramebufferMesh() {
    if (framebuffer_mesh_handle.has_value()) {
      return *framebuffer_mesh_handle;
    }

    // clang-format off
    std::vector<float> fb_verts = {
       1.f,  1.f, 1.f, 1.f,
      -1.f,  1.f, 0.f, 1.f,
      -1.f, -1.f, 0.f, 0.f,
       1.f, -1.f, 1.f, 0.f,
    };
    // clang-format on

    std::vector<uint32_t> idxs = {
      // 0, 1, 3,
      // 1, 2, 3
    };

    Layout layout = {
      { VEC3, "voe_position" },
      { VEC2, "voe_uvs" }
    };

    AssetHandle handle = AssetManager::CreateMemOnly<ModelSource>("FramebufferMesh-Source", fb_verts, idxs, layout);
    Ref<ModelSource> source = AssetManager::GetAsset<ModelSource>(handle);
    OE_ASSERT(source != nullptr, "Failed to get framebuffer mesh source");

    framebuffer_mesh_handle = AssetManager::CreateMemOnly<StaticModel>("FramebufferMesh", source);
    return *framebuffer_mesh_handle;
  }

  AssetHandle ModelFactory::CreateLine(const glm::vec3& start, const glm::vec3& end) {
    if (line_handle.has_value()) {
      return *line_handle;
    }

    /// this is undoubtedly incorrect

    std::vector<Vertex> vertices = {};
    vertices.resize(2);

    vertices[0].position = start;
    vertices[1].position = end;

    vertices[0].normal = vec3_sub(end, start);
    vertices[1].normal = vec3_sub(start, end);

    std::vector<Index> indices;
    indices.resize(1);
    /// 3rd index is irrelevant
    indices[0] = { 0, 1, 0 };

    AssetHandle source_handle = AssetManager::CreateMemOnly<ModelSource>("LineModel-Source", vertices, indices, glm::mat4(1.f));
    Ref<ModelSource> source = AssetManager::GetAsset<ModelSource>(source_handle);
    OE_ASSERT(source != nullptr, "Failed to get framebuffer mesh source");

    line_handle = AssetManager::CreateMemOnly<StaticModel>("LineModel", source);
    return *line_handle;
  }

  AssetHandle ModelFactory::CreateTriangle() {
    if (triangle_handle.has_value()) {
      return triangle_handle.value();
    }

    std::vector<Vertex> vertices{};
    vertices.resize(3);

    vertices[0].position = { -0.5f, -0.5f, 0.f };
    vertices[1].position = { 0.5f, -0.5f, 0.f };
    vertices[2].position = { 0.0f, 0.5f, 0.f };

    vertices[0].normal = { -1.f, -1.f, 0.f };
    vertices[1].normal = { 1.f, -1.f, 0.f };
    vertices[2].normal = { 0.f, 1.f, 0.f };

    std::vector<Index> indices;
    indices.resize(1);
    indices[0] = { 0, 1, 2 };

    AssetHandle source_handle = AssetManager::CreateMemOnly<ModelSource>("TriangleModel-Source", vertices, indices, glm::mat4(1.f));
    Ref<ModelSource> source = AssetManager::GetAsset<ModelSource>(source_handle);
    OE_ASSERT(source != nullptr, "Failed to get triangle mesh source");

    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>("TriangleModel", source);
    triangle_handle = handle;

    return handle;
  }

  AssetHandle ModelFactory::CreateRect() {
    if (rect_handle.has_value()) {
      return rect_handle.value();
    }

    std::vector<Vertex> vertices{};
    vertices.resize(6);

    vertices[0].position = { 0.5f, 0.5f, 0.f };
    vertices[1].position = { 0.5f, -0.5f, 0.f };
    vertices[2].position = { -0.5f, -0.5f, 0.f };
    vertices[3].position = { -0.5f, 0.5f, 0.f };

    vertices[0].normal = { 1.f, 1.f, 0.f };
    vertices[1].normal = { 1.f, -1.f, 0.f };
    vertices[2].normal = { -1.f, -1.f, 0.f };
    vertices[3].normal = { -1.f, 1.f, 0.f };

    std::vector<Index> indices;
    indices.resize(2);

    indices[0] = { 0, 1, 3 };
    indices[1] = { 1, 2, 3 };

    AssetHandle source_handle = AssetManager::CreateMemOnly<ModelSource>("RectModel-Source", vertices, indices, glm::mat4(1.f));
    Ref<ModelSource> source = AssetManager::GetAsset<ModelSource>(source_handle);
    OE_ASSERT(source != nullptr, "Failed to get rect mesh source");

    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>("RectModel", source);

    OE_DEBUG("Created rect mesh : [{}]", handle);
    rect_handle = handle;
    return handle;
  }

  AssetHandle ModelFactory::CreateBox() {
    if (box_handle.has_value()) {
      return box_handle.value();
    }

    std::vector<Vertex> vertices = GetBoxVertices();

    std::vector<Index> indices;
    indices.resize(12);
    indices[0] = { 0, 1, 2 };
    indices[1] = { 2, 3, 0 };

    indices[2] = { 1, 5, 6 };
    indices[3] = { 6, 2, 1 };

    indices[4] = { 7, 6, 5 };
    indices[5] = { 5, 4, 7 };

    indices[6] = { 4, 0, 3 };
    indices[7] = { 3, 7, 4 };

    indices[8] = { 4, 5, 1 };
    indices[9] = { 1, 0, 4 };

    indices[10] = { 3, 2, 6 };
    indices[11] = { 6, 7, 3 };

    OE_DEBUG("Creating box model");
    AssetHandle mesh_source_handle = AssetManager::CreateMemOnly<ModelSource>("BoxModel-Source", vertices, indices, glm::mat4(1.f));

    OE_DEBUG("Created box mesh source [{}]", mesh_source_handle);
    Ref<ModelSource> mesh_source = AssetManager::GetAsset<ModelSource>(mesh_source_handle);
    OE_ASSERT(mesh_source != nullptr, "Failed to get box mesh source");

    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>("BoxModel", mesh_source);

    OE_DEBUG("Created cube mesh [{}]", handle);
    box_handle = handle;
    return handle;
  }

  AssetHandle ModelFactory::CreateBoxWireframe() {
    if (box_wireframe_handle.has_value()) {
      return box_wireframe_handle.value();
    }

    std::vector<Vertex> vertices = GetBoxVertices();

    std::vector<Index> indices;
    indices.resize(8);
    /// top face
    indices[0] = {
      //   [edges]
      0, 1,  // (top-front)
      1      // (top-left.1)
    };
    indices[1] = {
      5,    // (top-left.2)
      5, 4  // (top-back)
    };
    indices[2] = {
      4, 0,  // (top-right)

      /// bottom face
      3,  // (bottom-front.1)
    };
    indices[3] = {
      2,    // (bottom-front.2)
      2, 6  // (bottom-left)
    };
    indices[4] = {
      6, 7,  // (bottom-back)
      7      // (bottom-right.1)
    };
    indices[5] = {
      3,  // (bottom-right.2)

      /// front face
      0, 3  // (front-right)
    };
    indices[6] = {
      1, 2,  // (front-left)

      /// left face
      5,  // (back-left.1)
    };
    indices[7] = {
      6,  // (back-left.2)

      /// back face
      4, 7  // (back-right)
    };

    AssetHandle mesh_source_handle = AssetManager::CreateMemOnly<ModelSource>("BoxWireframeModel-Source", vertices, indices, glm::mat4(1.f));
    Ref<ModelSource> mesh_source = AssetManager::GetAsset<ModelSource>(mesh_source_handle);
    OE_ASSERT(mesh_source != nullptr, "Failed to get box-wireframe mesh source");

    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>("BoxWireframeModel", mesh_source);

    OE_DEBUG("Created cube wireframe mesh [{}]", handle);
    box_wireframe_handle = handle;
    return handle;
  }

  AssetHandle ModelFactory::CreateSphere(float radius) {
    if (sphere_handle.has_value()) {
      return sphere_handle.value();
    }

    std::vector<Vertex> vertices;
    std::vector<Index> indices;

    constexpr float lat_bands = 30;
    constexpr float long_bands = 30;

    for (float latitude = 0.0f; latitude <= lat_bands; latitude++) {
      const float theta = latitude * (float)constants::pi / lat_bands;
      const float sin_theta = glm::sin(theta);
      const float cos_theta = glm::cos(theta);

      for (float longitude = 0.0f; longitude <= long_bands; longitude++) {
        const float phi = longitude * 2.f * (float)constants::pi / long_bands;
        const float sin_phi = glm::sin(phi);
        const float cos_phi = glm::cos(phi);

        Vertex& vertex = vertices.emplace_back();
        vertex.normal = { cos_phi * sin_theta, cos_theta, sin_phi * sin_theta };
        vertex.position = { radius * vertex.normal.x, radius * vertex.normal.y, radius * vertex.normal.z };
      }
    }

    for (uint32_t latitude = 0; latitude < (uint32_t)lat_bands; latitude++) {
      for (uint32_t longitude = 0; longitude < (uint32_t)long_bands; longitude++) {
        const uint32_t first = (latitude * ((uint32_t)long_bands + 1)) + longitude;
        const uint32_t second = first + (uint32_t)long_bands + 1;

        indices.push_back({ first, second, first + 1 });
        indices.push_back({ second, second + 1, first + 1 });
      }
    }

    AssetHandle mesh_source_handle = AssetManager::CreateMemOnly<ModelSource>("SphereModel-Source", vertices, indices, glm::mat4(1.f));
    Ref<ModelSource> mesh_source = AssetManager::GetAsset<ModelSource>(mesh_source_handle);
    OE_ASSERT(mesh_source != nullptr, "Failed to get sphere mesh source");

    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>("SphereModel", mesh_source);
    OE_DEBUG("Created sphere mesh [{}]", handle);
    sphere_handle = handle;
    return handle;
  }

  namespace {

    static void CalculateRing(size_t segments, float radius, float y, float dy, float height, float actual_radius, std::vector<Vertex>& vertices) {
      float seg_incr = 1.0f / (float)(segments - 1);
      for (size_t s = 0; s < segments; s++) {
        float x = glm::cos(float(constants::pi * 2) * s * seg_incr) * radius;
        float z = glm::sin(float(constants::pi * 2) * s * seg_incr) * radius;

        Vertex& vertex = vertices.emplace_back();
        vertex.position = glm::vec3(actual_radius * x, actual_radius * y + height * dy, actual_radius * z);
        vertex.normal = glm::normalize(glm::vec3(x, y, z));
      }
    }

  }  // namespace

  AssetHandle ModelFactory::CreateCapsule(float radius, float height) {
    if (capsule_handle.has_value()) {
      return capsule_handle.value();
    }

    constexpr size_t subdivision_height = 8;
    constexpr size_t rings_body = subdivision_height + 1;
    constexpr size_t rings_total = subdivision_height + rings_body;
    constexpr size_t num_segments = 12;
    constexpr float radius_mod = 0.021f;  // Needed to ensure that the wireframe is always visible

    std::vector<Vertex> vertices;
    std::vector<Index> indices;

    vertices.reserve(num_segments * rings_total);
    indices.reserve((num_segments - 1) * (rings_total - 1) * 2);

    float bodyIncr = 1.0f / (float)(rings_body - 1);
    float ringIncr = 1.0f / (float)(subdivision_height - 1);

    for (int r = 0; r < subdivision_height / 2; r++)
      CalculateRing(num_segments, glm::sin(float(constants::pi) * r * ringIncr), glm::sin(float(constants::pi) * (r * ringIncr - 0.5f)), -0.5f, height, radius + radius_mod, vertices);

    for (int r = 0; r < rings_body; r++)
      CalculateRing(num_segments, 1.0f, 0.0f, r * bodyIncr - 0.5f, height, radius + radius_mod, vertices);

    for (int r = subdivision_height / 2; r < subdivision_height; r++)
      CalculateRing(num_segments, glm::sin(float(constants::pi) * r * ringIncr), glm::sin(float(constants::pi) * (r * ringIncr - 0.5f)), 0.5f, height, radius + radius_mod, vertices);

    for (int r = 0; r < rings_total - 1; r++) {
      for (int s = 0; s < num_segments - 1; s++) {
        Index& index1 = indices.emplace_back();
        index1.v1 = (uint32_t)(r * num_segments + s + 1);
        index1.v2 = (uint32_t)(r * num_segments + s + 0);
        index1.v3 = (uint32_t)((r + 1) * num_segments + s + 1);

        Index& index2 = indices.emplace_back();
        index2.v1 = (uint32_t)((r + 1) * num_segments + s + 0);
        index2.v2 = (uint32_t)((r + 1) * num_segments + s + 1);
        index2.v3 = (uint32_t)(r * num_segments + s);
      }
    }

    AssetHandle mesh_source_handle = AssetManager::CreateMemOnly<ModelSource>("CapsuleModel-Source", vertices, indices, glm::mat4(1.f));
    Ref<ModelSource> mesh_source = AssetManager::GetAsset<ModelSource>(mesh_source_handle);
    OE_ASSERT(mesh_source != nullptr, "Failed to get capsule mesh source");

    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>("CapsuleModel", mesh_source);

    OE_DEBUG("Created capsule mesh [{}]", handle);
    capsule_handle = handle;
    return handle;
  }

  std::vector<Vertex> ModelFactory::GetBoxVertices() {
    if (box_vertices.has_value()) {
      return box_vertices.value();
    }

    std::vector<Vertex> vertices;
    vertices.resize(8);

    /* (-,-,+) */ vertices[0].position = { -1.f / 2.0f, -1.f / 2.0f, 1.f / 2.0f };
    /* (-,-,+) */ vertices[0].normal = { -1.0f, -1.0f, 1.0f };
    /* (-,-,+) */ vertices[0].uv_coord = { 0.f, 1.f };

    /* (+,-,+) */ vertices[1].position = { 1.f / 2.0f, -1.f / 2.0f, 1.f / 2.0f };
    /* (+,-,+) */ vertices[1].normal = { 1.0f, -1.0f, 1.0f };
    /* (+,-,+) */ vertices[1].uv_coord = { 1.f, 1.f };

    /* (+,+,+) */ vertices[2].position = { 1.f / 2.0f, 1.f / 2.0f, 1.f / 2.0f };
    /* (+,+,+) */ vertices[2].normal = { 1.0f, 1.0f, 1.0f };
    /* (+,+,+) */ vertices[2].uv_coord = { 1.f, 0.f };

    /* (-,+,+) */ vertices[3].position = { -1.f / 2.0f, 1.f / 2.0f, 1.f / 2.0f };
    /* (-,+,+) */ vertices[3].normal = { -1.0f, 1.0f, 1.0f };
    /* (-,+,+) */ vertices[3].uv_coord = { 0.f, 0.f };

    /* (-,-,-) */ vertices[4].position = { -1.f / 2.0f, -1.f / 2.0f, -1.f / 2.0f };
    /* (-,-,-) */ vertices[4].normal = { -1.0f, -1.0f, -1.0f };
    /* (-,-,-) */ vertices[4].uv_coord = { 0.f, 1.f };

    /* (+,-,-) */ vertices[5].position = { 1.f / 2.0f, -1.f / 2.0f, -1.f / 2.0f };
    /* (+,-,-) */ vertices[5].normal = { 1.0f, -1.0f, -1.0f };
    /* (+,-,-) */ vertices[5].uv_coord = { 1.f, 1.f };

    /* (+,+,-) */ vertices[6].position = { 1.f / 2.0f, 1.f / 2.0f, -1.f / 2.0f };
    /* (+,+,-) */ vertices[6].normal = { 1.0f, 1.0f, -1.0f };
    /* (+,+,-) */ vertices[6].uv_coord = { 1.f, 0.f };

    /* (-,+,-) */ vertices[7].position = { -1.f / 2.0f, 1.f / 2.0f, -1.f / 2.0f };
    /* (-,+,-) */ vertices[7].normal = { -1.0f, 1.0f, -1.0f };
    /* (-,+,-) */ vertices[7].uv_coord = { 0.f, 0.f };

    box_vertices = vertices;
    return vertices;
  }

}  // namespace other
