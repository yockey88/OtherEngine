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
    if (proj_path.hash != 0) {
      metadata.file_path = Path(proj_path.args[0]);
      OE_DEBUG("Project Path : {}" , metadata.file_path);
    }

    metadata.project_dir_folders = Filesystem::GetSubPaths(metadata.file_path.parent_path());

    Increment();
  }

  Project::~Project() {
    Decrement();
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
