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

    default_file_icon = LoadTexture("default_file.png");

    cs_file_icon = LoadTexture("cs_file.png");
    extension_map[FNV(".cs")] = cs_file_icon;

    lua_file_icon = LoadTexture("lua_file.png");
    extension_map[FNV(".lua")] = lua_file_icon;

    shader_file_icon = LoadTexture("shader_file.png");
    extension_map[FNV(".glsl")] = shader_file_icon;
    extension_map[FNV(".vert")] = shader_file_icon;
    extension_map[FNV(".frag")] = shader_file_icon;
    extension_map[FNV(".geom")] = shader_file_icon;
    extension_map[FNV(".oshader")] = shader_file_icon;
    
    scene_file_icon = LoadTexture("scene_file.png");
    extension_map[FNV(".yscn")] = scene_file_icon;
    extension_map[FNV(".yscene")] = scene_file_icon;

    hpp_file_icon = LoadTexture("hpp_file.png");
    extension_map[FNV(".h")] = hpp_file_icon;
    extension_map[FNV(".hpp")] = hpp_file_icon;

    cpp_file_icon = LoadTexture("cpp_file.png");
    extension_map[FNV(".cpp")] = cpp_file_icon;
  }
    
  void EditorImages::Shutdown() {
    /// this has to happen otherwise these will be destroyed during static destructor calling which is bad
    extension_map.clear();
    folder_icon = nullptr;
    default_file_icon = nullptr;
    cs_file_icon = nullptr;
    lua_file_icon = nullptr;
    shader_file_icon = nullptr;
    scene_file_icon = nullptr;
    hpp_file_icon = nullptr;
    cpp_file_icon = nullptr;
  }
    
  Ref<Texture2D> EditorImages::GetIconFromExtension(const std::string_view ext) {
    auto itr = extension_map.find(FNV(ext));
    if (itr == extension_map.end()) {
      return default_file_icon;
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
