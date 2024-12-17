/**
 * \file asset/asset_manager.cpp
 **/
#include "asset/asset_manager.hpp"

#include "asset/asset_database.hpp"
#include "asset/asset_defines.hpp"

namespace other {

  Ref<MaterialTable> AssetManager::material_table = nullptr;

  Ref<MaterialTable> AssetManager::GetMaterialTable() {
    if (material_table == nullptr) {
      material_table = Ref<MaterialTable>::Create();
    }
    return material_table;
  }

  AssetType AssetManager::AssetTypeFromExtension(const std::string_view extension) {
    auto it = asset_extensions.find(FNV(extension));
    if (it == asset_extensions.end()) {
      return AssetType::GENERIC_FILE;
    }
    return it->second;
  }

  bool AssetManager::AssetValid(AssetHandle handle) {
    return AssetDatabase::Contains(handle);
  }

}  // namespace other