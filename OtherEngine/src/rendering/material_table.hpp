/**
 * \file rendering/material_table.hpp
 **/
#ifndef OTHER_ENGINE_MATERIAL_TABLE_HPP
#define OTHER_ENGINE_MATERIAL_TABLE_HPP

#include <cstdint>

#include <glad/glad.h>
#include <glm/fwd.hpp>

#include "core/ref_counted.hpp"

#include "rendering/rendering_defines.hpp"

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

    MaterialTable(uint32_t mip_levels, uint32_t num_textures, const glm::vec2& size);
    virtual ~MaterialTable() override;

    void Recreate(uint32_t mip_levels, uint32_t num_textures, const glm::vec2& size);

    void Bind(uint32_t base_slot = GL_TEXTURE0) const;
    void Unbind() const;

    void SetTexture(TableType type, uint32_t idx, const uint8_t* data);
    void SetTexture(TableType type, uint32_t idx, const glm::vec3& color);

    bool HasTexture(TableType type, uint32_t idx) const;

    const uint8_t* GetTexturePixels(TableType type, uint32_t idx) const;
    const std::vector<uint8_t>& GetTexture(TableType type, uint32_t idx) const;

    template <typename Fn>
      requires requires(Fn f) {
        { f(std::declval<uint32_t>(), std::declval<uint32_t>()) } -> std::same_as<glm::vec3>;
      }
    void SetTexture(TableType type, uint32_t idx, Fn f) {
      OE_ASSERT(type < NUM_TABLES, "MaterialTable::GetTexture: Table type out of bounds");
      OE_ASSERT(idx < max_textures, "MaterialTable::GetTexture: Index out of bounds");
      std::vector<uint8_t> data;
      data.resize(size.x * size.y * 4);

      for (uint32_t x = 0; x < size.x; x++) {
        for (uint32_t y = 0; y < size.y; y++) {
          glm::vec3 color = f(x, y);
          uint32_t j = (x + y * size.x) * 4;
          data[j + 0] = static_cast<uint8_t>(color.r * 255);
          data[j + 1] = static_cast<uint8_t>(color.g * 255);
          data[j + 2] = static_cast<uint8_t>(color.b * 255);
          data[j + 3] = 255;
        }
      }

      SetTexture(type, idx, data.data());
    }

   private:
    uint32_t mip_levels = 0;
    uint32_t max_textures = 0;
    glm::vec2 size = { 0.f, 0.f };

    struct Table {
      std::vector<std::vector<uint8_t>> textures;
      uint32_t texture = 0;
    } tables[NUM_TABLES];

    void InitializeTable(Table& table, uint32_t max_textures);
    void ShutdownTable(Table& table);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_MATERIAL_TABLE_HPP
