/**
 * \file launcher_layer.cpp
 **/
#include "launcher_layer.hpp"

#include <filesystem>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/filesystem.hpp"

#include "event/event_queue.hpp"
#include "event/event.hpp"
#include "event/core_events.hpp"
#include "event/app_events.hpp"
#include "event/event_handler.hpp"

#include "rendering/ui/file_explorer.hpp"
#include "rendering/ui/ui_helpers.hpp"

#include "application/app.hpp"

#include "project/project.hpp"

#include "project_cache.hpp"
#include "create_project.hpp"

namespace other {

  constexpr static std::string_view kMainMenuBarName = "CoreLayerMainMenuBar";
  constexpr static uint64_t kMainMenuBarID = FNV("CoreLayerMainMenuBar"); 

  constexpr static std::string_view kFileExplorerName = "CoreLayerFileExplorer";
  constexpr static uint64_t kFileExplorerID = FNV("CoreLayerFileExplorer");

  void LauncherLayer::OnAttach() {
    ProjectCache::ReadCache("OtherEngine-Launcher/projects.cfg");
    render_state = RenderState::MAIN;
  }

  void LauncherLayer::OnUpdate(float dt) {
    if (selection_context.selected_path.has_value()) {
      auto path = selection_context.selected_path.value();
      if (!Filesystem::PathExists(path)) {
        OE_WARN("Selected path does not exist: {}", path);
        selection_context.selected_path = std::nullopt;
        return;
      }

      if (!Filesystem::IsDirectory(path)) {
        OE_WARN("Selected path is not a directory: {}", path);
        selection_context.selected_path = std::nullopt;
        return;
      }

      if (path.empty()) {
        selection_context.selected_path = std::nullopt;
        return;
      }

      switch (selection_context.context_type) {
        case CREATE_PROJECT: 
          CreateProject();
          break;
        case OPEN_PROJECT:
          OpenProject();
          break;
        default: 
          break;
      }

      /// we should have handled anything we needed to handle before this point
      selection_context.selected_path = std::nullopt;
    }
  }

  void LauncherLayer::OnRender() {
  }

  void LauncherLayer::OnUIRender() {
    auto fe_option = [this](const std::string& option , FileExplorerType type , SelectionContextType context_type , 
                            std::optional<RenderState> transition_to = std::nullopt) {
      if (ImGui::MenuItem(option.c_str())) { 
        FileExplorer file_explorer(kFileExplorerName.data() , "." , type);
        file_explorer.OnAttach();

        selection_context.selected_path = file_explorer.GetPath();
        selection_context.context_type = context_type;

        if (transition_to.has_value()) {
          render_state = *transition_to;
        }
      }
    };

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        fe_option("Open Project" , FileExplorerType::OPEN_FOLDER , SelectionContextType::OPEN_PROJECT);
        fe_option("Create Project" , FileExplorerType::SAVE_FILE , SelectionContextType::CREATE_PROJECT , RenderState::CREATING_PROJECT);

        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    switch (render_state) {
      case RenderState::MAIN:
        RenderMain();
        break;
      case RenderState::OPENING_PROJECT:
        RenderOpeningProject();
        break;
      case RenderState::CREATING_PROJECT:
         RenderCreateProject();
         break;
      default:
        break;
    }
  }

  void LauncherLayer::OnEvent(Event* event) {
  }

  void LauncherLayer::OnDetach() {
  }
      
  void LauncherLayer::CreateProject() {
    OE_ASSERT(selection_context.selected_path.has_value() , "Bad selection context LauncherLayer::CreateProject()");
    auto path = selection_context.selected_path.value();

    OE_DEBUG("Creating Project");

    {
      Ref<Layer> create_proj = NewRef<CreateProjectLayer>(ParentApp() , path);
      ParentApp()->PushLayer(create_proj);

      render_state = RenderState::CREATING_PROJECT;
    }
  }

  void LauncherLayer::OpenProject() {
    OE_ASSERT(selection_context.selected_path.has_value() , "Bad selection context LauncherLayer::OpenProject()");
    auto path = selection_context.selected_path.value();

    Opt<std::string> project_file = std::nullopt;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
      if (entry.is_regular_file() && entry.path().extension() == ".other") {
        project_file = entry.path().string();
        break;
      }
    }

    if (!project_file.has_value()) {
      OE_WARN("No project file found in directory: {}", path);
      selection_context.selected_path = std::nullopt;
      return;
    }

    editor_id = ParentApp()->PushUIWindow(NewRef<TextEditor>("Project Editor" , "." , project_file.value()));
    render_state = RenderState::OPENING_PROJECT;

    Project::QueueNewProject(project_file.value());
  }
      
  void LauncherLayer::RenderMain() {
    if (render_state != RenderState::MAIN) {
      OE_WARN("Bad render state LauncherLayer::RenderMain()");
      return;
    }

    OE_ASSERT(render_state == RenderState::MAIN , "Bad render state LauncherLayer::RenderMain()");

    bool showing_projects = ImGui::Begin("Projects" , nullptr);
    if (!showing_projects) {
      ImGui::End();
      return;
    }

    auto project_names = ProjectCache::GetProjectNames();
    auto project_paths = ProjectCache::GetProjectPaths();

    std::vector<std::string> col_headers = { 
      "Project Name" , "Project Directory" 
    };
    auto avail_region = ImGui::GetContentRegionAvail();
    uint32_t column_count = col_headers.size();

    ui::Table("Project-List" , col_headers , ImVec2(avail_region.x , 350) , column_count , [&]() {
      for (uint32_t i = 0; i < project_names.size(); ++i) {
        ImGui::TableSetColumnIndex(0);
        ImColor bg_color = (i % 2 == 0) ? 
          ImColor(ui::theme::background) : 
          ImColor(ui::theme::ColorWithMultiplier(ImColor(ui::theme::background) , 1.2f));

        {
          ScopedColor row_color(ImGuiCol_TableRowBg , bg_color.Value);

          if (ImGui::Selectable(project_names[i].c_str())) {
            selection_context.selected_path = project_paths[i];
            selection_context.context_type = SelectionContextType::OPEN_PROJECT;
          }
        }

        ImGui::TableSetColumnIndex(1);
        ScopedColor text_color(ImGuiCol_Text , ui::theme::text_dark);
        ImGui::Text("%s" , project_paths[i].c_str());


        ImGui::TableNextRow();
      }

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("Project-Table-Menu");
      }

      if (ImGui::BeginPopup("Project-Table-Menu")) {
        if (ImGui::Button("Create Project")) {
          FileExplorer file_explorer(kFileExplorerName.data() , "." , FileExplorerType::OPEN_FOLDER);
          file_explorer.OnAttach();

          selection_context.selected_path = file_explorer.GetPath();
          selection_context.context_type = SelectionContextType::CREATE_PROJECT;
          render_state = RenderState::CREATING_PROJECT;

          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    });

    ImGui::End();
  }

  void LauncherLayer::RenderOpeningProject() {
    OE_ASSERT(render_state == RenderState::OPENING_PROJECT , "Bad render state LauncherLayer::RenderOpeningProject()");

    bool rendering = ImGui::Begin("Project" , &rendering_open_project);
    if (!rendering_open_project) {
      Project::ClearQueuedProject();
      ParentApp()->RemoveUIWindow(editor_id);
      editor_id = 0;
      render_state = RenderState::MAIN;
      
      // have to set this to true or IMGUI wont render the window next time we try to open it
      rendering_open_project = true;
    }

    if (!rendering || !Project::HasQueuedProject()) {
      ImGui::End();
      return;
    }

    if (ImGui::Button("Open Project")) {
      std::filesystem::path find_exe = Filesystem::FindExecutableIn(Project::GetQueuedProjectPath());
      /// can clear it now that we have used the path 
      Project::ClearQueuedProject();

      if (find_exe.empty()) {
        OE_ERROR("Failed to find executable in project directory");
        ImGui::End();
        return;
      }

      Launch(find_exe , LaunchType::EDITOR);
    } else if (ImGui::Button("Launch Project")) {
      std::filesystem::path find_exe = Filesystem::FindExecutableIn(Project::GetQueuedProjectPath());
      /// can clear it now that we have used the path 
      Project::ClearQueuedProject();

      if (find_exe.empty()) {
        OE_ERROR("Failed to find executable in project directory");
        ImGui::End();
        return;
      }

      Launch(find_exe , LaunchType::RUNTIME);
    }

    ImGui::End();
  }

  void LauncherLayer::RenderCreateProject() {
    OE_ASSERT(render_state == RenderState::CREATING_PROJECT , "Bad render state LauncherLayer::RenderCreateProject()"); 

    bool rendering = ImGui::Begin("Project Settings" , &rendering_create_project);
    if (!rendering_create_project) {
      Project::ClearQueuedProject();
      ParentApp()->RemoveUIWindow(create_proj_id);
      editor_id = 0;
      render_state = RenderState::MAIN;
      
      // have to set this to true or IMGUI wont render the window next time we try to open it
      rendering_create_project = true;
    }

    if (!rendering) {
      ImGui::End();
      return;
    }

    ImGui::Text("Project Settings"); 

    ImGui::End();
  }

  void LauncherLayer::Launch(const std::filesystem::path& path , LaunchType type) {
    OE_DEBUG("Launching project: {}", path);
    if (PlatformLayer::LaunchProject(path , type)) {
      EventQueue::PushEvent<ShutdownEvent>(ExitCode::SUCCESS);
    } else {
      OE_ERROR("Failed to launch project");
      ImGui::End();
      return;
    }
  }

} // namespace other
