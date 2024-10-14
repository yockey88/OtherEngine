/**
 * \file rendering/model_factory.cpp
 **/
#include "rendering/model_factory.hpp"

#include "asset/asset_manager.hpp"

#include "rendering/model.hpp"

namespace other {

  Opt<AssetHandle> ModelFactory::triangle_handle = std::nullopt;
  Opt<AssetHandle> ModelFactory::rect_handle = std::nullopt;

  Opt<AssetHandle> ModelFactory::box_handle = std::nullopt;
  Opt<AssetHandle> ModelFactory::box_wireframe_handle;

  Opt<AssetHandle> ModelFactory::sphere_handle = std::nullopt;
  Opt<AssetHandle> ModelFactory::capsule_handle = std::nullopt;

  Opt<std::vector<Vertex>> ModelFactory::box_vertices = std::nullopt;

  AssetHandle ModelFactory::CreateTriangle(const glm::vec2& half_extents) {
    if (triangle_handle.has_value()) {
      return triangle_handle.value();
    }

    std::vector<Vertex> vertices{};
    vertices.resize(3);

    vertices[0].position = {-half_extents.x, -half_extents.y, 0.f};
    vertices[1].position = {+half_extents.x, -half_extents.y, 0.f};
    vertices[2].position = {0.f, half_extents.y, 0.f};

    vertices[0].normal = {-1.f, -1.f, 0.f};
    vertices[1].normal = {1.f, -1.f, 0.f};
    vertices[2].normal = {0.f, 1.f, 0.f};

    std::vector<Index> indices;
    indices.resize(1);
    indices[0] = {0, 1, 2};

    AssetHandle source_handle = AssetManager::CreateMemOnly<ModelSource>(vertices, indices, glm::mat4(1.f));
    Ref<ModelSource> source = AssetManager::GetAsset<ModelSource>(source_handle);

    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>(source);
    triangle_handle = handle;

    return handle;
  }

  AssetHandle ModelFactory::CreateRect(const glm::vec2& half_extents) {
    if (rect_handle.has_value()) {
      return rect_handle.value();
    }

    std::vector<Vertex> vertices{};
    vertices.resize(6);

    vertices[0].position = {+half_extents.x, +half_extents.y, 0.f};
    vertices[1].position = {+half_extents.x, -half_extents.y, 0.f};
    vertices[2].position = {-half_extents.x, -half_extents.y, 0.f};
    vertices[3].position = {-half_extents.x, +half_extents.y, 0.f};

    vertices[0].normal = {1.f, 1.f, 0.f};
    vertices[1].normal = {1.f, -1.f, 0.f};
    vertices[2].normal = {-1.f, -1.f, 0.f};
    vertices[3].normal = {-1.f, 1.f, 0.f};

    std::vector<Index> indices;
    indices.resize(2);

    indices[0] = {0, 1, 3};
    indices[1] = {1, 2, 3};

    AssetHandle source_handle = AssetManager::CreateMemOnly<ModelSource>(vertices, indices, glm::mat4(1.f));
    Ref<ModelSource> source = AssetManager::GetAsset<ModelSource>(source_handle);
    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>(source);

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
    /// front face
    indices[0] = {0, 1, 2};  /// upper = <+,+,+> => <-,+,+> => <-,-,+>
    indices[1] = {2, 3, 0};  /// lower = <-,-,+> => <+,-,+> => <+,+,+>

    /// left face
    indices[2] = {1, 5, 6};  /// upper = <-,+,+> => <-,+,-> => <-,-,->
    indices[3] = {6, 2, 1};  /// lower = <-,-,-> => <-,-,+> => <-,+,+>

    /// back face
    indices[4] = {5, 4, 7};  ///  upper = <-,+,-> => <+,+,-> => <+,-,->
    indices[5] = {7, 6, 5};  ///  lower = <+,-,-> => <-,-,-> => <-,+,->

    /// right face
    indices[6] = {4, 0, 3};  /// upper = <+,+,-> => <+,+,+> => <+,-,+>
    indices[7] = {3, 7, 4};  /// lower = <+,-,+> => <+,-,-> => <+,+,->

    /// top face
    indices[8] = {4, 5, 1};  /// back = <+,+,-> => <-,+,-> => <-,+,+>
    indices[9] = {1, 0, 4};  /// front = <-,+,+> => <+,+,+> => <+,+,->

    /// bottom face
    indices[10] = {3, 2, 6};  /// front = <+,-,+> => <-,-,+> => <-,-,->
    indices[11] = {6, 7, 3};  /// back = <-,-,-> => <+,-,-> => <+,-,+>

    AssetHandle mesh_source_handle = AssetManager::CreateMemOnly<ModelSource>(vertices, indices, glm::mat4(1.f));
    Ref<ModelSource> mesh_source = AssetManager::GetAsset<ModelSource>(mesh_source_handle);
    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>(mesh_source);

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

    AssetHandle mesh_source_handle = AssetManager::CreateMemOnly<ModelSource>(vertices, indices, glm::mat4(1.f));
    Ref<ModelSource> mesh_source = AssetManager::GetAsset<ModelSource>(mesh_source_handle);
    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>(mesh_source);

    OE_DEBUG("Created cube wireframe mesh [{}]", handle);
    box_wireframe_handle = handle;
    return handle;
  }

  AssetHandle ModelFactory::CreateSphere(float radius) {
    return 0;
  }

  AssetHandle ModelFactory::CreateCapsule(float radius, float height) {
    return 0;
  }

  std::vector<Vertex> ModelFactory::GetBoxVertices() {
    if (box_vertices.has_value()) {
      return box_vertices.value();
    }

    std::vector<Vertex> vertices;
    vertices.resize(8);

    /* (+,+,+) */ vertices[0].position = {1.f / 2.0f, 1.f / 2.0f, 1.f / 2.0f};
    /* (+,+,+) */ vertices[0].normal = {1.0f, 1.0f, 1.0f};

    /* (-,+,+) */ vertices[1].position = {-1.f / 2.0f, 1.f / 2.0f, 1.f / 2.0f};
    /* (-,+,+) */ vertices[1].normal = {-1.0f, 1.0f, 1.0f};

    /* (-,-,+) */ vertices[2].position = {-1.f / 2.0f, -1.f / 2.0f, 1.f / 2.0f};
    /* (-,-,+) */ vertices[2].normal = {-1.0f, -1.0f, 1.0f};

    /* (+,-,+) */ vertices[3].position = {1.f / 2.0f, -1.f / 2.0f, 1.f / 2.0f};
    /* (+,-,+) */ vertices[3].normal = {1.0f, -1.0f, 1.0f};

    /* (+,+,-) */ vertices[4].position = {1.f / 2.0f, 1.f / 2.0f, -1.f / 2.0f};
    /* (+,+,-) */ vertices[4].normal = {1.0f, 1.0f, -1.0f};

    /* (-,+,-) */ vertices[5].position = {-1.f / 2.0f, 1.f / 2.0f, -1.f / 2.0f};
    /* (-,+,-) */ vertices[5].normal = {-1.0f, 1.0f, -1.0f};

    /* (-,-,-) */ vertices[6].position = {-1.f / 2.0f, -1.f / 2.0f, -1.f / 2.0f};
    /* (-,-,-) */ vertices[6].normal = {-1.0f, -1.0f, -1.0f};

    /* (+,-,-) */ vertices[7].position = {1.f / 2.0f, -1.f / 2.0f, -1.f / 2.0f};
    /* (+,-,-) */ vertices[7].normal = {1.0f, -1.0f, -1.0f};

    box_vertices = vertices;
    return vertices;
  }

}  // namespace other
