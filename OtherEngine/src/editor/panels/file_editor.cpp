/**
 * \file editor/panels/file_editor.cpp
 **/
#include "editor/panels/file_editor.hpp"

#include <imgui/imgui.h>

#include "core/filesystem.hpp"

#include "event/event_queue.hpp"
#include "event/key_events.hpp"

#include "rendering/ui/ui_helpers.hpp"

namespace other {

  void FileEditor::OnAttach() {
    shader_directory = Filesystem::GetDirectory("project-root");
    OE_ASSERT(shader_directory != nullptr, "Failed to retrieve assets directory");

    EventQueue::RegisterEventDispatcher<KeyPressed>(
      "ShaderCreator--KeyPress",
      {
        [&](const KeyPressed& e) -> bool {
          if (e.Key() == Keyboard::Key::OE_ESCAPE && editor != nullptr) {
            /// simply intercept the escape key if editor is open
            return true;
          }
          return false;
        },
      }
    );
  }

  void FileEditor::OnDetach() {
    EventQueue::UnregisterEventDispatcher("ShaderCreator--KeyPress");
  }

  void FileEditor::OnUpdate(float dt) {
    if (editor != nullptr) {
      editor->OnUpdate(dt);
    }
  }

  bool FileEditor::OnGuiRender(bool& is_open) {
    // clang-format off
    if (!ImGui::Begin("Shader Editor", &is_open, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | 
                                                 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
      // clang-format on
      ImGui::End();
      return false;
    }

    ui::MenuBar([&]() {});

    RenderDirectory(shader_directory);

    if (editor != nullptr) {
      editor->OnRender();
      if (!editor->IsOpen()) {
        editor->OnDetach();
        editor = nullptr;
      }
    }

    ImGui::End();
    return false;
  }

  void FileEditor::RenderDirectory(Ref<Directory>& dir) {
    OE_ASSERT(dir != nullptr, "Directory is null");
    if (!ui::BeginTreeNode(dir->Name().c_str(), false)) {
      return;
    }
    ImGui::PushID(dir->handle.Get());

    ui::Button("Create File", [&]() {
      new_file_name = std::array<char, 256>();
      new_file_name->fill('\0');
      ImGui::OpenPopup("Create File");
    });

    if (ImGui::BeginPopup("Create File")) {
      OE_ASSERT(new_file_name.has_value(), "New file name is null");

      ImGui::InputText("File Name", new_file_name->data(), new_file_name->size());
      if (ImGui::Button("Create")) {
        std::string file_name(new_file_name->data());
        dir->AddFile(file_name);
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImGui::Text("%llu files", dir->file_handles.size());
    for (auto& [id, f] : dir->file_handles) {
      RenderFile(f);
    }

    for (auto& [id, c] : dir->children) {
      RenderDirectory(c);
    }

    ImGui::PopID();
    ui::EndTreeNode();
  }

  void FileEditor::RenderFile(Ref<FileHandle>& file) {
    if (!ui::BeginTreeNode(file->ProjectRelativePath().filename().string().c_str(), false)) {
      return;
    }
    ImGui::PushID(file->handle.Get());

    ui::Button("Open File", [&]() {
      editor = NewRef<TextEditor>(file->FileName(), shader_directory->AbsolutePath().string(), file->AbsolutePath().string());
      editor->OnAttach();
    });

    ui::Button("Options", [&]() {
      ImGui::OpenPopup("File Options");
    });

    if (ImGui::BeginPopup("File Options")) {
      ImGui::EndPopup();
    }

    ImGui::PopID();
    ui::EndTreeNode();
  }

}  // namespace other
