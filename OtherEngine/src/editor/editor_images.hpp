/**
 * \file editor/editor_images.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_IMAGES_HPP
#define OTHER_ENGINE_EDITOR_IMAGES_HPP

#include "rendering/texture.hpp"

namespace other {

  struct EditorImages {
    inline static Ref<Texture2D> folder_icon = nullptr;

    inline static Ref<Texture2D> default_file_icon = nullptr;

    inline static Ref<Texture2D> cs_file_icon = nullptr;
    inline static Ref<Texture2D> lua_file_icon = nullptr;

    inline static Ref<Texture2D> shader_file_icon = nullptr;

    inline static Ref<Texture2D> scene_file_icon = nullptr;

    inline static Ref<Texture2D> hpp_file_icon = nullptr;
    inline static Ref<Texture2D> cpp_file_icon = nullptr;

    static void Initialize();
    static void Shutdown();

    static Ref<Texture2D> GetIconFromExtension(const std::string_view ext);

    private:
      static Ref<Texture2D> LoadTexture(const Path& path);
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_IMAGES_HPP
