/**
 * \file project_cache.hpp
 */
#ifndef OTHER_ENGINE_PROJECT_CACHE_HPP
#define OTHER_ENGINE_PROJECT_CACHE_HPP

#include <string>
#include <vector>

#include "core/config.hpp"

namespace other {

  class ProjectCache {
    public:
      static void ReadCache(const std::string& path);

      static std::vector<std::string> GetProjectNames();
      static std::vector<std::string> GetProjectPaths();

      static std::string GetProjectPath(const std::string& name);

    private:
      static ConfigTable projects;
  };

} // namespace other

#endif // !OTHER_ENGINE_PROJECT_CACHE_HPP
