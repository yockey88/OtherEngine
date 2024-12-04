/**
 * \file editor/panels/shader_creator.hpp
 **/
#ifndef OTHER_ENGINE_SHADER_CREATOR_HPP
#define OTHER_ENGINE_SHADER_CREATOR_HPP

#include <map>

#include "core/file_handle.hpp"
#include "core/uuid.hpp"

#include "editor/editor_panel.hpp"

namespace other {

  class ShaderCreator : public EditorPanel {
   public:
    ShaderCreator() {}
    virtual ~ShaderCreator() override {}

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual bool OnGuiRender(bool& is_open) override;

   private:
    std::map<UUID, Ref<FileHandle>> vertex_files;
    std::map<UUID, Ref<FileHandle>> fragment_files;
    std::map<UUID, Ref<FileHandle>> geometry_files;
    std::map<UUID, Ref<FileHandle>> other_shader_files;

    Opt<Ref<FileHandle>> vertex_selection = std::nullopt;
    Opt<Ref<FileHandle>> fragment_selection = std::nullopt;
    Opt<Ref<FileHandle>> geometry_selection = std::nullopt;
    Opt<Ref<FileHandle>> other_shader_selection = std::nullopt;

    Opt<Ref<Shader>> shader = std::nullopt;

    Ref<FileHandle> RenderColumn(const std::string_view title, const std::string_view prefix, std::map<UUID, Ref<FileHandle>>& files);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SHADER_CREATOR_HPP
