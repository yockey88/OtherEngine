/**
 * \file project/project.cpp
 **/
#include "project/project.hpp"

#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "core/config_keys.hpp"

namespace other {

  Opt<std::string> Project::queued_project_path = std::nullopt;
   
  Project::Project(const CmdLine& cmdline , const ConfigTable& config)
      : cmdline(cmdline) , config(config) {
    metadata.name = config.GetVal<std::string>(kProjectSection , kNameValue).value_or("Unnamed Project");
    auto proj_path = cmdline.GetArg("--project").value_or(Arg{});
    if (proj_path.hash != 0 && proj_path.args.size() > 0) {
      metadata.file_path = Path(proj_path.args[0]);
      OE_DEBUG("Project Path : {}" , metadata.file_path);
    }

    auto project_dir = cmdline.GetArg("--cwd").value_or(Arg{});
    if (project_dir.hash != 0 && project_dir.args.size() > 0) {
      metadata.project_directory = Path(project_dir.args[0]);
      OE_DEBUG("Project Directory : {}" , metadata.project_directory);
    }

    metadata.project_dir_folders = Filesystem::GetSubDirs(metadata.file_path.parent_path());
    OE_DEBUG("Retrieved {} project folders" , metadata.project_dir_folders.size());

    for (auto& dir : metadata.project_dir_folders) {
      if (dir.string().find("assets") != std::string::npos) {
        metadata.assets_dir = dir;
      }
    }
  }

  Ref<Project> Project::Create(const CmdLine& cmdline , const ConfigTable& data) {
    return NewRef<Project>(cmdline , data);
  }

  void Project::QueueNewProject(const std::string& path) {
    queued_project_path = path;
  }

  bool Project::HasQueuedProject() { 
    return queued_project_path.has_value(); 
  }

  // this should attempt to relaunch the launcher
  std::string Project::GetQueuedProjectPath() { 
    return queued_project_path.value_or("OtherEngine-Launcher/launcher.other"); 
  }

  void Project::ClearQueuedProject() {
    queued_project_path = std::nullopt;
  }

} // namespace other
