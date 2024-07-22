/**
 * \file editor/content_browser_item.cpp
 **/
#include "editor/content_browser_item.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "asset/asset_metadata.hpp"
#include "input/keyboard.hpp"
#include "application/app_state.hpp"

#include "rendering/ui/ui_colors.hpp"
#include "rendering/ui/ui_helpers.hpp"

#include "editor/editor_settings.hpp"
#include "editor/editor_asset_handler.hpp"

namespace other {

  void ContentBrowserItem::OnRenderBegin() {
    ImGui::PushID(handle.Get());
    ImGui::BeginGroup();
  }

  CBActionResult ContentBrowserItem::Render() {
    CBActionResult result;
    const auto& editor_settings = EditorSettings::Get();
    const float thumbnail_size = editor_settings.thumbnail_size;
    const bool display_asset_type = false;

    SetDisplayName();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing , ImVec2(0.f , 0.f));

    const float edge_offset = 4.f;
    const float text_line_h = ImGui::GetTextLineHeightWithSpacing() * 2.f + edge_offset * 2.f;
    const float info_panel_h = glm::max(display_asset_type ? thumbnail_size * 2.f : text_line_h , text_line_h);

    const ImVec2 top_l = ImGui::GetCursorScreenPos();
    const ImVec2 thumb_bot_r = { top_l.x + thumbnail_size , top_l.y + thumbnail_size };
    const ImVec2 info_top_l  = { top_l.x , top_l.y + thumbnail_size };
    const ImVec2 bot_r       = { top_l.x + thumbnail_size , top_l.y + thumbnail_size * info_panel_h };

    auto draw_shadow = [](const ImVec2& top_l , const ImVec2& bot_r , bool directory) {
      auto* draw_list = ImGui::GetWindowDrawList();
      const ImRect item_rect = ui::RectOffset(ImRect(top_l , bot_r) , 1.f , 1.f);
      draw_list->AddRect(item_rect.Min , item_rect.Max , ui::theme::property_field , 6.f , directory ? 0 : ImDrawFlags_RoundCornersBottom);
    };

    const bool focused = ImGui::IsWindowFocused();

    if (type == Type::DIRECTORY) {
      draw_shadow(top_l , bot_r , false);
      
      auto* draw_list = ImGui::GetWindowDrawList();
      draw_list->AddRectFilled(top_l , bot_r , ui::theme::no_color , 6.f);
    } else if (ImGui::ItemHoverable(ImRect(top_l , bot_r), ImGui::GetID(&handle) , ImDrawFlags_RoundCornersBottom)) {
      auto* draw_list = ImGui::GetWindowDrawList();
      draw_shadow(top_l , bot_r, true);

      draw_list->AddRectFilled(top_l , thumb_bot_r , ui::theme::background_dark);
      draw_list->AddRectFilled(info_top_l , bot_r , ui::theme::group_header , 6.f , ImDrawFlags_RoundCornersBottom);
    }

    ImGui::InvisibleButton("##thumbnail-button" , ImVec2{ thumbnail_size , thumbnail_size });
    ui::DrawButtonImage(icon , IM_COL32(255 , 255 , 255 , 255) , IM_COL32(255 , 255 , 255 , 255) , IM_COL32(255 , 255 , 255 , 255) ,
                        ui::RectExpanded(ui::GetItemRect() , -6.f , -6.f));

    /// renaming
    auto renaming_widget = [&]{
      ImGui::SetKeyboardFocusHere();
      ImGui::InputText("##rename-item" , rename_buffer.data() , kMaxBufferSize);

      if (ImGui::IsItemDeactivatedAfterEdit() || Keyboard::Pressed(Keyboard::Key::OE_RETURN)) {
        Rename(std::string{ rename_buffer.data() });
        renaming = false;
        SetDisplayName();
        result.Set(CBAction::RENAMED , true);
      }
    };

    ui::ShiftCursor(edge_offset , edge_offset);
    if (type == Type::DIRECTORY) {
      ImGui::BeginVertical((std::string("InfoPanel") + display_name).c_str() , ImVec2(thumbnail_size - edge_offset * 2.f , info_panel_h - edge_offset));
      {
        ImGui::BeginHorizontal(filename.c_str() , ImVec2(thumbnail_size - 2.f , 0.f));
        ImGui::Spring();
        {
          ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + (thumbnail_size - edge_offset * 2.f));
          const float text_w = std::min(ImGui::CalcTextSize(display_name.c_str()).x , thumbnail_size);
          if (renaming) {
            ImGui::SetNextItemWidth(thumbnail_size - edge_offset * 3.f);
            renaming_widget();
          } else {
            ImGui::SetNextItemWidth(text_w);
            ImGui::Text("%s" , display_name.c_str());
          }

          ImGui::PopTextWrapPos();
        }
        ImGui::Spring();
        ImGui::EndHorizontal();

        ImGui::Spring();
      }
      ImGui::EndVertical();
    } else {
      ImGui::BeginVertical(("InfoPanel" + display_name).c_str() , ImVec2(thumbnail_size - edge_offset * 3.f , info_panel_h - edge_offset));
      {
        ImGui::BeginHorizontal("label" , ImVec2(0.f , 0.f));

        ImGui::SuspendLayout();
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + (thumbnail_size - edge_offset * 2.f));
        if (renaming) {
          ImGui::SetNextItemWidth(thumbnail_size - edge_offset * 3.f);
          renaming_widget();
        } else {
          ImGui::Text("%s" , display_name.c_str());
        }
        ImGui::PopTextWrapPos();
        ImGui::ResumeLayout();
        
        ImGui::Spring();
        ImGui::EndHorizontal();
      }
      ImGui::Spring();
      if (display_asset_type) {
        ImGui::BeginHorizontal("asset-type" , ImVec2(0.f , 0.f));
        ImGui::Spring();
        {
          const AssetMetadata& metadata = AppState::Assets().As<EditorAssetHandler>()->GetMetadata(handle);
          using namespace std::string_view_literals;
          ImGui::Text("%s" , fmt::format("{}"sv , metadata.type).c_str());
          // std::string asset_type = 
        }
        ImGui::EndHorizontal();

        ImGui::Spring(-1.f , edge_offset);
      }
      ImGui::EndVertical();
    }
    ui::ShiftCursor(-edge_offset , -edge_offset);

    if (!renaming) {
      if (Keyboard::Down(Keyboard::Key::OE_F2) && ImGui::IsItemHovered()) {
        StartRenaming();
      }
    }
    
    ImGui::PopStyleVar();
    ImGui::EndGroup();

    if (selected || ImGui::IsItemHovered()) {
      ImRect rect = ui::GetItemRect();
      auto* drawlist = ImGui::GetWindowDrawList();

      if (selected) {
        const bool mouse_down = ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsItemHovered();
        ImColor col_transition = ui::theme::ColorWithMultiplier(ui::theme::selection , 0.8f);
        drawlist->AddRect(rect.Min , rect.Max , mouse_down ? ImColor(col_transition) : ImColor(ui::theme::selection) , 6.f ,
                          type == Type::DIRECTORY ? 0 : ImDrawFlags_RoundCornersBottom , 1.f);
      } else if (type == Type::DIRECTORY) {
        drawlist->AddRect(rect.Min , rect.Max , ui::theme::muted , 6.f , ImDrawFlags_RoundCornersBottom , 1.f);
      }
    }

    UpdateDrop(result);
    
    bool start_dragging = false;
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      start_dragging = true;
      dragging = true;

      OE_DEBUG("started dragging");

      void* tex_id = (void*)(uintptr_t)icon->GetRendererId();
      ImGui::Image(tex_id , ImVec2(20 , 20) , ImVec2(0 , 0) , ImVec2(1 , 1) , ImVec4(1 , 1 , 1 , 1) , ImVec4(0 , 0 , 0 , 0));
      ImGui::TextUnformatted(filename.c_str());

      ImGui::SetDragDropPayload("asset-payload" , &handle , sizeof(AssetHandle));

      result.Set(CBAction::SELECTED , true);
      ImGui::EndDragDropSource();
    }

    if (ImGui::IsItemHovered()) {
      result.Set(CBAction::HOVERED , true);

      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !renaming) {
        result.Set(CBAction::ACTIVATED , true);
      } else {
        bool action = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        bool skip_bc_dragging = dragging && selected;
        if (action && !skip_bc_dragging) {
          if (just_selected) {
            just_selected = false;
          }

          if (!selected) {
            result.Set(CBAction::SELECTED , true);
            just_selected = true;
          }
        }
      }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing , ImVec2(4.f , 4.f));
    if (ImGui::BeginPopupContextItem("CBItemContextMenu")) {
      result.Set(CBAction::SELECTED , true);
      OnContextMenuOpen(result);
      ImGui::EndPopup();
    }
    ImGui::PopStyleVar();

    dragging = start_dragging;

    return result; 
  }

  void ContentBrowserItem::OnRenderEnd() {
    ImGui::PopID();
    ImGui::NextColumn();
  }

  ContentBrowserItem::Type ContentBrowserItem::GetType() const {
    return type;
  }

  AssetHandle ContentBrowserItem::GetHandle() const {
    return handle;
  }

  const std::string& ContentBrowserItem::GetFilename() const {
    return filename;
  }

  const Ref<Texture>& ContentBrowserItem::GetIcon() const {
    return icon;
  }

  void ContentBrowserItem::OnContextMenuOpen(CBActionResult& result) {
  }
      
  void ContentBrowserItem::SetDisplayName() {
    const auto& editor_settings = EditorSettings::Get(); 

    int32_t max_chars = 0.001525875f * (editor_settings.thumbnail_size * editor_settings.thumbnail_size);

    if (filename.size() > max_chars) {
      display_name = filename.substr(0 , max_chars) + "...";
    } else {
      display_name = filename;
    }

    display_name = OverrideDisplayName(display_name);
  }

  void CBDirectory::Delete() {
  }

  bool CBDirectory::Move(const Path& path) {
    return false; 
  }

  void CBDirectory::OnRenamed(const std::string& name) {
  }

  void CBDirectory::UpdateDrop(CBActionResult& result) {
    if (ImGui::BeginDragDropTarget()) {
    //   const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("asset-payload");

    //   if (payload != nullptr) {
    //   }

    //   ImGui::EndDragDropTarget();
    }
  }

  void CBDirectory::UpdateDirectoryPath(Ref<Directory> directory , const Path& new_parent , const Path& new_name) {
  }
  
  CBItem::CBItem(const AssetMetadata& metadata , const Ref<Texture2D>& texture) 
      : ContentBrowserItem(Type::FILE , metadata.handle, metadata.path.filename().string() , texture) ,
        asset_metadata(metadata) {
  }

  void CBItem::Delete() {
  }

  bool CBItem::Move(const Path& path) {
    return false; 
  }

  void CBItem::OnRenamed(const std::string& name) {
  }
      
  std::string CBItem::OverrideDisplayName(const std::string& new_name) {
    if (asset_metadata.type == SOURCE_FILE) {
      return new_name;
    }
    return new_name.substr(0 , new_name.find_first_of('.'));
  }
    
  size_t CBItemList::size() {
    return items.size();
  }

  void CBItemList::clear() {
    items.clear();
  }
  
  void CBItemList::erase(AssetHandle handle) {
    size_t idx = find(handle); 

    if (idx == invalid_idx) {
      return;
    }

    auto itr = begin() + idx;
    items.erase(itr);
  }

  size_t CBItemList::find(AssetHandle handle) {
    if (items.empty()) {
      return invalid_idx;
    }

    for (size_t i = 0; i < items.size(); ++i) {
      if (items[i]->GetHandle() == handle) {
        return i;
      }
    }

    return invalid_idx;
  }
    
  void CBItemList::push_back(const Ref<ContentBrowserItem>& item) {
    items.push_back(item);
  }

} // namespace other
