/**
 * \file project/project.cpp
 **/
#include "project/project.hpp"

namespace other {

  ProjectMetadata Project::metadata;
  Opt<std::string> Project::queued_project_path = std::nullopt;

  Project::~Project() {
    Decrement();
  }

  Project::Project(Project&& other) {
  }

  Project::Project(const Project& other) {
    Increment();
  }

  Project& Project::operator=(Project&& other) {
    return *this;
  }

  Project& Project::operator=(const Project& other) {
    Increment();
    return *this;
  }

  ProjectMetadata Project::CreateMetadata(const std::string& path , const ConfigTable& cfg) {
    ProjectMetadata data;
    data.path = path;

    auto name_opt = cfg.GetVal<std::string>("PROJECT" , "NAME");
    if (name_opt.has_value()) {
      data.name = name_opt.value();
    } else {
      data.name = "Unnamed Project";
    }
  
    auto desc_opt = cfg.GetVal<std::string>("PROJECT" , "DESCRIPTION");
    if (desc_opt.has_value()) {
      data.description = desc_opt.value();
    } else {
      data.description = "No description provided";
    }

    return data;
  }
      
  Ref<Project> Project::Create(ProjectMetadata data) {
    metadata = data;
    return NewRef<Project>();
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

} // namespace other
