/**
 * \file rendering/material_table.cpp
 **/
#include "rendering/material_table.hpp"

#include <cstring>

#include "core/logger.hpp"

#include "rendering/rendering_defines.hpp"

namespace other {

  MaterialTable::MaterialTable(uint32_t mip_levels, uint32_t max_textures, const glm::vec2& size)
      : mip_levels(mip_levels), max_textures(max_textures), size(size) {
    OE_ASSERT(mip_levels > 0, "Mip levels must be greater than 0");
    OE_ASSERT(max_textures > 0, "Max textures must be greater than 0");

    for (uint32_t i = 0; i < NUM_TABLES; i++) {
      InitializeTable(tables[i], max_textures);
    }
  }

  MaterialTable::~MaterialTable() {
    for (uint32_t i = 0; i < NUM_TABLES; i++) {
      ShutdownTable(tables[i]);
    }
  }

  void MaterialTable::Recreate(uint32_t mip_levels, uint32_t num_textures, const glm::vec2& size) {
    OE_ASSERT(mip_levels > 0, "Mip levels must be greater than 0");
    OE_ASSERT(num_textures > 0, "Max textures must be greater than 0");

    for (uint32_t i = 0; i < NUM_TABLES; i++) {
      ShutdownTable(tables[i]);
    }

    this->mip_levels = mip_levels;
    max_textures = num_textures;
    this->size = size;

    for (uint32_t i = 0; i < NUM_TABLES; i++) {
      InitializeTable(tables[i], num_textures);
    }
  }

  void MaterialTable::Bind(uint32_t base_slot) const {
    for (uint32_t i = 0; i < NUM_TABLES; i++) {
      const Table& table = tables[i];
      uint32_t slot = base_slot + i;

      glActiveTexture(GL_TEXTURE0 + slot);
      CHECKGL();
      glBindTexture(GL_TEXTURE_2D_ARRAY, table.texture);
      CHECKGL();
    }
  }

  void MaterialTable::Unbind() const {
    for (uint32_t i = 0; i < NUM_TABLES; i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      CHECKGL();
      glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
      CHECKGL();
    }
  }

  void MaterialTable::SetTexture(TableType type, uint32_t idx, const uint8_t* data) {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::SetTexture: Table type out of bounds");
    OE_ASSERT(idx < max_textures, "MaterialTable::SetTexture: Index out of bounds");

    glBindTexture(GL_TEXTURE_2D_ARRAY, tables[type].texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, idx, size.x, size.y, 1, other::ChannelType::RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    std::vector<uint8_t>& tex_data = tables[type].textures.emplace_back();
    tex_data.resize(size.x * size.y * 4);
    std::memcpy(tex_data.data(), data, size.x * size.y * 4);
  }

  void MaterialTable::SetTexture(TableType type, uint32_t idx, const glm::vec3& color) {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::SetTexture: Table type out of bounds");
    OE_ASSERT(idx < max_textures, "MaterialTable::SetTexture: Index out of bounds");

    std::vector<uint8_t> data;
    data.resize(size.x * size.y * 4);
    for (uint32_t j = 0; j < size.x * size.y; j += 4) {
      data[j + 0] = static_cast<uint8_t>(color.r * 255);
      data[j + 1] = static_cast<uint8_t>(color.g * 255);
      data[j + 2] = static_cast<uint8_t>(color.b * 255);
      data[j + 3] = 255;
    }

    SetTexture(type, idx, data.data());
  }

  bool MaterialTable::HasTexture(TableType type, uint32_t idx) const {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::GetTexture: Table type out of bounds");
    OE_ASSERT(idx < max_textures, "MaterialTable::GetTexture: Index out of bounds");
    return !tables[type].textures[idx].empty();
  }

  const uint8_t* MaterialTable::GetTexturePixels(TableType type, uint32_t idx) const {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::GetTexture: Table type out of bounds");
    OE_ASSERT(idx < max_textures, "MaterialTable::GetTexture: Index out of bounds");
    if (tables[type].textures[idx].empty()) {
      return nullptr;
    }

    return tables[type].textures[idx].data();
  }

  const std::vector<uint8_t>& MaterialTable::GetTexture(TableType type, uint32_t idx) const {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::GetTexture: Table type out of bounds");
    OE_ASSERT(idx < max_textures, "MaterialTable::GetTexture: Index out of bounds");
    return tables[type].textures[idx];
  }

  void MaterialTable::InitializeTable(Table& table, uint32_t max_textures) {
    OE_ASSERT(max_textures > 0, "Max textures must be greater than 0");
    glGenTextures(1, &table.texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, table.texture);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mip_levels, GL_RGBA8, size.x, size.y, max_textures);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    table.textures.resize(max_textures);
  }

  void MaterialTable::ShutdownTable(Table& table) {
    glDeleteTextures(1, &table.texture);
    table.texture = 0;
    table.textures.clear();
  }

}  // namespace other
