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

namespace other {
  
  struct ProjectMetadata {
    std::string name;
    std::string path;
    std::string description;
  };
  
  class Project : public RefCounted {
    public:
      Project() {}
      virtual ~Project() override;

      Project(Project&& other);
      Project(const Project& other);
      Project& operator=(Project&& other);
      Project& operator=(const Project& other);

      static ProjectMetadata CreateMetadata(const std::string& path , const ConfigTable& cfg);
      static Ref<Project> Create(ProjectMetadata data);

      static ProjectMetadata GetMetadata() { return metadata; }

      static void QueueNewProject(const std::string& path);
      static bool HasQueuedProject();
      // this should attempt to relaunch the launcher
      static std::string GetQueuedProjectPath();

    private:
      static Opt<std::string> queued_project_path;
      static ProjectMetadata metadata;
  };

} // namespace other

#endif // !OTHER_ENGINE_PROJECT_HPP
