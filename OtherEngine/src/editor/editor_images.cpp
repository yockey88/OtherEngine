/**
 * \file editor/editor_images.cpp
 **/
#include "editor/editor_images.hpp"

#include <map>

#include "core/filesystem.hpp"

namespace other {

  static std::map<UUID , Ref<Texture2D>> extension_map;    

  void EditorImages::Initialize() {
    folder_icon = LoadTexture("folder.png");

    cs_file_icon = LoadTexture("cs_file.png");
    extension_map[FNV(".cs")] = cs_file_icon;
  }
    
  Ref<Texture2D> EditorImages::GetIconFromExtension(const std::string_view ext) {
    auto itr = extension_map.find(FNV(ext));
    if (itr == extension_map.end()) {
      return nullptr;
    }

    return itr->second;
  }

  Ref<Texture2D> EditorImages::LoadTexture(const Path& path) {
    auto engine_dir = Filesystem::GetEngineCoreDir();
    auto texture_dir = engine_dir / "OtherEngine" / "assets" / "textures" / "editor";
    auto real_path =  texture_dir / path;

    OE_ASSERT(Filesystem::FileExists(real_path) , "Attempting to load non existent editor icon {}" , path);

    TextureSpecification spec {
      .channels = RGBA ,
      .type = TEX_2D ,
      .filters = {
        .min = NEAREST ,
        .mag = NEAREST ,
      } ,
      .wrap = {
        .s_val = REPEAT , 
        .t_val = REPEAT , 
      } ,
      .name = path.filename().string() ,
    };

    return NewRef<Texture2D>(real_path , spec);
  }

} // namespace other
