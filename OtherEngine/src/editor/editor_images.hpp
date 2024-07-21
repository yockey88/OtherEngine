/**
 * \file editor/editor_images.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_IMAGES_HPP
#define OTHER_ENGINE_EDITOR_IMAGES_HPP

#include "rendering/texture.hpp"

namespace other {

  struct EditorImages {
    inline static Ref<Texture2D> folder_icon = nullptr;
    inline static Ref<Texture2D> cs_file_icon = nullptr;

    static void Initialize();

    static Ref<Texture2D> GetIconFromExtension(const std::string_view ext);

    private:
      static Ref<Texture2D> LoadTexture(const Path& path);
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_IMAGES_HPP
