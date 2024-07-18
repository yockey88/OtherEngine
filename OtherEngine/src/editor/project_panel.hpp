/**
 * \file editor/project_panel.hpp
 **/
#ifndef OTHER_ENGINE_PROJECT_PANEL_HPP
#define OTHER_ENGINE_PROJECT_PANEL_HPP

#include <vector>
#include <string>

#include <imgui/imgui.h>
#include <zep/syntax.h>

#include "core/defines.hpp"

#include "project/project.hpp"

#include "scripting/script_module.hpp"
#include "rendering/ui/ui_window_map.hpp"

#include "editor/editor_panel.hpp"

namespace other {

  class ProjectPanel : public EditorPanel {
    public:
      ProjectPanel(Editor& editor)
        : EditorPanel(editor) {}
      virtual ~ProjectPanel() {}

      virtual void OnAttach() override;
      virtual void OnGuiRender(bool& is_open) override;
      virtual void OnUpdate(float dt) override;
      virtual void OnEvent(Event* e) override;
      virtual void OnProjectChange(const Ref<Project>& project) override;
      virtual void SetSceneContext(const Ref<Scene>& scene) override;

    private:
      const std::map<UUID , std::string_view> tags {
        { kEditorDirNameHash    , kEditorDirName    } ,
        { kMaterialsDirNameHash , kMaterialsDirName } ,
        { kScenesDirNameHash    , kScenesDirName    } ,
        { kScriptsDirNameHash   , kScriptsDirName   } ,
        { kShadersDirNameHash   , kShadersDirName   }
      };
      
      const std::map<UUID , std::string_view> drag_drop_tags {
        { kEditorDirNameHash    , "project-editor-folder"    } ,
        { kMaterialsDirNameHash , "project-materials-folder" } ,
        { kScenesDirNameHash    , "project-scenes-folder"    } ,
        { kScriptsDirNameHash   , "project-scripts-folder"   } ,
        { kShadersDirNameHash   , "project-shaders-folder"   }
      };

      std::vector<Ref<Directory>> project_directories;

      Ref<Project> active_proj;
      Ref<Scene> active_scene;

      struct CurrentAssetFolder {
        Path full_path;
        std::vector<std::string> directory_names {};
      } current_asset_path;

      UIWindowMap ui_windows;

      Opt<Path> selection = std::nullopt;
      Opt<std::string> selection_tag = std::nullopt;

      /// Render functions
      void RenderTopBar(float height) const;
      void RenderBottomBar(float height) const;
      void RenderDirectoryTree(const Path& path);

      void RenderContents(const Path& path);
      void RenderScriptContents();
      void RenderEditorFiles();
      void RenderPathContents(const Path& path) const;

      void RenderScriptObjectContextMenu(const ScriptObjectTag& tag);
      void ScriptObjectDragDropSource(UUID dir_tag , const ScriptObjectTag& tag);

      std::string GetContextMenuTagFromSelection() const;
      void RenderContextMenu();

      bool IsDirSelected(const Path& p) const;
      bool IsDescendantSelected(const Path& dir) const;
      void SetSelectionContext(const Path& path);
      void ValidateAndRenderSelectionCtx();
  };

} // namespace other

#endif // !OTHER_ENGINE_PROJECT_PANEL_HPP
