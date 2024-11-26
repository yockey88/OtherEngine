/**
 * \file project/project.hpp
 **/
#ifndef OTHER_ENGINE_PROJECT_HPP
#define OTHER_ENGINE_PROJECT_HPP

#include <optional>
#include <string>
#include <string_view>

#include "core/config.hpp"
#include "core/defines.hpp"
#include "core/directory.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "parsing/cmd_line_parser.hpp"

namespace other {

  constexpr std::string_view kProjectRoot = "project-root";
  constexpr UUID kProjectRootHash = FNV(kProjectRoot);

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
    ASSETS_DIR = 0,
    EDITOR_DIR,
    MATERIALS_DIR,
    SCENES_DIR,
    SCRIPT_DIR,
    SHADERS_DIR,

    NUM_PROJECT_DIRS,
    INVALID_PROJECT_DIR = NUM_PROJECT_DIRS,
  };

  struct ProjectMetadata {
    std::string name = "";
    Path main_project_file = "";
    Path project_directory = "";
    Path file_path = "";

    Path bin_dir = "";
    Path assets_dir = "";
    Path materials_dir;
    Path scenes_dir;
    Path scripts_dir;
    Path shaders_dir;
    Path editor_dir = "";

    Opt<Path> script_bin_dir = std::nullopt;
    Opt<std::string> primary_scene = std::nullopt;

    Path cs_project_file = "";
    Path cs_editor_project_file = "";
  };

  class Project : public RefCounted {
   public:
    Project(const CmdLine& cmdline, const ConfigTable& config);
    virtual ~Project() override {}

    static Ref<Project> Create(const CmdLine& cmdline, const ConfigTable& data);

    void LoadFiles(Ref<Directory> project_dir_handle);
    bool RegenProjectFile();

    ProjectMetadata& GetMetadata();
    std::string GetName();
    Path GetFilePath();

    const CmdLine& cmdline;
    const ConfigTable& config;

   private:
    ProjectMetadata metadata;

    void InitializeVirtualFolders();
    void MountDirectory(const std::string_view name, const Path& path);

   public:
    static void QueueNewProject(const Path& path);
    static bool HasQueuedProject();
    // this should attempt to relaunch the launcher
    static std::string GetQueuedProjectPath();
    static void ClearQueuedProject();

   private:
    static Opt<std::string> queued_project_path;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PROJECT_HPP
