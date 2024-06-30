/**
 * \file rendering/ui/text_editor.cpp
 **/
#include "rendering/ui/text_editor.hpp"

#include <fstream>
#include <imgui/imgui.h>

#include "core/logger.hpp"
#include "core/filesystem.hpp"

#include "event/key_events.hpp"
#include "event/event_handler.hpp"

namespace other {
    
  void TextEditor::CreateEditor() {
    OE_INFO("Creating text editor for file: {}" , file_path);
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

    OE_INFO("Text editor created for file: {}" , file_path);
  }

  void TextEditor::OnAttach() {
    if (!Filesystem::FileExists(file_path)) {
      OE_WARN("File does not exist: {}" , file_path);
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
    if (zep == nullptr) {
      return;
    }
    zep->editor->RefreshRequired();
  }

  void TextEditor::OnEvent(Event* event) {
    if (zep == nullptr) {
      return;
    }

    EventHandler handler(event);
    handler.Handle<KeyPressed>([this](KeyPressed& key_event) -> bool {
      if (has_focus && key_event.Key() == Keyboard::Key::OE_ESCAPE) {
        return true;
      }
      return false;
    });
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

    if (!ImGui::Begin("Text Editor")) {
      ImGui::End();
      return;
    }

    has_focus = ImGui::IsWindowFocused();

    auto min = ImGui::GetCursorScreenPos();
    auto max = ImGui::GetContentRegionAvail();

    if (max.x <= 0) { 
      max.x = 1; 
    } 
    if (max.y <= 0) { 
      max.y = 1; 
    } 

    max.x = min.x + max.x;
    max.y = min.y + max.y;

    ImGui::InvisibleButton("EditorContainer" , max);

    zep->editor->SetDisplayRegion(Zep::NVec2f(min.x , min.y) , Zep::NVec2f(max.x , max.y));
    zep->editor->Display();
   
    /// idk why this has to happen here I wish it could be in update
    zep->HandleInput();

    if (has_focus) {
      uint32_t focus = 0;
      if (focus++ < 2) {
        ImGui::SetWindowFocus();
      }
    }


    ImGui::End();
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

  void TextEditor::HandleMessage(Zep::MessagePtr message) {
    zep->editor->Notify(message);
    if (message->str == "RequestQuit") {
      Close();
    }
  }

} // namespace other
