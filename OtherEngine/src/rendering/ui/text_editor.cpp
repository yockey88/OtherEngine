/**
 * \file rendering/ui/text_editor.cpp
 **/
#include "rendering/ui/text_editor.hpp"

#include <fstream>
#include <imgui/imgui.h>

#include "core/logger.hpp"
#include "core/filesystem.hpp"

namespace other {
    
  void TextEditor::CreateEditor() {
    zep = NewScope<ZepWrapper>(root , pixel_scale , callback);

    auto& display = zep->editor->GetDisplay();
    auto font = ImGui::GetIO().Fonts->Fonts[0];
    auto pixel_height = font->FontSize;

    display.SetFont(Zep::ZepTextType::UI , std::make_shared<Zep::ZepFont_ImGui>(display , font , (uint32_t)pixel_height));
    display.SetFont(Zep::ZepTextType::Text , std::make_shared<Zep::ZepFont_ImGui>(display , font , (uint32_t)pixel_height));
    display.SetFont(Zep::ZepTextType::Heading1 , std::make_shared<Zep::ZepFont_ImGui>(display , font , (uint32_t)pixel_height * 1.5));
    display.SetFont(Zep::ZepTextType::Heading2 , std::make_shared<Zep::ZepFont_ImGui>(display , font , (uint32_t)pixel_height * 1.25));
    display.SetFont(Zep::ZepTextType::Heading3 , std::make_shared<Zep::ZepFont_ImGui>(display , font , (uint32_t)pixel_height * 1.125));

    buffer = zep->editor->InitWithFile(file_path);
  }

  void TextEditor::OnAttach() {
    if (!Filesystem::FileExists(file_path)) {
      return;
    }

    file_exists = true;
    CreateEditor();
  }
      
  void TextEditor::OnDetach() {
    if (file_exists) {
      zep->Destroy();
    }
    zep = nullptr;
  }
 
  void TextEditor::OnUpdate(float dt) {
    zep->editor->RefreshRequired();
  }
  
  void TextEditor::Render() {
    if (!file_exists) {
      ImGui::Text(
        "File does not exist: %s. Create file?" , 
        (root + "/" + file).c_str()
      );
      if (ImGui::Button("Yes")) {
        std::ofstream file(file_path);
        file.close();
        file_exists = true;
        CreateEditor();
      } else if (ImGui::Button("No")) {
        Close();
      }
    }

    if (zep == nullptr) {
      return;
    }

    auto min = ImGui::GetCursorScreenPos();
    auto max = ImGui::GetContentRegionAvail();

    if (max.x <= 0) max.x = 1;
    if (max.y <= 0) max.y = 1;

    ImGui::InvisibleButton("EditorContainer" , max);

    max.x = min.x + max.x;
    max.y = min.y + max.y;

    zep->editor->SetDisplayRegion(Zep::NVec2f(min.x , min.y) , Zep::NVec2f(max.x , max.y));
    zep->editor->Display();
   
    zep->HandleInput();

    uint32_t focus = 0;
    if (focus++ < 2) {
        ImGui::SetWindowFocus();
    }
  }
      
  void TextEditor::LoadFile(const std::string& file) {
    if (!Filesystem::FileExists(root + "/" + file)) {
      OE_ERROR("Failed to open file: {}", root);
      return;
    }

    this->file = file;
    file_path = root + "/" + file;
    zep->editor->InitWithFile(file_path);
  }

} // namespace other