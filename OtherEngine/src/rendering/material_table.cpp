/**
 * \file rendering/material_table.cpp
 **/
#include "rendering/material_table.hpp"

#include <cstring>

#include "core/logger.hpp"

#include "rendering/rendering_defines.hpp"

namespace other {

  uint32_t MaterialTable::Material::Albedo() const {
    OE_ASSERT(indices != nullptr, "MaterialTable::Material::Albedo(), indices are null!");
    return indices->albedo;
  }

  uint32_t MaterialTable::Material::Normal() const {
    OE_ASSERT(indices != nullptr, "MaterialTable::Material::Normal(), indices are null!");
    return indices->normal;
  }

  uint32_t MaterialTable::Material::Roughness() const {
    OE_ASSERT(indices != nullptr, "MaterialTable::Material::Normal(), indices are null!");
    return indices->roughness;
  }

  MaterialTable::Material::operator other::Material() const {
    return {
      .albedo_tex_idx = Albedo(),
      .normal_tex_idx = Normal(),
      .roughness_tex_idx = Roughness(),
    };
  }

  MaterialTable::MaterialTable(const glm::vec2& size)
      : size(size) {
    handle = GenerateUUID();
    default_material = RegisterMaterial(glm::vec4(1.f), glm::vec4(1.f), glm::vec4(1.f));
  }

  MaterialTable::~MaterialTable() {
    for (uint32_t i = 0; i < NUM_TABLES; i++) {
      ShutdownTable(tables[i]);
    }
  }

  UUID MaterialTable::DefaultMaterial() const {
    return default_material;
  }

  MaterialTable::Texture MaterialTable::CreateTexture(const glm::vec4& color, const glm::vec2& size) {
    Texture tex = { .id = Random::GenerateUUID() };
    tex.data.resize(size.x * size.y * 4);
    for (uint32_t j = 0; j < size.x * size.y * 4; j += 4) {
      tex.data[j + 0] = static_cast<uint8_t>(color.r * 255);
      tex.data[j + 1] = static_cast<uint8_t>(color.g * 255);
      tex.data[j + 2] = static_cast<uint8_t>(color.b * 255);
      tex.data[j + 3] = static_cast<uint8_t>(color.a * 255);
    }
    return tex;
  }

  MaterialTable::Texture MaterialTable::CreateTexture(const Ref<other::Texture>& texture, const glm::vec2& size) {
    OE_ASSERT(texture->PixelData().Size() == size.x * size.y * 4, "MaterialTable::CreateTexture: Texture size mismatch");

    size_t buff_size = texture->PixelData().Size();
    return CreateTexture(buff_size, texture->PixelData().RawBytes(), size);
  }

  MaterialTable::Texture MaterialTable::CreateTexture(size_t buff_size, const uint8_t* data, const glm::vec2& size) {
    OE_ASSERT(buff_size == size.x * size.y * 4, "MaterialTable::CreateTexture: Buffer size does not match size");
    MaterialTable::Texture tex = { .id = Random::GenerateUUID() };
    tex.data.resize(size.x * size.y * 4);
    std::memcpy(tex.data.data(), data, size.x * size.y * 4);
    return tex;
  }

  void MaterialTable::Bind(uint32_t base_slot) const {
    std::span<const Table> tables_span{ tables, NUM_TABLES };
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

  UUID MaterialTable::RegisterMaterial(const glm::vec4& albedo, const glm::vec4& normal, const glm::vec4& roughness) {
    UUID id = GenerateUUID();
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

    OE_ASSERT(HasMaterial(id), "Material not found in table");
    return id;
  }

  UUID MaterialTable::RegisterMaterial(Ref<other::Texture>& albedo, Ref<other::Texture>& normal, Ref<other::Texture>& roughness) {
    UUID id = GenerateUUID();
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

    OE_ASSERT(HasMaterial(id), "Material not found in table");
    return id;
  }

  UUID MaterialTable::RegisterMaterial(MaterialTable::Texture& albedo, MaterialTable::Texture& normal, MaterialTable::Texture& roughness) {
    UUID id = GenerateUUID();
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

    material.indices->albedo = Register(TableType::ALBEDO, albedo);
    material.indices->normal = Register(TableType::NORMAL, normal);
    material.indices->roughness = Register(TableType::ROUGHNESS, roughness);

    OE_ASSERT(HasMaterial(id), "Material not found in table");
    return id;
  }

  bool MaterialTable::HasMaterial(UUID id) const {
    return materials.find(id) != materials.end();
  }

  bool MaterialTable::HasTexture(TableType type, uint32_t idx) const {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::HasTexture: Table type out of bounds");
    bool valid_idx = idx < tables[type].textures.size();
    if (!valid_idx) {
      return false;
    }

    return tables[type].textures[idx].id.Get() != 0;
  }

  bool MaterialTable::HasTexture(UUID id) const {
    for (uint32_t i = 0; i < NUM_TABLES; i++) {
      if (HasTexture(static_cast<TableType>(i), id)) {
        return true;
      }
    }
    return false;
  }

  bool MaterialTable::HasTexture(TableType type, UUID id) const {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::HasTexture: Table type out of bounds");
    for (const Texture& tex : tables[type].textures) {
      if (tex.id == id) {
        return true;
      }
    }
    return false;
  }

  MaterialTable::Material MaterialTable::GetMaterial(UUID id) const {
    auto itr = materials.find(id);
    if (itr == materials.end()) {
      OE_WARN("MaterialTable::GetMaterial: Material not found with ID : {}", id);
      return {};
    }
    return itr->second;
  }

  const MaterialTable::Texture& MaterialTable::GetTexture(TableType type, uint32_t idx) const {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::GetTexture: Table type out of bounds");
    OE_ASSERT(idx < tables[type].textures.size(), "MaterialTable::GetTexture: Index out of bounds");
    return tables[type].textures[idx];
  }

  UUID MaterialTable::GenerateUUID() const {
    UUID id = 0;
    do {
      id = Random::GenerateUUID();
    } while (HasMaterial(id) || HasTexture(id) || id.Get() == 0);
    return id;
  }

  void MaterialTable::RegisterMaterial(UUID id, Texture& albedo, Texture& normal, Texture& roughness) {
    auto itr = materials.find(id);
    if (itr != materials.end()) {
      OE_ERROR("MaterialTable::RegisterMaterial: Material already exists with ID : {}", id);
      return;
    }

    {
      MaterialIndex index;
      Material material;
      index.id = id;
      material.id = index.id;

      auto [matitr, res] = materials.insert({ index.id, material });
      auto [idxitr, res2] = material_indices.insert({ index.id, index });
      if (!res || !res2) {
        OE_WARN("MaterialTable::RegisterMaterial: Failed to insert material with ID : {}", index.id);
        return;
      }
    }

    auto mat_itr = materials.find(id);
    OE_ASSERT(mat_itr != materials.end(), "MaterialTable::RegisterMaterial: Failed to find material with ID : {}", id);
    auto [_, material] = *mat_itr;
    {
      auto idx_itr = material_indices.find(id);
      OE_ASSERT(idx_itr != material_indices.end(), "MaterialTable::RegisterMaterial: Failed to find index with ID : {}", id);
      material.indices = &idx_itr->second;
    }

    material.indices->albedo = Register(TableType::ALBEDO, albedo);
    material.indices->normal = Register(TableType::NORMAL, normal);
    material.indices->roughness = Register(TableType::ROUGHNESS, roughness);
    OE_ASSERT(HasMaterial(id), "Material not found in table");
  }

  void MaterialTable::InitializeTable(Table& table, uint32_t max_textures) {
    OE_ASSERT(max_textures > 0, "Max textures must be greater than 0");
    if (table.texture != 0) {
      ShutdownTable(table);
    }

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
    if (table.texture == 0) {
      return;
    }

    glDeleteTextures(1, &table.texture);
    table.texture = 0;
    table.textures.clear();
  }

  uint32_t MaterialTable::Register(TableType type, Texture& texture) {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::Register: Table type out of bounds");
    OE_ASSERT(texture.id.Get() != 0, "MaterialTable::Register: Texture source ID is 0");

    /// delete gpu resources and re-allocate
    /// TODO: only resize when absolutely necessary
    std::vector<Texture> textures = tables[type].textures;

    uint32_t next_idx = textures.size();
    uint32_t new_max_textures = next_idx + 1;

    ShutdownTable(tables[type]);
    InitializeTable(tables[type], new_max_textures);
    for (uint32_t i = 0; i < textures.size(); i++) {
      SetTexture(type, i, textures[i].data.data());
    }
    SetTexture(type, next_idx, texture.data.data());

    largest_table_size = std::max(largest_table_size, new_max_textures);
    return next_idx;
  }

  void MaterialTable::SetTexture(TableType type, uint32_t idx, const uint8_t* data) {
    OE_ASSERT(data != nullptr, "MaterialTable::SetTexture: Data is null");
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::SetTexture: Table type out of bounds");
    OE_ASSERT(idx < tables[type].textures.size(), "MaterialTable::SetTexture: Index out of bounds");

    glBindTexture(GL_TEXTURE_2D_ARRAY, tables[type].texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, idx, size.x, size.y, 1, other::ChannelType::RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    Texture& tex_data = tables[type].textures[idx];
    tex_data.data.resize(size.x * size.y * 4);
    OE_ASSERT(tex_data.data.size() == size.x * size.y * 4, "MaterialTable::SetTexture: Texture data size mismatch");
    /// TODO: this is fast but.... we don't have a size to check :( scary
    std::memcpy(tex_data.data.data(), data, size.x * size.y * 4);
  }

  void MaterialTable::SetTexture(TableType type, uint32_t idx, const glm::vec4& color) {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::SetTexture: Table type out of bounds");
    OE_ASSERT(idx < tables[type].textures.size(), "MaterialTable::SetTexture: Index out of bounds");

    std::vector<uint8_t> data;
    data.resize(size.x * size.y * 4);
    for (uint32_t j = 0; j < size.x * size.y * 4; j += 4) {
      data[j + 0] = static_cast<uint8_t>(color.r * 255);
      data[j + 1] = static_cast<uint8_t>(color.g * 255);
      data[j + 2] = static_cast<uint8_t>(color.b * 255);
      data[j + 3] = static_cast<uint8_t>(color.a * 255);
    }

    SetTexture(type, idx, data.data());
  }

  void MaterialTable::SetTexture(TableType type, uint32_t idx, Texture& texture) {
    OE_ASSERT(type < NUM_TABLES, "MaterialTable::SetTexture: Table type out of bounds");
    OE_ASSERT(idx < tables[type].textures.size(), "MaterialTable::SetTexture: Index out of bounds");

    glBindTexture(GL_TEXTURE_2D_ARRAY, tables[type].texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, idx, size.x, size.y, 1, other::ChannelType::RGBA, GL_UNSIGNED_BYTE, texture.data.data());
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    tables[type].textures[idx] = texture;
  }

}  // namespace other
