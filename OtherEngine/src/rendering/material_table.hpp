/**
 * \file rendering/material_table.hpp
 **/
#ifndef OTHER_ENGINE_MATERIAL_TABLE_HPP
#define OTHER_ENGINE_MATERIAL_TABLE_HPP

#include <cstdint>
#include <map>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "core/rand.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

#include "rendering/material.hpp"
#include "rendering/texture.hpp"

namespace other {

  class MaterialTable : public RefCounted {
   public:
    enum TableType {
      ALBEDO = 0,
      NORMAL,
      ROUGHNESS,

      NUM_TABLES,
      INVALID_TABLE = NUM_TABLES
    };

    struct Texture {
      UUID id;
      std::vector<uint8_t> data;
    };

    struct MaterialIndex {
      UUID id;
      uint32_t albedo = 0;
      uint32_t normal = 0;
      uint32_t roughness = 0;
    };

    struct Material {
      UUID id;

      uint32_t Albedo() const;
      uint32_t Normal() const;
      uint32_t Roughness() const;

      operator other::Material() const;

     private:
      friend class MaterialTable;
      MaterialIndex* indices;
    };

    MaterialTable(const glm::vec2& size = { 1920.f, 1080.f });
    virtual ~MaterialTable() override;

    UUID DefaultMaterial() const;

    static Texture CreateTexture(const glm::vec4& color, const glm::vec2& size);
    static Texture CreateTexture(const Ref<other::Texture>& texture, const glm::vec2& size);
    static Texture CreateTexture(size_t buff_size, const uint8_t* data, const glm::vec2& size);

    template <typename Fn>
      requires requires(Fn f) {
        { f(std::declval<uint32_t>(), std::declval<uint32_t>()) } -> std::same_as<glm::vec4>;
      }
    static Texture CreateTexture(Fn f, const glm::ivec2& size) {
      Texture tex = { .id = Random::GenerateUUID() };
      tex.data.resize(size.x * size.y * 4);
      for (uint32_t x = 0; x < size.x; x++) {
        for (uint32_t y = 0; y < size.y; y++) {
          uint32_t idx = (x + y * size.x) * 4;
          glm::vec4 color = f(x, y);
          tex.data[idx + 0] = static_cast<uint8_t>(color.r * 255);
          tex.data[idx + 1] = static_cast<uint8_t>(color.g * 255);
          tex.data[idx + 2] = static_cast<uint8_t>(color.b * 255);
          tex.data[idx + 3] = static_cast<uint8_t>(color.a * 255);
        }
      }
      return tex;
    }

    void Bind(uint32_t base_slot = GL_TEXTURE0) const;
    void Unbind() const;

    UUID RegisterMaterial(const glm::vec4& albedo, const glm::vec4& normal, const glm::vec4& roughness);
    UUID RegisterMaterial(Ref<other::Texture>& albedo, Ref<other::Texture>& normal, Ref<other::Texture>& roughness);
    UUID RegisterMaterial(MaterialTable::Texture& albedo, MaterialTable::Texture& normal, MaterialTable::Texture& roughness);

    bool HasMaterial(UUID id) const;
    bool HasTexture(TableType type, uint32_t idx) const;
    bool HasTexture(UUID id) const;
    bool HasTexture(TableType type, UUID id) const;

    Material GetMaterial(UUID id) const;
    const Texture& GetTexture(TableType type, uint32_t idx) const;

    template <typename Fn>
      requires requires(Fn f) {
        { f(std::declval<uint32_t>(), std::declval<uint32_t>()) } -> std::same_as<glm::vec4>;
      }
    UUID RegisterMaterial(Fn albedo, Fn normal, Fn roughness) {
      if constexpr (requires(Fn f) { { f == nullptr } -> std::same_as<bool>; }) {
        if (albedo == nullptr || normal == nullptr || roughness == nullptr) {
          OE_WARN("MaterialTable::RegisterMaterial: Function pointers are null");
          return 0;
        }
      }

      UUID id = Random::GenerateUUID();
      {
        MaterialIndex index;
        Material material;
        index.id = id;
        material.id = index.id;

        auto [matitr, res] = materials.insert({ index.id, material });
        auto [idxitr, res2] = material_indices.insert({ index.id, index });
        if (!res || !res2) {
          OE_WARN("MaterialTable::RegisterMaterial: Failed to insert material with ID : {}", index.id);
          return 0;
        }

        Material& mat = matitr->second;
        MaterialIndex& idx = idxitr->second;
        mat.indices = &idx;
      }
      auto itr = materials.find(id);
      OE_ASSERT(itr != materials.end(), "MaterialTable::RegisterMaterial: Failed to find material with ID : {}", id);
      auto [_, material] = *itr;
      {
        auto idx_itr = material_indices.find(id);
        OE_ASSERT(idx_itr != material_indices.end(), "MaterialTable::RegisterMaterial: Failed to find index with ID : {}", id);
        material.indices = &idx_itr->second;
      }
      OE_ASSERT(material.indices != nullptr, "MaterialTable::RegisterMaterial: Material indices are null");

      Texture albedo_tex = CreateTexture(albedo, size);
      material.indices->albedo = Register(TableType::ALBEDO, albedo_tex);

      Texture normal_tex = CreateTexture(normal, size);
      material.indices->normal = Register(TableType::NORMAL, normal_tex);

      Texture roughness_tex = CreateTexture(roughness, size);
      material.indices->roughness = Register(TableType::ROUGHNESS, roughness_tex);

      return id;
    }

    UUID handle;

   private:
    uint32_t mip_levels = 1;
    UUID default_material = 0;

    uint32_t largest_table_size = 0;
    glm::vec2 size = { 0.f, 0.f };

    std::map<UUID, Material> materials;
    std::map<UUID, MaterialIndex> material_indices;

    struct Table {
      std::vector<Texture> textures;
      uint32_t texture = 0;
    } tables[NUM_TABLES];

    UUID GenerateUUID() const;

    void RegisterMaterial(UUID id, Texture& albedo, Texture& normal, Texture& roughness);

    void InitializeTable(Table& table, uint32_t max_textures);
    void ShutdownTable(Table& table);

    void Recreate(uint32_t mip_levels, uint32_t num_textures, const glm::vec2& size);

    uint32_t Register(TableType type, Texture& texture);

    void SetTexture(TableType type, uint32_t idx, const uint8_t* data);
    void SetTexture(TableType type, uint32_t idx, const glm::vec4& color);
    void SetTexture(TableType type, uint32_t idx, Texture& texture);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_MATERIAL_TABLE_HPP
