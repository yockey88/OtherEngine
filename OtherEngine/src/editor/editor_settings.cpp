/**
 * \file editor/editor_settings.cpp
 **/
#include "editor/editor_settings.hpp"

#include "rendering/ui/ui_helpers.hpp"

namespace other {

  EditorSettings& EditorSettings::Get() {
    static EditorSettings settings;
    return settings;
  }
      
  SettingsWindow::SettingsWindow()
      : UIWindow("Settings" , ImGuiWindowFlags_MenuBar) {
    PushRenderFunction([this]{
      if (ImGui::BeginMenuBar()) {
        ImGui::EndMenuBar();
      }
    });

    PushRenderFunction([this]{
      ImGuiTableFlags table_flags = ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_BordersInnerV;

      if (ImGui::BeginTable("##setting-table" , 2 , table_flags , ImVec2(0.f , 0.f))) {
        ImGui::TableSetupColumn("Categories");
        ImGui::TableSetupColumn("Settings Column" , ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();

        /// settings categories
        ImGui::TableSetColumnIndex(0);
        ImGui::BeginChild("##settings-categories");
        {
          ScopedStyle spacing(ImGuiStyleVar_ItemSpacing , ImVec2(0 , 0));
          ScopedColorStack item_bg(ImGuiCol_Header , IM_COL32_DISABLE ,
                                   ImGuiCol_HeaderActive , IM_COL32_DISABLE);
          /// render categories
          if (ImGui::Selectable("Editor")) {
            /// context switch
          }
        }
        ImGui::EndChild();
        
        ImGui::TableSetColumnIndex(1);
        constexpr float top_bar_h = 26.f;
        constexpr float bottom_bar_h = 32.f;
        constexpr float tot_h_offset = top_bar_h + bottom_bar_h;
        ImGui::BeginChild("##settings" ,
                          ImVec2(ImGui::GetContentRegionAvail().x , ImGui::GetWindowHeight() - tot_h_offset)); 
        {
          ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize , 0.f);
          // top bar
          ImGui::PopStyleVar();

          /// scrolling
          EditorSettings();

          // bottom bar
        }
        ImGui::EndChild();

        ImGui::EndTable();
      }
    });
  }
          
  void SettingsWindow::EditorSettings() {
    ui::BeginPropertyGrid();

    ui::Property("Icon Size" , &EditorSettings::Get().thumbnail_size);

    ui::EndPropertyGrid();
  }

} // namespace other
