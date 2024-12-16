/**
 * \file editor/panels/project_panel.cpp
 **/
#include "editor/panels/project_panel.hpp"

#include <algorithm>
#include <filesystem>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <zep/theme.h>
#include <zep/window.h>

#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "input/mouse.hpp"
#include "project/project.hpp"

#include "rendering/ui/confirmation_window.hpp"
#include "rendering/ui/ui_helpers.hpp"

#include "editor/script_editor.hpp"

namespace other {

  void ProjectPanel::OnAttach() {
    OE_ASSERT(active_proj != nullptr, "Project Panel Project context is nullptr");
    OE_DEBUG("Attaching Project Panel");

    Ref<Directory> selection = Filesystem::GetDirectory("project-root");
    if (selection == nullptr) {
      OE_ERROR("Failed to get project root directory");
      return;
    }
    project_folders.root = NewRef<CBDirectory>(selection);

    // Ref<Directory> source_dir = Filesystem::GetDirectory("src");
    // if (source_dir == nullptr) {
    //   OE_ERROR("Failed to get source directory");
    //   return;
    // }
    // project_folders.source = NewRef<CBDirectory>(NewRef<Directory>(selection->ABsolut / "src"));
  }

  bool ProjectPanel::OnGuiRender(bool& is_open) {
    ImGui::SetNextWindowSize(ImVec2(200.f, ImGui::GetContentRegionAvail().y));
    if (!ImGui::Begin("Project", &is_open, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar)) {
      ImGui::End();
      return false;
    }

    if (active_proj == nullptr) {
      ImGui::End();
      return false;
    }

    ImGuiTableFlags table_flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV;
    if (ImGui::BeginTable("##project-table", 2, table_flags, ImVec2(0.f, 0.f))) {
      ImGui::TableSetupColumn("Outliner");
      ImGui::TableSetupColumn("Directory Structure", ImGuiTableColumnFlags_WidthStretch);

      ImGui::TableNextRow();

      /// project directory
      ImGui::TableSetColumnIndex(0);
      ImGui::BeginChild("##directory-items");
      {
        ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
        ScopedColorStack item_bg(ImGuiCol_Header, IM_COL32_DISABLE, ImGuiCol_HeaderActive, IM_COL32_DISABLE);

        RenderDirectoryTree(project_folders.root);
        // RenderDirectoryTree(project_folders.assets);
        // RenderDirectoryTree(project_folders.source);
      }
      ImGui::EndChild();

      /// Directory Content
      ImGui::TableSetColumnIndex(1);
      constexpr float top_bar_h = 26.f;
      constexpr float bottom_bar_h = 32.f;
      constexpr float tot_h_offset = top_bar_h + bottom_bar_h;
      ImGui::BeginChild("##project-directory-content", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight() - tot_h_offset));
      {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        RenderTopBar(top_bar_h);
        ImGui::PopStyleVar();

        ImGui::Separator();

        ImGui::BeginChild("Scrolling");
        {
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.35f));
          ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.f, 4.f));

          auto assets_dir = active_proj->GetMetadata().assets_dir;
          if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::BeginMenu("New")) {
              if (ImGui::MenuItem("Folder")) {}
              if (ImGui::MenuItem("Scene")) {}
              if (ImGui::MenuItem("Material")) {}

              if (ImGui::BeginMenu("Script")) {
                ImGui::EndMenu();
              }

              ImGui::EndMenu();
            }

            ImGui::EndPopup();
          }

          ImGui::PopStyleVar();

          const float padding_for_outline = 2.0f;
          const float scroll_barr_offset = 20.0f + ImGui::GetStyle().ScrollbarSize;
          float panel_w = ImGui::GetContentRegionAvail().x - scroll_barr_offset;
          float cell_size = 128 + 2.f + padding_for_outline;
          float col_count = (int32_t)(panel_w / cell_size);
          if (col_count < 1) {
            col_count = 1;
          }

          {
            const float row_spacing = 12.0f;
            ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(padding_for_outline, row_spacing));
            ImGui::Columns(col_count, 0, false);

            ScopedStyle border(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
            RenderItems();
          }

          ImGui::PopStyleColor(2);
        }
        ImGui::EndChild();
      }
      ImGui::EndChild();

      RenderBottomBar(bottom_bar_h);

      ImGui::EndTable();
    }

    ImGui::End();

    return false;
  }

  void ProjectPanel::OnUpdate(float dt) {
    for (auto& [id, window] : ui_windows) {
      window->Update(dt);
    }
  }

  void ProjectPanel::OnProjectChange(const Ref<Project>& project) {
    active_proj = project;
  }

  void ProjectPanel::SetSceneContext(const Ref<Scene>& scene) {
  }

  void ProjectPanel::OnScriptReload() {
    /// reset assets dir
    // project_folders.assets = nullptr;
    // project_folders.assets = NewRef<CBDirectory>(NewRef<Directory>(project_folders.root->directory->path / "assets"));

    // project_folders.asset_folders.clear();

    // for (auto& entry : std::filesystem::directory_iterator(project_folders.assets->directory->path)) {
    //   if (!entry.is_directory()) {
    //     return;
    //   }

    //   Ref<Directory> child_dir = NewRef<Directory>(entry.path());
    //   project_folders.asset_folders[child_dir->handle] = NewRef<CBDirectory>(child_dir);
    // }

    // if (selection_item.has_value()) {
    //   current_items = LoadItems(selection_item.value());
    // }
  }

  void ProjectPanel::RenderTopBar(float height) const {
    ImGui::BeginChild("##project_directory_contents_top_bar", ImVec2(0.f, height));
    ImGui::BeginHorizontal("##project_directory_contents_top_bar");
    {
      if (selection_item != nullptr) {
        /// construct navigation string
        ImGui::Text("%s", selection_item->directory->ProjectRelativePath().filename().string().c_str());
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
    ImGui::BeginChild("##project_directory_contents_bottom_bar", ImVec2(0.f, height));
    ImGui::BeginHorizontal("##project_directory_contents_bottom_bar");
    {
    }
    ImGui::EndHorizontal();
    ImGui::EndChild();
  }

  bool ProjectPanel::TreeNode(const std::string& id, const std::string& label, ImGuiTreeNodeFlags flags, const Ref<Texture2D>& icon) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) {
      return false;
    }

    return ui::TreeNodeWithIcon(icon, window->GetID(id.c_str()), flags, label.c_str(), nullptr);
  }

  void ProjectPanel::RenderDirectoryTree(const Ref<CBDirectory>& directory) {
    OE_ASSERT(directory != nullptr, "Directory is nullptr");
    OE_ASSERT(directory->directory != nullptr, "Directory is nullptr");

    ImGui::PushID(directory->directory->AbsolutePath().string().c_str());
    auto non_absolute = directory->directory->AbsolutePath().stem();
    auto files = Filesystem::GetSubDirs(directory->directory->AbsolutePath());

    std::string id_str = non_absolute.string();
    ImGuiID node_id = ImGui::GetID(id_str.c_str());
    bool prev_state = ImGui::TreeNodeBehaviorIsOpen(node_id);

    auto* window = ImGui::GetCurrentWindow();
    window->DC.CurrLineSize.y = 20.f;
    window->DC.CurrLineTextBaseOffset = 3.0f;

    const ImRect item_rect = {
      window->WorkRect.Min.x,
      window->DC.CursorPos.y,
      window->WorkRect.Min.y,
      window->DC.CursorPos.y + window->DC.CurrLineSize.y
    };

    const bool item_clicked = [&item_rect, &node_id]() -> bool {
      if (ImGui::ItemHoverable(item_rect, node_id, 0)) {
        return ImGui::IsMouseDown(ImGuiMouseButton_Left) ||
          ImGui::IsMouseReleased(ImGuiMouseButton_Left);
      }
      return false;
    }();

    const bool window_focused = ImGui::IsWindowFocused();

    auto fill_w_color = [&](const ImColor& color) {
      const ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(color);
      ImGui::GetWindowDrawList()->AddRectFilled(item_rect.Min, item_rect.Max, bg_color);
    };

    const bool any_descendant_selected = IsDescendantSelected(selection_item);
    const bool is_active_dir = IsDirSelected(directory);

    ImGuiTreeNodeFlags flags = (is_active_dir ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanFullWidth;
    if (directory->children.size() == 0) {
      flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (item_clicked) {
      SetSelectionContext(directory);
    }

    if (is_active_dir || item_clicked) {
      if (window_focused) {
        fill_w_color(ui::theme::selection);
      } else {
        const ImColor col = ui::theme::ColorWithMultiplier(ui::theme::selection, 0.8f);
        fill_w_color(ui::theme::ColorWithMultipliedSaturation(col, 0.7f));
      }

      ImGui::PushStyleColor(ImGuiCol_Text, ui::theme::background_dark);
    } else if (any_descendant_selected) {
      fill_w_color(ui::theme::selection_muted);
    }

    bool open = ImGui::TreeNodeEx(non_absolute.string().c_str(), flags);
    // bool open = TreeNode(id_str, non_absolute.string(), flags, EditorImages::folder_icon);

    if (is_active_dir || item_clicked) {
      ImGui::PopStyleColor();
    }

    ui::ShiftCursorY(3.0f);

    // create context menu

    if (open) {
      for (auto& child : directory->children) {
        RenderDirectoryTree(child);
      }

      ImGui::TreePop();
    } else if (prev_state) {
      /// not open on this frame but was open last frame
    }

    ImGui::PopID();
  }

  template <typename T>
  concept id_t = requires(T t) {
    t.first.Get();
  };

  void ProjectPanel::RenderScriptObjectContextMenu(const ScriptObjectTag& tag) {
    if (ImGui::BeginPopupContextWindow((tag.name + "-context-menu").c_str())) {
      if (ImGui::BeginMenu("Options")) {
        // if (ImGui::MenuItem(("Delete " + tag.name).c_str())) {
        //   Ref<UIWindow> confirmation_window = Ref<ConfirmationWindow>::Create(
        //     /* window name and warning msg */
        //     "Confirm File Deletion", fmtstr("This action is destructive and irreversible!\n Are you sure you want to delete {}", tag.path),
        //     /* action on confirmation */
        //     [object = tag]() {
        //       if (!Filesystem::RemoveFile(object.path)) {
        //         OE_ERROR("Failed to delete script {} : {}", object.name, object.path);
        //       }
        //     }
        //   );
        //   // GetEditor().PushUIWindow(confirmation_window);
        //   ImGui::CloseCurrentPopup();
        // }
        ImGui::EndMenu();
      }

      ImGui::EndPopup();
    }
  }

  void ProjectPanel::ScriptObjectDragDropSource(UUID dir_tag, const ScriptObjectTag& tag) {
    std::string dd_tag = drag_drop_tags.at(dir_tag).data();
    ImGuiDragDropFlags dd_flags = ImGuiDragDropFlags_SourceNoDisableHover;
    if (ImGui::BeginDragDropSource(dd_flags)) {
      ImGui::Text("%s", tag.name.c_str());
      ImGui::SetDragDropPayload(dd_tag.c_str(), tag.name.c_str(), tag.name.length() + 1);
      ImGui::EndDragDropSource();
    }
  }

  std::string ProjectPanel::GetContextMenuTagFromSelection() const {
    if (selection_item == nullptr) {
      return "";
    }

    auto itr = std::ranges::find_if(tags, [&](const auto& tag_pair) -> bool {
      std::string tag = selection_item->directory->ProjectRelativePath().stem().string();
      return tag_pair.first.Get() == FNV(tag);
    });

    if (itr == tags.end()) {
      return "";
    }

    return std::string{ itr->second };
  }

  template <typename Fn>
  void DrawContextMenu(const std::string_view tag, Fn callback) {
    if (ImGui::BeginPopup(("##project-context-" + std::string{ tag }).c_str())) {
      callback();
      ImGui::EndPopup();
    }
  }

  void ProjectPanel::RenderContextMenu() {
    if (ImGui::IsWindowHovered() && Mouse::Pressed(Mouse::Button::RIGHT)) {
      std::string tag = GetContextMenuTagFromSelection();
      if (tag.empty()) {
        return;
      }

      selection_tag = tag;
    } else if (!ImGui::IsWindowHovered()) {
      selection_tag = std::nullopt;
    }

    if (selection_tag.has_value()) {
      ImGui::OpenPopup(("##project-context-" + selection_tag.value()).c_str());
    }

    DrawContextMenu(tags.at(FNV("editor")), [] {
    });

    DrawContextMenu(tags.at(FNV("materials")), [] {
    });

    DrawContextMenu(tags.at(FNV("scenes")), [] {
    });

    DrawContextMenu(tags.at(FNV("scripts")), [this] {
      ImGui::Text("Script Context Menu");

      if (ImGui::Button("Close")) {
        ImGui::CloseCurrentPopup();
        selection_tag = std::nullopt;
      }
    });

    DrawContextMenu(tags.at(FNV("shaders")), [] {
    });
  }

  bool ProjectPanel::IsDirSelected(const Ref<CBDirectory>& dir) const {
    if (selection_item == nullptr) {
      return false;
    }

    OE_ASSERT(dir != nullptr, "Directory is nullptr");
    if (!dir->IsSelected()) {
      return false;
    }
    OE_ASSERT(selection_item->directory != nullptr, "Selection item is nullptr");
    OE_ASSERT(dir->directory != nullptr, "Directory is nullptr");
    OE_ASSERT(selection_item->directory->AbsolutePath() == dir->directory->AbsolutePath(), "Selection item and directory are not the same");
    return true;
  }

  bool ProjectPanel::IsDescendantSelected(const Ref<CBDirectory>& dir) const {
    if (selection_item == nullptr) {
      return false;
    }

    for (const Ref<CBDirectory>& child : dir->children) {
      if (IsDirSelected(child)) {
        return true;
      }

      if (IsDescendantSelected(child)) {
        return true;
      }
    }

    return false;
  }

  void ProjectPanel::SetSelectionContext(const Ref<CBDirectory>& dir) {
    OE_ASSERT(dir != nullptr, "Directory is nullptr");
    if (IsDirSelected(dir)) {
      selection_item->Deselect();
      selection_item = nullptr;
      return;
    }

    if (selection_item != nullptr) {
      selection_item->Deselect();
    }

    selection_item = dir;
    current_items = LoadItems(selection_item);
    selection_item->Select();
  }

  void ProjectPanel::ActivateAsset(const Ref<CBItem>& asset) {
    Ref<FileHandle> handle = Filesystem::GetFile(asset->asset_data->AbsolutePath());
    if (handle == nullptr) {
      OE_ERROR("Failed to get file handle for {}", asset->asset_data->AbsolutePath());
      return;
    }

    switch (handle->GetAssetType()) {
      case SCENE: {
      } break;
      // case DYNAMIC_LIBRARY: {
      // } break;
      default:
        OE_DEBUG("Implement asset activation {}", handle->GetAssetType());
        break;
    }
  }

  void ProjectPanel::RenderItems() {
    for (auto& itm : current_items) {
      bool set_selection_ctx = false;
      bool asset_activated = false;

      itm->OnRenderBegin();

      CBActionResult result = itm->Render();

      // if (result.Has(CBAction::ACTIVATED)) {
      //   asset_activated = true;
      // }

      if (result.Has(CBAction::SELECTED)) {
        set_selection_ctx = true;
      }

      if (result.Has(CBAction::DESELECTED)) {
      }

      // if (result.Has(CBAction::START_RENAMING)) {}

      if (result.Has(CBAction::RELOAD)) {}

      if (result.Has(CBAction::OPEN_NATIVE)) {}

      if (result.Has(CBAction::OPEN_EXTERNAL)) {}

      if (result.Has(CBAction::HOVERED)) {}

      if (result.Has(CBAction::RENAMED)) {}

      itm->OnRenderEnd();

      if (set_selection_ctx && itm->GetType() == ContentBrowserItem::Type::DIRECTORY) {
        SetSelectionContext(itm.As<CBDirectory>());
        return;
      } else if (asset_activated) {
        ActivateAsset(itm.As<CBItem>());
      }
    }
  }

  CBItemList ProjectPanel::LoadItems(const Ref<CBDirectory>& dir) {
    CBItemList items;

    for (auto& [id, child] : dir->directory->children) {
      items.push_back(NewRef<CBDirectory>(child));
    }

    for (Path& path : dir->directory->GetFilePaths()) {
      Ref<FileHandle> file = Filesystem::GetFile(dir->directory->AbsolutePath() / path);
      if (file == nullptr) {
        continue;
      }

      Ref<Texture2D> icon = EditorImages::GetIconFromExtension(path.extension().string());
      items.push_back(NewRef<CBItem>(file, icon));
    }

    return items;
  }

}  // namespace other
