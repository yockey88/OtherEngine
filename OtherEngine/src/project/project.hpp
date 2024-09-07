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
#include "core/directory.hpp"
#include "core/directory_watcher.hpp"
#include "core/file_watcher.hpp"

#include "parsing/cmd_line_parser.hpp"

namespace other {

  constexpr std::string_view kAssetsDirName = "assets";
  constexpr UUID kAssetsDirNameHash = FNV(kAssetsDirName);

  constexpr std::string_view kEditorDirName = "editor";
  constexpr UUID kEditorDirNameHash = FNV(kEditorDirName);

  constexpr std::string_view kMaterialsDirName = "materials";
  constexpr UUID kMaterialsDirNameHash = FNV(kMaterialsDirName);
  
  constexpr std::string_view kScenesDirName = "scenes";
  constexpr UUID kScenesDirNameHash = FNV(kScenesDirName);

  constexpr std::string_view kScriptsDirName = "scripts";
  constexpr UUID kScriptsDirNameHash = FNV(kScriptsDirName);

  constexpr std::string_view kShadersDirName = "shaders";
  constexpr UUID kShadersDirNameHash = FNV(kShadersDirName);

  enum ProjectDirectoryType {
    ASSETS_DIR = 0 ,
    EDITOR_DIR ,
    MATERIALS_DIR ,
    SCENES_DIR ,
    SCRIPT_DIR , 
    SHADERS_DIR ,

    NUM_PROJECT_DIRS , 
    INVALID_PROJECT_DIR = NUM_PROJECT_DIRS ,
  };

  using ProjectDirectoryPair = std::pair<UUID , std::string_view>;
  constexpr std::array<ProjectDirectoryPair , NUM_PROJECT_DIRS> kProjectTags = {
    ProjectDirectoryPair{ kAssetsDirNameHash , kAssetsDirName } ,  
    ProjectDirectoryPair{ kEditorDirNameHash , kEditorDirName } ,  
    ProjectDirectoryPair{ kMaterialsDirNameHash , kMaterialsDirName } ,  
    ProjectDirectoryPair{ kScenesDirNameHash , kScenesDirName } ,  
    ProjectDirectoryPair{ kShadersDirNameHash , kShadersDirName } ,  
  };
  
  struct ProjectMetadata {
    std::string name = "";
    Path project_directory = "";
    Path file_path = "";
    Path bin_dir = "";
    Path assets_dir = "";

    Opt<Path> script_bin_dir = std::nullopt;

    Opt<std::string> primary_scene = std::nullopt;

    Path cs_project_file = "";
    Path cs_editor_project_file = "";

    std::map<UUID , Ref<Directory>> directories{};

    std::vector<Scope<FileWatcher>> filewatchers;

    Scope<DirectoryWatcher> cs_editor_watcher = nullptr;
    Scope<DirectoryWatcher> lua_editor_watcher = nullptr;

    Scope<DirectoryWatcher> cs_scripts_watcher = nullptr;
    Scope<DirectoryWatcher> lua_scripts_watcher = nullptr;
  };
  
  class Project : public RefCounted {
    public:
      Project(const CmdLine& cmdline , const ConfigTable& config);
      virtual ~Project() override {}

      static Ref<Project> Create(const CmdLine& cmdline , const ConfigTable& data);

      void LoadFiles();

      bool RegenProjectFile();
      void CreateScriptWatchers();

      const CmdLine& cmdline;
      const ConfigTable& config;

      ProjectMetadata& GetMetadata() { return metadata; }
      bool EditorDirectoryChanged();
      bool ScriptDirectoryChanged();
      bool AnyScriptChanged();

      std::string GetName() { return metadata.name; }
      Path GetFilePath() { return metadata.file_path; }
      const std::map<UUID , Ref<Directory>>& GetProjectDirectories() const {
        return metadata.directories;
      }

    private:
      ProjectMetadata metadata;
      
      void CreateFileWatchers(const Path& dirpath);

    public:
      static void QueueNewProject(const std::string& path);
      static bool HasQueuedProject();
      // this should attempt to relaunch the launcher
      static std::string GetQueuedProjectPath();
      static void ClearQueuedProject();

    private:
      static Opt<std::string> queued_project_path;

  };

} // namespace other

#endif // !OTHER_ENGINE_PROJECT_HPP
