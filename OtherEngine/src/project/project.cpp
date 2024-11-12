/**
 * \file project/project.cpp
 **/
#include "project/project.hpp"

#include <filesystem>

#include "core/config_keys.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

namespace other {

  Opt<std::string> Project::queued_project_path = std::nullopt;

  Project::Project(const CmdLine& cmdline, const ConfigTable& config)
      : cmdline(cmdline), config(config) {
    metadata.main_project_file = config.GetPath();
    metadata.name = config.GetVal<std::string>(kProjectSection, kNameValue, false).value_or("Unnamed Project");
    metadata.bin_dir = config.GetVal<std::string>(kProjectSection, kBinDirValue, false).value_or("bin/Debug/");
    metadata.primary_scene = config.GetVal<std::string>(kProjectSection, kPrimarySceneValue, false);
    metadata.lua_directory = config.GetVal<std::string>(kProjectSection, kLuaDirValue, false);

    OE_DEBUG("Project Name : {}", metadata.name);
    OE_DEBUG("Bin Dir : {}", metadata.bin_dir);

    auto script_bin = config.GetVal<std::string>(kProjectSection, kScriptBinDirValue, false);
    if (script_bin.has_value()) {
      metadata.script_bin_dir = *script_bin;
    }

    auto proj_path = cmdline.GetArg("--project").value_or(Arg{});
    if (proj_path.hash != 0 && proj_path.args.size() > 0) {
      metadata.file_path = Path(proj_path.args[0]);
      OE_DEBUG("Project Path : {}", metadata.file_path);
    }

    auto project_dir = cmdline.GetArg("--cwd").value_or(Arg{});
    if (project_dir.hash != 0 && project_dir.args.size() > 0) {
      metadata.project_directory = Path(project_dir.args[0]);
    } else {
      metadata.project_directory = Filesystem::GetWorkingDirectory();
    }
    OE_DEBUG("Project Directory : {}", metadata.project_directory);

    auto assets_dir = config.GetVal<std::string>(kProjectSection, kAssetsDirValue, false);
    if (assets_dir.has_value()) {
      metadata.assets_dir = metadata.project_directory / *assets_dir;
    } else {
      metadata.assets_dir = metadata.project_directory / "assets";
    }
    OE_DEBUG("Assets Directory : {}", metadata.assets_dir);

    auto paths = Filesystem::GetSubPaths(metadata.project_directory);
    for (auto& p : paths) {
      if (p.extension() == ".csproj" && p.string().find("Editor") != std::string::npos) {
        metadata.cs_editor_project_file = p;
      } else if (p.extension() == ".csproj") {
        metadata.cs_project_file = p;
      }
    }

    InitializeVirtualFolders();
  }

  Ref<Project> Project::Create(const CmdLine& cmdline, const ConfigTable& data) {
    return NewRef<Project>(cmdline, data);
  }

  bool Project::RegenProjectFile() {
    // CreateScriptWatchers();

    std::string makefilename = "premake5.lua";  // metadata.name + ".lua";
    Path premake = metadata.file_path.parent_path() / "premake" / "premake5.exe";
    Path project = metadata.file_path.parent_path() / makefilename;

    std::string premakestr = premake.string();
    std::string projectstr = project.string();

    std::replace(premakestr.begin(), premakestr.end(), '/', '\\');
    std::replace(projectstr.begin(), projectstr.end(), '/', '\\');

    /// TODO: replace vs2022 with platform specific generator
    std::string cmd = fmtstr("{} vs2022 --file={}", premakestr, projectstr);
    return system(cmd.c_str()) == 0;
  }

  ProjectMetadata& Project::GetMetadata() {
    return metadata;
  }

  std::string Project::GetName() {
    return metadata.name;
  }

  Path Project::GetFilePath() {
    return metadata.file_path;
  }

  void Project::InitializeVirtualFolders() {
    Ref<Directory> bin_dir = Filesystem::MountDirectory("bin", metadata.project_directory / metadata.bin_dir);
    OE_ASSERT(bin_dir != nullptr, "Failed to mount bin directory : {}", metadata.project_directory / metadata.bin_dir);
    OE_ASSERT(bin_dir->Exists(), "Bin directory does not exist : {}", metadata.project_directory / metadata.bin_dir);

    Ref<Directory> assets_dir = Filesystem::MountDirectory("assets", metadata.assets_dir);
    OE_ASSERT(assets_dir != nullptr, "Failed to mount assets directory : {}", metadata.assets_dir);
    OE_ASSERT(assets_dir->Exists(), "Assets directory does not exist : {}", metadata.assets_dir);

    if (metadata.script_bin_dir.has_value()) {
      Ref<Directory> script_bin_dir = Filesystem::MountDirectory("script-bin", *bin_dir / *metadata.script_bin_dir);
      OE_ASSERT(script_bin_dir != nullptr, "Failed to mount script bin directory : {}", *bin_dir / *metadata.script_bin_dir);
      OE_ASSERT(script_bin_dir->Exists(), "Script bin directory does not exist : {}", *bin_dir / *metadata.script_bin_dir);
    } else {
      Ref<Directory> script_bin_dir = Filesystem::MountDirectory("script-bin", *bin_dir);
      OE_ASSERT(script_bin_dir != nullptr, "Failed to mount script bin directory : {}", Path(*bin_dir));
      OE_ASSERT(script_bin_dir->Exists(), "Script bin directory does not exist : {}", Path(*bin_dir));
    }

    if (metadata.lua_directory.has_value()) {
      Ref<Directory> lua_dir = Filesystem::MountDirectory("lua", *metadata.lua_directory);
      OE_ASSERT(lua_dir != nullptr, "Failed to mount lua directory : {}", Path(*metadata.lua_directory));
      OE_ASSERT(lua_dir->Exists(), "Lua directory does not exist : {}", Path(*metadata.lua_directory));
    }

    Ref<Directory> project_dir = Filesystem::MountDirectory("project-root", metadata.project_directory);
    OE_ASSERT(project_dir != nullptr, "Failed to mount project directory : {}", metadata.project_directory);
    OE_ASSERT(project_dir->Exists(), "Project directory does not exist : {}", metadata.project_directory);

    const Path editor_dir = metadata.project_directory / kEditorDirName;
    Ref<Directory> editor_dir_handle = Filesystem::MountDirectory("editor", editor_dir);
    OE_ASSERT(editor_dir_handle != nullptr, "Failed to mount editor directory: {}", editor_dir);
    OE_ASSERT(editor_dir_handle->Exists(), "Editor directory does not exist : {}", editor_dir);

    const Path materials_dir = metadata.project_directory / kMaterialsDirName;
    Ref<Directory> materials_dir_handle = Filesystem::MountDirectory("materials", materials_dir);
    OE_ASSERT(materials_dir_handle != nullptr, "Failed to mount materials directory : {}", materials_dir);
    OE_ASSERT(materials_dir_handle->Exists(), "Materials directory does not exist : {}", materials_dir);

    const Path scenes_dir = metadata.project_directory / kScenesDirName;
    Ref<Directory> scenes_dir_handle = Filesystem::MountDirectory("scenes", scenes_dir);
    OE_ASSERT(scenes_dir_handle != nullptr, "Failed to mount scenes directory: {}", scenes_dir);
    OE_ASSERT(scenes_dir_handle->Exists(), "Scenes directory does not exist : {}", scenes_dir);

    const Path scripts_dir = metadata.project_directory / kScriptsDirName;
    Ref<Directory> scripts_dir_handle = Filesystem::MountDirectory("scripts", scripts_dir);
    OE_ASSERT(scripts_dir_handle != nullptr, "Failed to mount scripts directory : {}", scripts_dir);
    OE_ASSERT(scripts_dir_handle->Exists(), "Scripts directory does not exist : {}", scripts_dir);

    const Path shaders_dir = metadata.project_directory / kShadersDirName;
    Ref<Directory> shaders_dir_handle = Filesystem::MountDirectory("shaders", shaders_dir);
    OE_ASSERT(shaders_dir_handle != nullptr, "Failed to mount shaders directory : {}", shaders_dir);
    OE_ASSERT(shaders_dir_handle->Exists(), "Shaders directory does not exist : {}", shaders_dir);
  }

  void Project::QueueNewProject(const Path& path) {
    queued_project_path = path.string();
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

}  // namespace other
