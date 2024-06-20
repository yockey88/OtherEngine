/**
 * \file editor/project_panel.cpp
 **/
#include "editor/project_panel.hpp"

#include <algorithm>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_colors.hpp"

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
      /// project directory
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
      }
      ImGui::EndChild();

      ImGui::TableSetColumnIndex(1);
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
          // const float scroll_barr_offset = 20.0f + ImGui::GetStyle().ScrollbarSize;
          {
            const float row_spacing = 12.0f;
            ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(padding_for_outline, row_spacing));

            ScopedStyle border(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
            // render items
            ValidateAndRenderSelectionCtx();
          }
        }
        ImGui::EndChild();
      }
      ImGui::EndChild();

      RenderBottomBar(bottom_bar_h);
 
      ImGui::EndTable();
    }

    ImGui::End();
  }

  void ProjectPanel::OnEvent(Event* e) {
  }

  void ProjectPanel::OnProjectChange(const Ref<Project>& project) {
    active_proj = project;
  }

  void ProjectPanel::SetSceneContext(const Ref<Scene>& scene) {
  }
      
  void ProjectPanel::RenderTopBar(float height) const {
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

  void ProjectPanel::RenderBottomBar(float height) const {
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
    bool prev_state = ImGui::TreeNodeBehaviorIsOpen(node_id);

    auto* window = ImGui::GetCurrentWindow();
    window->DC.CurrLineSize.y = 20.f;
    window->DC.CurrLineTextBaseOffset = 3.0f;

    const ImRect item_rect = {
      window->WorkRect.Min.x , 
      window->DC.CursorPos.y , 
      window->WorkRect.Min.y , 
      window->DC.CursorPos.y + 
        window->DC.CurrLineSize.y
    };

    const bool item_clicked = [&item_rect , &node_id]() -> bool {
      if (ImGui::ItemHoverable(item_rect , node_id , 0)) {
        return ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left);
      }
      return false;
    } ();

    const bool window_focused = ImGui::IsWindowFocused();

    auto fill_w_color = [&](const ImColor& color) {
      const ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(color);
      ImGui::GetWindowDrawList()->AddRectFilled(item_rect.Min , item_rect.Max , bg_color);
    };

    auto check_if_descendant_selected = [&](const Path& dir , auto is_any_descendant_selected) -> bool {
      if (!selection.has_value()) {
        return false;
      }

      if (dir == selection.value()) {
        return true;
      }
      
      auto items = Filesystem::GetSubPaths(dir);
      if (!items.empty()) {
        for (const auto& d : items) {
          if (!Filesystem::IsDirectory(d)) {
            continue;
          }

          if (is_any_descendant_selected(d , is_any_descendant_selected)) {
            return true;
          }
        }
      }

      return false;
    };

    const bool any_descendant_selected = check_if_descendant_selected(path , check_if_descendant_selected);
    const bool is_active_directory = IsDirSelected(path);

    ImGuiTreeNodeFlags flags = (is_active_directory ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanFullWidth;
    
    if (item_clicked && !any_descendant_selected) {
      SetSelectionContext(path);
    }

    if (is_active_directory || item_clicked) {
      if (window_focused) {
        fill_w_color(ui::theme::selection);
      } else {
        const ImColor col = ui::theme::ColorWithMultiplier(ui::theme::selection , 0.8f);
        fill_w_color(ui::theme::ColorWithMultipliedSaturation(col , 0.7f));
      }

      ImGui::PushStyleColor(ImGuiCol_Text , ui::theme::background_dark);
    } else if (any_descendant_selected) {
      fill_w_color(ui::theme::selection_muted);
    }

    bool open = ImGui::TreeNodeEx(non_absolute.string().c_str() , flags);

    if (is_active_directory || item_clicked) {
      ImGui::PopStyleColor();
    }

    ui::ShiftCursorY(3.0f);

    // create context menu

    // directory items 

    if (open) {
      for (auto& e : Filesystem::GetSubPaths(path)) {
        if (!Filesystem::IsDirectory(e)) {
          continue;
        }

        RenderDirectoryTree(e);
      }

      ImGui::TreePop();
    } else if (prev_state) {
      /// not open on this frame but was open last frame 
    }

    ImGui::PopID();
  }

  void ProjectPanel::RenderContents(const Path& path) const {
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
      
  std::string ProjectPanel::GetContextMenuTagFromSelection() const {
    if (!selection.has_value()) {
      return "";
    }

    auto itr = std::find_if(tags.begin() , tags.end() , [&](const auto& tag_pair) -> bool {
      return tag_pair.first.Get() == FNV(selection.value().string());
    });

    if (itr == tags.end()) {
      return "";
    } 

    return std::string{ itr->second };
  }

  template <typename Fn>
  void DrawContextMenu(const std::string_view tag , Fn callback) {
    if (ImGui::BeginPopup(("##project-context-" + std::string{ tag }).c_str())) {
      callback();
      ImGui::EndPopup();
    }
  }
      
  void ProjectPanel::RenderContextMenu() const {
    std::string tag = "";
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      tag = GetContextMenuTagFromSelection();
      if (!tag.empty()) {
        ImGui::OpenPopup(("##project-context-" + tag).c_str());
      }
    }

    if (tag.empty()) {
      return;
    }
    
    DrawContextMenu(tags.at(FNV("editor")) , []{});

    DrawContextMenu(tags.at(FNV("material")) , []{});

    DrawContextMenu(tags.at(FNV("scenes")) , []{});

    DrawContextMenu(tags.at(FNV("scripts")) , []{});

    DrawContextMenu(tags.at(FNV("shaders")) , []{});
  }

  bool ProjectPanel::IsDirSelected(const Path& dir) const {
    return selection.has_value() && dir == selection.value(); 
  }

  bool ProjectPanel::IsDescendantSelected(const Path& dir) const {
    if (!selection.has_value()) {
      return false;
    }

    if (dir == selection.value()) {
      return true;
    }
    
    auto items = Filesystem::GetSubPaths(dir);
    if (!items.empty()) {
      for (const auto& d : items) {
        if (!Filesystem::IsDirectory(d)) {
          continue;
        }

        if (IsDescendantSelected(d)) {
          return true;
        }
      }
    }

    return false;
  }

  void ProjectPanel::SetSelectionContext(const Path& path) {
    selection = path;
  }

  void ProjectPanel::ValidateAndRenderSelectionCtx() const {
    if (selection.has_value()) {
      RenderContents(selection.value());    
      RenderContextMenu();
    } else {
      // render root 
    }
  }

} // namespace other 
