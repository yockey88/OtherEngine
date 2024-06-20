/**
 * \file project/project.hpp
 **/
#ifndef OTHER_ENGINE_PROJECT_HPP
#define OTHER_ENGINE_PROJECT_HPP

#include <string>

#include "core/defines.hpp"
#include "core/ref_counted.hpp"
#include "core/ref.hpp"
#include "core/config.hpp"
#include "parsing/cmd_line_parser.hpp"

namespace other {
  
  struct ProjectMetadata {
    std::string name = "";
    Path project_directory = "";
    Path file_path = "";
    Path assets_dir = "";

    std::vector<Path> project_dir_folders{};
  };
  
  class Project : public RefCounted {
    public:
      Project(const CmdLine& cmdline , const ConfigTable& config);
      virtual ~Project() override {}

      static Ref<Project> Create(const CmdLine& cmdline , const ConfigTable& data);

      ProjectMetadata GetMetadata() { return metadata; }

      std::string GetName() { return metadata.name; }
      Path GetFilePath() { return metadata.file_path; }
      std::vector<Path> GetProjectDirPaths() { return metadata.project_dir_folders; }

      static void QueueNewProject(const std::string& path);
      static bool HasQueuedProject();
      // this should attempt to relaunch the launcher
      static std::string GetQueuedProjectPath();
      static void ClearQueuedProject();

    private:
      const CmdLine& cmdline;
      const ConfigTable& config;
      ProjectMetadata metadata;

      static Opt<std::string> queued_project_path;
  };

} // namespace other

#endif // !OTHER_ENGINE_PROJECT_HPP
