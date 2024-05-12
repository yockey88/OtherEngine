/**
 * \file editor/scene_panel.cpp
 **/
#include "editor/scene_panel.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "ecs/entity.hpp"
#include "editor/editor.hpp"
#include "editor/selection_manager.hpp"

namespace other {

  static std::vector<std::string> things_in_scene{};

  void ScenePanel::OnGuiRender(bool& is_open) {
    if (ImGui::Begin("Hierarchy" , &is_open)) {
      if (active_scene == nullptr) {

        AcceptAssetPayload(); 

        ImGui::End();
        return;
      } else {
        RenderSceneHierarchy(active_scene->SceneEntities());
      }
    }
    ImGui::End();
  }

  void ScenePanel::OnEvent(Event* e) {
  }

  void ScenePanel::OnProjectChange(const Ref<Project>& project) {
  }

  void ScenePanel::SetSceneContext(const Ref<Scene>& scene) {
    OE_ASSERT(scene != nullptr , "Attempting to set null scene context in ScenePanel");
    active_scene = scene;
  }

  void ScenePanel::AcceptAssetPayload() {
    if (ImGui::BeginDragDropTarget()) {

      const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("project_content_folder");
      if (payload == nullptr) {
	ImGui::EndDragDropTarget();
	return;
      }

      Path path = std::string{ 
        static_cast<const char*>(payload->Data) , 
      };

      OE_DEBUG("Accepted asset with extension : {}" , path.extension());
      if (!Filesystem::FileExists(path)) {
        OE_WARN("Can not find file : {}" , path);
      } else if (path.extension() != ".yscn") {
        OE_WARN("Scene hierarchy currently only accepts '.yscn' files, loaded file has {} extension" , 
                path.extension());
      } else {
        /**
         * There are a few cases here to think about
         *  1 > dragged scene file into scene view? 
         *      > scene already loaded?
         *        - display pop up asking for confirmation
         *        - save and load new file
         *      > no scene loaded? 
         *        - load scene
         *      
         *  2 > dragged asset into hierarchy view? 
         *      > scene loaded? 
         *        (this would mean they dragged into hierarchy and not into an entity)
         *        - create new empty entity
         *        - add correct component to entity
         *      > no scene loaded?
         *        - open corresponding asset editor??
         **/
        GetEditor().LoadScene(path);
      }

      ImGui::EndDragDropTarget();
    }
  }
      
  void ScenePanel::RenderSceneHierarchy(const std::map<UUID , Entity*>& entities) {
    for (auto& [id , entity] : entities) {
      ImGui::PushID(id.Get());
      RenderEntity(id , entity);
      ImGui::PopID();
    }
  }
      
  void ScenePanel::RenderEntity(const UUID& id , Entity* entity) {
    ImGuiID ig_id = ImGui::GetID(entity->Name().c_str());

    bool prev_state = ImGui::TreeNodeBehaviorIsOpen(ig_id);

    /// TODO figure this shit out, this shit is gonna be so goddamn complicated if we do a tree structure
    if (ImGui::TreeNode(entity->Name().c_str())) {
      if (SelectionManager::ActiveSelection() != entity) {
        SelectionManager::Select(entity); 
      }
      
      /// render entity children

      ImGui::TreePop();
    } else if (prev_state && SelectionManager::ActiveSelection() == entity) {
      SelectionManager::ClearSelection();
    }
  }

} // namespace other 
