/**
 * \file editor/project_panel.hpp
 **/
#ifndef OTHER_ENGINE_PROJECT_PANEL_HPP
#define OTHER_ENGINE_PROJECT_PANEL_HPP

#include <vector>
#include <string>

#include <imgui/imgui.h>

#include "core/defines.hpp"
#include "project/project.hpp"
#include "editor/editor_panel.hpp"

namespace other {

  class ProjectPanel : public EditorPanel {
    public:
      ProjectPanel(Editor& editor)
        : EditorPanel(editor) {}
      virtual ~ProjectPanel() {}

      virtual void OnGuiRender(bool& is_open) override;
      virtual void OnEvent(Event* e) override;
      virtual void OnProjectChange(const Ref<Project>& project) override;
      virtual void SetSceneContext(const Ref<Scene>& scene) override;

    private:
      const std::map<UUID , std::string_view> tags {
        { FNV("editor")    , "editor" } ,
        { FNV("materials") , "materials" } ,
        { FNV("scenes")    , "scenes" } ,
        { FNV("scripts")   , "scripts" } ,
        { FNV("shaders")   , "shaders" }
      };

      Ref<Project> active_proj;
      Ref<Scene> active_scene;

      struct CurrentAssetFolder {
        Path full_path;
        std::vector<std::string> directory_names {};
      } current_asset_path;

      Opt<Path> selection = std::nullopt;
      Opt<std::string> selection_tag = std::nullopt;


      /// Render functions
      void RenderTopBar(float height) const;
      void RenderBottomBar(float height) const;
      void RenderDirectoryTree(const Path& path);

      void RenderContents(const Path& path) const;
      std::string GetContextMenuTagFromSelection() const;
      void RenderContextMenu();

      bool IsDirSelected(const Path& p) const;
      bool IsDescendantSelected(const Path& dir) const;
      void SetSelectionContext(const Path& path);
      void ValidateAndRenderSelectionCtx();
  };

} // namespace other

#endif // !OTHER_ENGINE_PROJECT_PANEL_HPP
