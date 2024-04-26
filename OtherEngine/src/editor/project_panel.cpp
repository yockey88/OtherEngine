/**
 * \file editor/project_panel.cpp
 **/
#include "editor/project_panel.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "rendering/ui/ui_colors.hpp"
#include "rendering/ui/ui_helpers.hpp"

namespace other {

  ProjectPanel::ProjectPanel() {
  }

  void ProjectPanel::OnGuiRender(bool& is_open) {
    ImGui::SetNextWindowSize(ImVec2(200.f , ImGui::GetContentRegionAvail().y));
    if (!ImGui::Begin("Project" , &is_open , ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar) || 
        active_proj == nullptr) {
      ImGui::End();
      return;
    }

    RenderProjectDirectoryTable();

    ImGui::End();
  }

  void ProjectPanel::OnEvent(Event* e) {
  }

  void ProjectPanel::OnProjectChange(const Ref<Project>& project) {
    // honesetly if project changes we should be reloading the entire goddamn thing
    //  I could care less
    // restart everything motherfucker
    active_proj = project;
  }

  void ProjectPanel::SetSceneContext(const Ref<Scene>& scene) {
  }

  void ProjectPanel::RenderProjectDirectoryTable() {
    ImGuiTableFlags table_flags = ImGuiTableFlags_Resizable 
      | ImGuiTableFlags_SizingFixedFit
      | ImGuiTableFlags_BordersInnerV;
    if (ImGui::BeginTable("##project_table" , 2 , table_flags , ImVec2(0.f , 0.f))) {
      ImGui::TableSetupColumn("Outliner");
      ImGui::TableSetupColumn("Directory Structure" , ImGuiTableColumnFlags_WidthStretch);

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      RenderProjectDirectoryStructure();

      ImGui::TableSetColumnIndex(1);

      RenderCurrentProjectFolderContents();
 
      ImGui::EndTable();
    }
  }
      
  void ProjectPanel::RenderProjectDirectoryStructure() {
    ImGui::BeginChild("##project_directory");
    {
      ScopedStyle spacing(ImGuiStyleVar_ItemSpacing , ImVec2(0.f , 0.f));
      ScopedColorStack item_bg(ImGuiCol_Header , IM_COL32_DISABLE , 
                               ImGuiCol_HeaderActive , IM_COL32_DISABLE);

      auto folders = active_proj->GetProjectDirPaths();
      std::sort(folders.begin() , folders.end() , [](const auto& a , const auto& b) -> bool {
        return a.stem().string() < b.stem().string();
      });

      for (auto& dir : folders) {
        if (!Filesystem::IsDirectory(dir)) {
          continue;
        }
        RenderDirectoryTree(dir);
      }

      /// draw side shader here
      /// get window rect
      /// push clip rect
      /// draw shader here
      /// pop clip rect
    }
    ImGui::EndChild();
  }
      
  void ProjectPanel::RenderCurrentProjectFolderContents() {
    /// Directory Content
    constexpr float top_bar_h = 26.f;
    constexpr float bottom_bar_h = 32.f;
    constexpr float tot_h_offset = top_bar_h + bottom_bar_h;
    ImGui::BeginChild("##project_directory_content" , 
                       ImVec2(ImGui::GetContentRegionAvail().x , ImGui::GetContentRegionAvail().y - tot_h_offset)); 
    {
      ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
      RenderTopBar(top_bar_h);
      ImGui::PopStyleVar();

      ImGui::Separator();

      ImGui::BeginChild("Scrolling");
      {
        // pop up on right right click here
        const float padding_for_outline = 2.0f;
        const float scroll_barr_offset = 20.0f + ImGui::GetStyle().ScrollbarSize;
        {
          const float row_spacing = 12.0f;
          ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(padding_for_outline, row_spacing));

          ScopedStyle border(ImGuiStyleVar_FrameBorderSize, 0.0f);
          ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
          // render items
          if (selection.has_value()) {
            for (auto& f : Filesystem::GetSubPaths(selection.value())) {
              ImGui::PushID(f.string().c_str());
              if (Filesystem::IsDirectory(f)) {
                // render dir
              } else {
                ImGui::Text("%s" , f.string().c_str());
              }
              ImGui::PopID();
            }
          }
        }
      }
      ImGui::EndChild();
    }
    ImGui::EndChild();

    if (ImGui::BeginDragDropTarget()) {
      ImGui::EndDragDropTarget();
    }

    RenderBottomBar(bottom_bar_h);
  }
      
  void ProjectPanel::RenderTopBar(float height) {
    ImGui::BeginChild("##project_directory_contents_top_bar" , ImVec2(0.f , height));
    ImGui::BeginHorizontal("##project_directory_contents_top_bar");
    {
      // std::string str = "";
      // if (selection.has_value()) {
      //   /// construct navigation string
      //   for (auto i = 0; i < current_asset_path.directory_names.size(); ++i) {
      //     str += current_asset_path.directory_names[i];
      //     if (i < current_asset_path.directory_names.size() - 1) {
      //       str += " > ";
      //     }
      //   }

      //   ImGui::Text("%s" , selection.value().string().c_str());
      //   ImGui::Text("%s" , str.c_str());
      // } else {
      //   ImGui::Text("No Directory Selected");
      // }

      // navigation
      // search
      // [ empty space ]
      // settings
    }
    ImGui::EndHorizontal();
    ImGui::EndChild();
  }

  void ProjectPanel::RenderBottomBar(float height) {
    ImGui::BeginChild("##project_directory_contents_bottom_bar" , ImVec2(0.f , height));
    ImGui::BeginHorizontal("##project_directory_contents_bottom_bar");
    {
    }
    ImGui::EndHorizontal();
    ImGui::EndChild();
  }
      
  void ProjectPanel::RenderDirectoryTree(const Path& path) {
    ImGui::PushID(path.string().c_str());
    auto non_absolute = path.relative_path();

    bool prev_state = ImGui::TreeNodeBehaviorIsOpen(ImGui::GetID(non_absolute.string().c_str()));
    bool curr_state = ImGui::TreeNode(non_absolute.string().c_str());

    if (curr_state) {
      /// I think that this should do the path processing on new selection but no every frame???
      ///   I genuinely have no idea tho
      // if (!prev_state) {
      //   current_asset_path.full_path = path;  
      //   current_asset_path.directory_names.clear();

      //   // std::string path_str = non_absolute.string();
      //   // std::string str = "";

      //   // /// parse path into components to display on top bar for navigation purposes 
      //   // for (size_t i = 0; i <= path_str.size(); ++i) {
      //   //   if (i >= path_str.size() || path_str[i] == '\\' || !str.empty()) {
      //   //     current_asset_path.directory_names.push_back(str);
      //   //     str.clear();
      //   //   } else {
      //   //     str += path_str[i];
      //   //   }
      //   // }
      // }

      for (auto& e : Filesystem::GetSubPaths(path)) {
        if (!Filesystem::IsDirectory(e)) {
          continue;
        }

        RenderDirectoryTree(e);
      }
      selection = path;

      ImGui::TreePop();  
    } else if (prev_state) {
      selection = std::nullopt;
    }

    ImGui::PopID();
  }

} // namespace other 
