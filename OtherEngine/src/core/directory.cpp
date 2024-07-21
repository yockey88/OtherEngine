/**
 * \file core/directory.cpp
 **/
#include "core/directory.hpp"

#include <filesystem>

#include "core/rand.hpp"
#include "application/app_state.hpp"

#include "editor/editor_asset_handler.hpp"

namespace other {
    
  Directory::Directory() {
    handle = Random::GenerateUUID();
  }

  Directory::Directory(const Path& path) 
      : path(path) {
    Initialize();
  }
    
  Directory::Directory(const Ref<Directory>& parent , const Path& path) 
      : parent_dir(parent) , path(path) {
    Initialize();
  }
      
  void Directory::Initialize() {
    handle = FNV(path.filename().string());
    CollectAssets();
    CollectChildren();
  }

  void Directory::CollectAssets() {
    for (auto& entry : std::filesystem::directory_iterator(path)) {
      if (entry.is_directory()) {
        continue;
      }

      if (entry.path().extension() == ".cs") {
        auto handle = AppState::Assets().As<EditorAssetHandler>()->ImportAsset(entry.path());
        assets.insert(handle);
      }
    } 
  }

  void Directory::CollectChildren() {
    for (auto& entry : std::filesystem::directory_iterator(path)) {
      if (entry.is_directory()) {
        children[FNV(entry.path().filename().string())] = NewRef<Directory>(entry.path());
      }
    }
  }

} // namespace other
