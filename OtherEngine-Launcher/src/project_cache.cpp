/**
 * \file project_cache.cpp
 **/
#include "project_cache.hpp"

#include <fstream>

#include "core/logger.hpp"

#include "parsing/ini_parser.hpp"

namespace other {

  ConfigTable ProjectCache::projects;

  void ProjectCache::ReadCache(const std::string& path) {
    try {
      IniFileParser parser(path);
      projects = parser.Parse();
    } catch (const std::exception& e) {
      println("Failed to read project cache: {}", e.what());
    } 
  }

  std::vector<std::string> ProjectCache::GetProjectNames() {
    return projects.GetKeys("PROJECTS");
  }

  std::vector<std::string> ProjectCache::GetProjectPaths() {
    std::vector<std::string> paths;
    for (const auto& name : GetProjectNames()) {
      auto uc_name = name;
      std::transform(uc_name.begin(), uc_name.end(), uc_name.begin(), ::toupper);
      paths.push_back(projects.Get("PROJECTS", uc_name)[0]);
    }
    return paths;
  }
      
  void ProjectCache::WriteProjectToCache(const Path& cache_path , const std::string& name , const Path& path) {
    std::string contents;
    {
      std::ifstream infile(cache_path);
      if (!infile.is_open()) {
        OE_ERROR("Failed to open project cache : {}" , cache_path);
        return;
      }

      std::stringstream ss;
      ss << infile.rdbuf();

      contents = ss.str();
    }
      
    contents += "\n" + name + " = \"" + path.string() + "\"";

    std::ofstream outfile(cache_path);
    if (!outfile.is_open()) {
      OE_ERROR("Failed to open project cache : {}" , cache_path);
      return;
    }

    outfile << contents;
  }

} // namespace other
