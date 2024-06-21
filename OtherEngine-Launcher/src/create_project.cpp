/**
 * \file create_project.cpp
 **/
#include "create_project.hpp"

#include <fstream>

#include "core/logger.hpp"
#include "core/filesystem.hpp"

#include "application/app.hpp"

#include "event/event.hpp"
#include "event/key_events.hpp"
#include "event/event_queue.hpp"
#include "event/event_handler.hpp"

#include "rendering/ui/ui_helpers.hpp"

namespace other {
  
  void CreateProjectLayer::OnAttach() {
    OE_ASSERT(!project_path.empty() , "Project path is empty");
    OE_ASSERT(Filesystem::IsDirectory(project_path) , "Project path does not exist: {}", project_path);

    for (const auto& entry : std::filesystem::directory_iterator(project_path)) {
      project_dir_contents.push_back(entry.path());
    }

    render_state = RenderState::MAIN;
  }

  void CreateProjectLayer::OnUpdate(float dt) {
    if (!rendering_main) {
      ParentApp()->PopLayer();
      return;
    }

    if (!project_name.has_value()) {
      return;
    }

    if (project_name.value().empty()) {
      OE_WARN("Project name can not be an empty string");
      project_name = std::nullopt;
    }

    OE_INFO("Project name: {}" , project_name.value());
    final_name = project_name.value();

    if (!Filesystem::CreateDir(project_path + "/" + project_name.value())) {
      OE_WARN("Failed to create project: {}" , project_name.value());
      project_name = std::nullopt;
    }

    std::string cfg_file = project_name.value() + ".other";

    project_dir = std::filesystem::absolute(std::filesystem::path(project_path)) / project_name.value();
    project_file = project_dir / cfg_file;

    std::ofstream file(project_file);
    if (!file.is_open()) {
      OE_WARN("Failed to create project file: {}" , project_name.value());
      project_name = std::nullopt;
    }


    project_dir_contents.clear();
    for (const auto& entry : std::filesystem::directory_iterator(project_dir)) {
      project_dir_contents.push_back(entry.path());
    }

    render_state = RenderState::CREATING_PROJECT;

    // write project metadata
    file << "[project]\n";
    file << "name = " << project_name.value() << "\n";
    file << "path = " << project_dir.string() << "\n";

    // write project settings
    // write bare minimum engine settings
    // create project directory structure
    // write build scripts
    
    ParentApp()->PushUIWindow(NewRef<TextEditor>("Project Editor" , "." , project_file.string()));

    project_name = std::nullopt;
  }

  void CreateProjectLayer::OnEvent(Event* event) {
    EventHandler handler(event);
    handler.Handle<KeyPressed>([this](KeyPressed& key_event) -> bool {
      if (key_event.Key() == Keyboard::Key::OE_RETURN && render_state == RenderState::MAIN) {
        project_name = CaptureBuffer(project_name_buffer);
        return true;
      } 
      return false;
    });
  }

  void CreateProjectLayer::OnUIRender() {
    switch (render_state) {
      case RenderState::MAIN:
        RenderMain();
        break;
      case RenderState::CREATING_PROJECT: 
        CreatingProject();
        break;
      default: {
        OE_ASSERT(false , "Invalid render state");
      } break;
    }
  }

  void CreateProjectLayer::RenderDirContents() {
    for (const auto& entry : project_dir_contents) {
      ImGui::Text("%s" , entry.filename().string().c_str());
    }

    ui::Underline();
    ImGui::Separator();
  }

  void CreateProjectLayer::RenderMain() {
    bool open = ImGui::Begin("Create Project" , &rendering_main);
    if (!open) {
      ImGui::End();
      return;
    }

    ImGui::Text("Project Path: %s" , project_path.c_str());

    ui::Underline();
    ImGui::Separator();

    if (text_editor_id.Get() != 0) {
      ImGui::End();
      return;
    }

    if (ImGui::Button("Create Project")) {
      project_name = CaptureBuffer(project_name_buffer);
    }
    ImGui::SameLine();
    ImGui::InputText("Project Name" , project_name_buffer.data() , kBufferSize);

    ImGui::End();
  }

  void CreateProjectLayer::CreatingProject() {
    bool open = ImGui::Begin("Creating Project" , &rendering_create_project);
    if (!open) {
      ImGui::End();
      return;
    }

    RenderDirContents();

    ImGui::Text("Creating Project: %s" , final_name.c_str());
    ImGui::Text("Project Path: %s" , project_dir.string().c_str());

    ui::Underline();
    ImGui::Separator();

    auto item = [&](Opt<std::string>& out , const std::string& label , std::array<char , kBufferSize>& buffer) {
      if (ImGui::Button("Confirm")) {
        out = CaptureBuffer(buffer);
      }
      ImGui::SameLine();
      ImGui::InputText(label.c_str() , buffer.data() , kBufferSize);
    };

    item(author , "Author" , author_buffer);
    item(description , "Description" , description_buffer);

    ImGui::End();
  }

  std::string CreateProjectLayer::CaptureBuffer(const std::array<char , kBufferSize>& buffer) {
    auto str = std::string(buffer.data());
    project_name_buffer.fill(0);
    return str;
  }

} // namespace other
