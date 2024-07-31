/**
 * \file create_project.cpp
 **/
#include "create_project.hpp"

#include <core/platform.hpp>
#include <filesystem>
#include <fstream>
#include <optional>

#include "core/logger.hpp"
#include "core/filesystem.hpp"

#include "application/app.hpp"

#include "event/event.hpp"
#include "event/key_events.hpp"
#include "event/event_queue.hpp"
#include "event/event_handler.hpp"

#include "project_cache.hpp"
#include "rendering/ui/ui_helpers.hpp"

namespace other {

  /// generate folders
  ///   - logs/
  ///   - <Project-Name>/
  ///     - <Project-Name>/assets/
  ///       - <Project-Name>/assets/editor
  ///       - <Project-Name>/assets/materials
  ///       - <Project-Name>/assets/scenes
  ///       - <Project-Name>/assets/scripts
  ///       - <Project-Name>/assets/shaders
  ///     - <Project-Name>/src/
  ///   - <project-name>.other
  ///
  /// copy over premake dir and ymake.lua
  /// create premake5.lua
  ///
  /// create <Project-Name>/premake5.lua
  ///
  /// create <Project-Name>/src/<project-name>.cpp
  /// create <Project-Name>/src/<project-name>.hpp
  
  void CreateProjectLayer::OnAttach() {
    OE_ASSERT(!project_path.empty() , "Project path is empty");
    
    if (!CreateProjectFolder()) {
      OE_ERROR("Failed to create project directory!");

      /// this is always going to be the top layer
      ParentApp()->PopLayer();
      return;
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
      return;
    }

    if (!CreateCoreProjectFiles()) {
      OE_ERROR("Failed to create core project files!");
      ParentApp()->PopLayer();
      return;
    }
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
      
  bool CreateProjectLayer::CreateProjectFolder() {
    if (!Filesystem::PathExists(project_path)) {
      if (!Filesystem::CreateDir(project_path)) {
        return false;
      }
    }

    project_dir = project_path;
    Path logs = project_dir / "logs";
    Path premake = project_dir / "premake";
    Path make = project_dir / "ymake.lua";
    Path imguiini = project_dir / "imgui.ini";

    if (!Filesystem::CreateDir(logs)) {
      return false;
    }

    if (!Filesystem::CreateDir(premake)) {
      return false;
    }
    
    Path engine_core = Filesystem::GetEngineCoreDir();
    Path engine_premake = engine_core / "premake";
    Path engine_make = engine_core / "ymake.lua";
    Path engine_imguiini = engine_core / "OtherEngine-Launcher" / "assets" / "imgui.ini";

    std::filesystem::copy(engine_premake , premake , 
                          std::filesystem::copy_options::overwrite_existing |
                          std::filesystem::copy_options::recursive);
    std::filesystem::copy_file(engine_make , make ,
                               std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy_file(engine_imguiini , imguiini ,
                               std::filesystem::copy_options::overwrite_existing);

    return true;
  }

  bool CreateProjectLayer::CreateCoreProjectFiles() {
    OE_INFO("Project name: {}" , project_name.value());
    final_name = project_name.value();

    if (!Filesystem::CreateDir(project_path + "/" + project_name.value())) {
      OE_WARN("Failed to create project director: {}" , project_name.value());
      return false;
    }

    std::string cfg_file = final_name + ".other";
    std::string makefilename = "premake5.lua"; // final_name + ".lua";
    
    Path engine_core = Filesystem::GetEngineCoreDir(); 
    Path launcher_dir = engine_core / "OtherEngine-Launcher";
    Path launcher_assets = launcher_dir / "assets";
    Path make = launcher_assets / "premake5.lua";

    project_dir = std::filesystem::absolute(Path(project_path)) / project_name.value();
    project_file = Path{ project_path } / cfg_file;

    Path default_tlmake = launcher_assets / "top_level_premake5.lua.txt";
    Path default_pmake = launcher_assets / "project_premake5.lua.txt";
    Path proj_tlmake = Path{ project_path } / makefilename;
    Path proj_pmake = Path{ project_path } / final_name / "premake5.lua";

    /// make backslashes into forwards
    std::string fixed_path = project_path;
    std::ranges::replace(fixed_path , '\\' , '/');
    
    Path default_cfg = launcher_assets / "default.other.txt";
    
    Path assets = project_dir / "assets";
    Path src = project_dir / "src";
    
    Path hpp = src / (final_name + ".hpp");
    Path cpp = src / (final_name + ".cpp");

    Path defaulthpp = launcher_assets / "default.hpp.txt";
    Path defaultcpp = launcher_assets / "default.cpp.txt";

    auto make_processor = [&](std::string& contents) {
      std::string enginedir = "@enginedir";
      std::string projectname = "@projectname";
      std::string projectpath = "@projectpath";
      std::string appname = "@appname";

      size_t idx = contents.find(enginedir);
      while (idx < contents.length()) {
        contents.replace(idx , enginedir.length() , engine_core.string());
        idx = contents.find(enginedir);
      }
      
      idx = contents.find(projectname);
      while (idx < contents.length()) {
        contents.replace(idx , projectname.length() , final_name);
        idx = contents.find(projectname);
      }

      idx = contents.find(projectpath);
      while (idx < contents.length()) {
        contents.replace(idx , projectpath.length() , fixed_path);
        idx = contents.find(projectpath);
      }
      
      std::string appname_out = final_name;
      appname_out[0] = toupper(appname_out[0]);
      
      idx = contents.find(appname);
      while (idx < contents.length()) {
        contents.replace(idx , appname.length() , appname_out);
        idx = contents.find(appname);
      }
    };

    CopyWithEdits(default_tlmake , proj_tlmake , make_processor);
    CopyWithEdits(default_pmake , proj_pmake , make_processor); 
    CopyWithEdits(default_cfg , project_file , [&](std::string& contents) {
      std::string projectname = "@projectname";
      std::string projectpath = "@projectpath";
      std::string appname = "@appname";
      
      size_t idx = contents.find(projectname);
      while (idx < contents.length()) {
        contents.replace(idx , projectname.length() , final_name);
        idx = contents.find(projectname);
      }

      idx = contents.find(projectpath);
      while (idx < contents.length()) {
        contents.replace(idx , projectpath.length() , fixed_path);
        idx = contents.find(projectpath);
      }
      
      std::string appname_out = final_name;
      appname_out[0] = toupper(appname_out[0]);
      
      idx = contents.find(appname);
      while (idx < contents.length()) {
        contents.replace(idx , appname.length() , appname_out);
        idx = contents.find(appname);
      }
    });

    render_state = RenderState::CREATING_PROJECT;

    if (!Filesystem::CreateDir(assets)) {
      OE_ERROR("Failed to create 'assets'");
    }
    
    if (!Filesystem::CreateDir(src)) {
      OE_ERROR("Failed to create 'src'");
    }

    
    CopyWithEdits(defaulthpp , hpp , [&](std::string& contents) {
      std::string projectname = "@projectname";
      std::string headerguard = "@headerguard";
      std::string appname = "@appname";
      
      size_t idx = contents.find(projectname);
      while (idx < contents.length()) {
        contents.replace(idx , projectname.length() , final_name);
        idx = contents.find(projectname);
      }

      std::string headerguard_out;
      std::ranges::transform(final_name , std::back_inserter(headerguard_out) , ::toupper);

      idx = contents.find(headerguard);
      while (idx < contents.length()) {
        contents.replace(idx , headerguard.length() , headerguard_out);
        idx = contents.find(headerguard);
      }

      std::string appname_out = final_name;
      appname_out[0] = toupper(appname_out[0]);
      
      idx = contents.find(appname);
      while (idx < contents.length()) {
        contents.replace(idx , appname.length() , appname_out);
        idx = contents.find(appname);
      }
    });
    CopyWithEdits(defaultcpp , cpp , [&](std::string& contents) {
      std::string projectname = "@projectname";
      
      size_t idx = contents.find(projectname);
      while (idx < contents.length()) {
        contents.replace(idx , projectname.length() , final_name);
        idx = contents.find(projectname);
      }
    });
    // 

    if (!Filesystem::CreateDir(assets / "scripts")) {
      return false;
    }
    
    if (!Filesystem::CreateDir(assets / "scenes")) {
      return false;
    }
    
    if (!Filesystem::CreateDir(assets / "editor")) {
      return false;
    }
    
    if (!Filesystem::CreateDir(assets / "materials")) {
      return false;
    }
    
    if (!Filesystem::CreateDir(assets / "shaders")) {
      return false;
    }

    Path premakeexe = std::filesystem::absolute(Path{ project_path } / "premake" / "premake5.exe");
    Path makefile = std::filesystem::absolute(Path{ project_path } / makefilename);

    std::string cmd = fmtstr("\"{}\" vs2022 --file={}" , premakeexe.string() , makefile.string());
    OE_DEBUG("Premake command : {}" , cmd);
    if (system(cmd.c_str()) != 0) {
      OE_ERROR("Failed to build project!");
      return false;
    }

    std::string slnfilename = final_name + ".sln";
    Path slnfile = Path{ project_path } / slnfilename;
    if (!PlatformLayer::BuildProject(slnfile)) {
      OE_ERROR("Failed to build project!");
      return false;
    }

    ProjectCache::WriteProjectToCache("OtherEngine-Launcher/projects.cfg" , final_name , project_file.parent_path());
    
    ParentApp()->PushUIWindow(NewRef<TextEditor>("Project Editor" , "." , project_file.string()));
    Project::QueueNewProject(project_file.string());

    project_name = std::nullopt;
    return true;
  }
      
  bool CreateProjectLayer::CopyFileTo(const Path& from , const Path& to) {
    OE_ASSERT(Filesystem::PathExists(from) , "For some reason copy src path doesn't exist!");

    try {
      std::filesystem::copy_file(from , to);
      return true;
    } catch (const std::filesystem::filesystem_error& e) {
      return false;
    }
  }
      
  bool CreateProjectLayer::CopyWithEdits(const Path& from , const Path& to , std::function<void(std::string&)> modifier) {
    OE_ASSERT(Filesystem::PathExists(from) , "For some reason copy src path doesn't exist!");
    
    std::string contents;
    {
      std::ifstream file(from);
      if (!file.is_open()) {
        project_name = std::nullopt;
        return false;
      }

      std::stringstream ss;
      ss << file.rdbuf();
      contents = ss.str();
    }

    modifier(contents);

    std::ofstream out_file(to);
    if (!out_file.is_open()) {
      OE_WARN("Failed to create project file: {}" , project_name.value());
      project_name = std::nullopt;
      return false;
    }

    out_file << contents;
    return true;
  }

  void CreateProjectLayer::RenderDirContents() {
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
