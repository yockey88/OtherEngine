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
      Ref<Project> active_proj;
      Ref<Scene> active_scene;

      struct CurrentAssetFolder {
        Path full_path;
        std::vector<std::string> directory_names {};
      } current_asset_path;

      /// null selection => root directory
      Opt<Path> selection = std::nullopt;

      /// Render functions
      void RenderProjectDirectoryStructure();
      void RenderCurrentProjectFolderContents();
      void RenderTopBar(float height);
      void RenderBottomBar(float height);
      void RenderDirectoryTree(const Path& path);

      void RenderContents(const Path& path);
  };

} // namespace other

#endif // !OTHER_ENGINE_PROJECT_PANEL_HPP
