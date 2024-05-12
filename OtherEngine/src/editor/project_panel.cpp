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

  void ProjectPanel::OnGuiRender(bool& is_open) {
    ImGui::SetNextWindowSize(ImVec2(200.f , ImGui::GetContentRegionAvail().y));
    if (!ImGui::Begin("Project" , &is_open , ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar) || 
        active_proj == nullptr) {
      ImGui::End();
      return;
    }

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

    ImGui::End();

    // we clear the selection here because things get weird if selection persists between frames
    //   since render directory folders recursively
    selection = std::nullopt;
  }

  void ProjectPanel::OnEvent(Event* e) {
  }

  void ProjectPanel::OnProjectChange(const Ref<Project>& project) {
    active_proj = project;
  }

  void ProjectPanel::SetSceneContext(const Ref<Scene>& scene) {
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
            RenderContents(selection.value());
          } else {
            // render root 
          }
        }
      }
      ImGui::EndChild();
    }
    ImGui::EndChild();

    RenderBottomBar(bottom_bar_h);
  }
      
  void ProjectPanel::RenderTopBar(float height) {
    ImGui::BeginChild("##project_directory_contents_top_bar" , ImVec2(0.f , height));
    ImGui::BeginHorizontal("##project_directory_contents_top_bar");
    {
      if (selection.has_value()) {
        /// construct navigation string
        ImGui::Text("%s" , selection.value().string().c_str());
      } else {
        ImGui::Text("No Directory Selected");
      }

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
    auto files = Filesystem::GetSubDirs(path);

    ImGuiID node_id = ImGui::GetID(non_absolute.string().c_str());

    if (ImGui::TreeNode(non_absolute.string().c_str())) {
      selection = path;

      for (auto& e : Filesystem::GetSubPaths(path)) {
        if (!Filesystem::IsDirectory(e)) {
          continue;
        }

        RenderDirectoryTree(e);
      }

      ImGui::TreePop();  
    } else if (ImGui::TreeNodeBehaviorIsOpen(node_id)) {
      /// not open on this frame but was open last frame 
    }

    ImGui::PopID();
  }

  void ProjectPanel::RenderContents(const Path& path) {
    for (auto& f : Filesystem::GetSubPaths(path)) {
      ImGui::PushID(f.string().c_str());
      
      if (Filesystem::IsDirectory(f)) {
        // render folder icon and if clicked set selection context
      } else {
        ImGui::Selectable(f.string().c_str());

        /// need to keep track of context here because drag-drop tag is dependent on active selected folder
        ///   different assets have different valid drop targets
        ImGuiDragDropFlags dd_flags = ImGuiDragDropFlags_SourceNoDisableHover;
        if (ImGui::BeginDragDropSource(dd_flags)) {
          
          ImGui::Text("%s" , f.string().c_str());
          ImGui::SetDragDropPayload("project_content_folder" , f.string().c_str() , f.string().size() + 1);

          ImGui::EndDragDropSource();
        }
      }

      ImGui::PopID();
    }
  }

} // namespace other 
