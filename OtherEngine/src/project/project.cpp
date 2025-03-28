/**
 * \file project/project.cpp
 **/
#include "project/project.hpp"

#include <filesystem>

#include "core/config_keys.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "application/app_state.hpp"

namespace other {

  Opt<std::string> Project::queued_project_path = std::nullopt;

  Project::Project(const CmdLine& cmdline, const ConfigTable& config)
      : cmdline(cmdline), config(config) {
    metadata.main_project_file = config.GetPath();
    metadata.name = config.GetVal<std::string>(kProjectSection, kNameValue, false).value_or("Unnamed Project");
    metadata.bin_dir = config.GetVal<std::string>(kProjectSection, kBinDirValue, false).value_or("bin/Debug/");
    metadata.primary_scene = config.GetVal<std::string>(kProjectSection, kPrimarySceneValue, false);

    std::string editor_dir = config.GetVal<std::string>(kEditorSection, kEditorDirValue, false).value_or(std::string{ kEditorDirName });
    std::string materials_dir = config.GetVal<std::string>(kProjectSection, kMaterialsDirValue, false).value_or(std::string{ kMaterialsDirName });
    std::string scenes_dir = config.GetVal<std::string>(kProjectSection, kScenesDirValue, false).value_or(std::string{ kScenesDirName });
    std::string scripts_dir = config.GetVal<std::string>(kProjectSection, kScriptsDirValue, false).value_or(std::string{ kScriptsDirName });
    std::string shaders_dir = config.GetVal<std::string>(kProjectSection, kShadersDirValue, false).value_or(std::string{ kShadersDirName });

    OE_DEBUG("Project Name : {}", metadata.name);

    auto script_bin = config.GetVal<std::string>(kProjectSection, kScriptBinDirValue, false);
    if (script_bin.has_value()) {
      metadata.script_bin_dir = *script_bin;
    }

    auto proj_path = cmdline.GetArg("--project").value_or(Arg{});
    if (proj_path.hash != 0 && proj_path.args.size() > 0) {
      metadata.file_path = Path(proj_path.args[0]);
    }

    auto cs_dir = cmdline.GetArg("--cs").value_or(Arg{});
    if (cs_dir.hash != 0 && cs_dir.args.size() > 0) {
      metadata.cs_dir = Path(cs_dir.args[0]);
    } else if (config.KeyExists(kProjectSection, kScriptsDirValue)) {
      metadata.cs_dir = metadata.project_directory / *config.GetVal<std::string>(kProjectSection, kScriptsDirValue, false);
    } else {
      std::string default_csproj_name = metadata.name + "_scripts";
      std::string cs_dir = config.GetVal<std::string>(kProjectSection, kScriptsDirValue, false).value_or(default_csproj_name);
      metadata.cs_dir = metadata.project_directory / cs_dir;
    }

    try {
      for (auto& entry : std::filesystem::directory_iterator(metadata.cs_dir)) {
        if (entry.path().extension() == ".csproj") {
          metadata.cs_project_file = entry.path();
        }
      }
    } catch (std::filesystem::filesystem_error& e) {
      OE_ERROR("Filesystem error : {}", e.what());
    } catch (...) {
      OE_ERROR("Unknown Filesystem error");
    }
    if (metadata.cs_project_file.empty()) {
      OE_ERROR("Failed to find C# project file in directory : {}", metadata.cs_dir);
    }

    auto project_dir = cmdline.GetArg("--cwd").value_or(Arg{});
    if (project_dir.hash != 0 && project_dir.args.size() > 0) {
      metadata.project_directory = Path(project_dir.args[0]);
    } else {
      metadata.project_directory = Filesystem::GetWorkingDirectory();
    }

    auto assets_dir = config.GetVal<std::string>(kProjectSection, kAssetsDirValue, false);
    if (assets_dir.has_value()) {
      metadata.assets_dir = metadata.project_directory / *assets_dir;
    } else {
      metadata.assets_dir = metadata.project_directory / "assets";
    }

    auto paths = Filesystem::GetSubPaths(metadata.project_directory);
    for (auto& p : paths) {
      if (p.extension() == ".csproj" && p.string().find("Editor") != std::string::npos) {
        metadata.cs_editor_project_file = p;
      } else if (p.extension() == ".csproj") {
        metadata.cs_project_file = p;
      }
    }

    metadata.editor_dir = metadata.project_directory / editor_dir;
    metadata.materials_dir = metadata.project_directory / materials_dir;
    metadata.scenes_dir = metadata.project_directory / scenes_dir;
    metadata.shaders_dir = metadata.project_directory / shaders_dir;

    {
      std::stringstream ss;
      ss << "\nProject Name : " << metadata.name << "\n"
         << " > Main Project Directory : " << metadata.project_directory << "\n"
         << " > Main Project File : " << metadata.main_project_file << "\n"
         << " > C# Directory : " << metadata.cs_dir << "\n"
         << " > C# Project File : " << metadata.cs_project_file << "\n"
         << " > C# Script Bin : " << metadata.script_bin_dir.value_or("None") << "\n"
         << "----------------------------------" << "\n"
         << "  - Bin Dir : " << metadata.bin_dir << "\n"
         << "  - Assets Dir : " << metadata.assets_dir << "\n"
         << "  - Materials Dir : " << metadata.materials_dir << "\n"
         << "  - Scenes Dir : " << metadata.scenes_dir << "\n"
         << "  - Shaders Dir : " << metadata.shaders_dir << "\n"
         << "  - Scripts Dir : " << metadata.cs_dir << "\n";
      OE_DEBUG(ss.str());
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
    {
      Ref<Directory> dir = Filesystem::MountProjectRoot("project-root", metadata.project_directory);
      if (dir == nullptr || !dir->Exists()) {
        OE_ERROR("Failed to mount project directory : {}", metadata.project_directory);
      }
    }
    MountDirectory("scripts", metadata.cs_dir);

    MountDirectory("bin", metadata.project_directory / metadata.bin_dir);
    MountDirectory("assets", metadata.assets_dir);

    Ref<Directory> bin_dir = Filesystem::GetDirectory("bin");
    if (bin_dir != nullptr && metadata.script_bin_dir.has_value()) {
      MountDirectory("script-bin", *bin_dir / *metadata.script_bin_dir);
    } else {
      MountDirectory("script-bin", *bin_dir);
    }

    if (AppState::mode == EngineMode::EDITOR
#ifdef OE_TESTING_ENVIRONMENT
        || AppState::mode == EngineMode::TESTING
#endif  // OE_TESTING_ENVIRONMENT
    ) {
      MountDirectory("editor", metadata.editor_dir);
    }

    MountDirectory("materials", metadata.materials_dir);
    MountDirectory("scenes", metadata.scenes_dir);
    MountDirectory("shaders", metadata.shaders_dir);
  }

  void Project::MountDirectory(const std::string_view name, const Path& path) {
    Ref<Directory> dir = Filesystem::MountDirectory(name, path);
    if (dir == nullptr || !dir->Exists()) {
      OE_ERROR("Failed to mount project directory {} : {}", name, path);
    }
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
