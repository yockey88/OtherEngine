/**
 * \file project/project.cpp
 **/
#include "project/project.hpp"

#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "core/config_keys.hpp"
#include <filesystem>

namespace other {
namespace {
  
  static Ref<Directory> GetDirStructure(Ref<Directory>& parent , const Path& path) {
    Ref<Directory> this_dir = Ref<Directory>::Create(parent , path);

    std::vector<Path> sub_dirs = Filesystem::GetSubDirs(path);
    std::vector<Ref<Directory>> dirs;
    for (auto& p : sub_dirs) {
      dirs.push_back(GetDirStructure(this_dir , p)); 
    }

    return this_dir;
  }

  static Ref<Directory> GetDirStructure(const Path& path) {
    Ref<Directory> this_dir = Ref<Directory>::Create(path);

    std::vector<Path> sub_dirs = Filesystem::GetSubDirs(path);
    std::vector<Ref<Directory>> dirs;
    for (auto& p : sub_dirs) {
      dirs.push_back(GetDirStructure(this_dir , p)); 
    }

    return this_dir;
  }

  static std::vector<Ref<Directory>> GetAllDirectories(const Path& basepath) {
    std::vector<Path> sub_dirs = Filesystem::GetSubDirs(basepath);
    std::vector<Ref<Directory>> dirs;
    for (auto& p : sub_dirs) {
      dirs.push_back(GetDirStructure(p)); 
    }
    return dirs; 
  }

} // anonymous namespace 

  Opt<std::string> Project::queued_project_path = std::nullopt;
   
  Project::Project(const CmdLine& cmdline , const ConfigTable& config)
      : cmdline(cmdline) , config(config) {
    metadata.name = config.GetVal<std::string>(kProjectSection , kNameValue).value_or("Unnamed Project");
    OE_DEBUG("Project Name : {}" , metadata.name);

    auto proj_path = cmdline.GetArg("--project").value_or(Arg{});
    if (proj_path.hash != 0 && proj_path.args.size() > 0) {
      metadata.file_path = Path(proj_path.args[0]);
      OE_DEBUG("Project Path : {}" , metadata.file_path);
    }

    auto project_dir = cmdline.GetArg("--cwd").value_or(Arg{});
    if (project_dir.hash != 0 && project_dir.args.size() > 0) {
      metadata.project_directory = Path(project_dir.args[0]) / metadata.name;
      OE_DEBUG("Project Directory : {}" , metadata.project_directory);
    }

    auto paths = Filesystem::GetSubPaths(metadata.project_directory);
    for (auto& p : paths) {
      if (p.extension() == ".csproj" && p.string().find("Editor") != std::string::npos) {
        metadata.cs_editor_project_file = p;
      } else if (p.extension() == ".csproj") {
        metadata.cs_project_file = p; 
      }
    }
  }

  Ref<Project> Project::Create(const CmdLine& cmdline , const ConfigTable& data) {
    return NewRef<Project>(cmdline , data);
  }
      
  void Project::LoadFiles() {
    auto dirs = GetAllDirectories(metadata.project_directory);
    for (auto& d : dirs) {
      auto stem = d->path.stem();

      /// ignore dot dirs and build dirs 
      if (stem.string()[0] == '.' || stem == "bin" || stem == "obj") {
        continue;
      }

      /// check if this is our assets dir
      if (stem == "assets") {
        metadata.assets_dir = d->path;
      }

      metadata.directories[FNV(stem.string())] = d;
    }

    Path editor_path = metadata.assets_dir / kEditorDirName;
    Path scripts_path = metadata.assets_dir / kScriptsDirName;

    OE_DEBUG("Creating Directory Watchers for Reloading");

    metadata.cs_editor_watcher = NewScope<DirectoryWatcher>(editor_path.string() , ".cs");
    metadata.cs_scripts_watcher = NewScope<DirectoryWatcher>(scripts_path.string() , ".cs");
    
    metadata.lua_editor_watcher = NewScope<DirectoryWatcher>(editor_path.string() , ".lua");
    metadata.lua_scripts_watcher = NewScope<DirectoryWatcher>(scripts_path.string() , ".lua");

    OE_DEBUG("Creating Script Watches");
    CreateScriptWatchers();
  }
 
  bool Project::RegenProjectFile() {
    CreateScriptWatchers();

    Path premake = metadata.file_path.parent_path() / "premake" / "premake5.exe";
    Path project_sln = metadata.file_path.parent_path() / "premake5.lua";

    /// TODO: replace vs2022 with platform specific generator 
    std::string cmd = fmtstr("{} vs2022 --file={}" , premake.string() , project_sln.string());
    return system(cmd.c_str()) == 0;
  }
  
  void Project::CreateScriptWatchers() {
    if (metadata.filewatchers.size() > 0) {
      metadata.filewatchers.clear();
    }

    Path editor_path = metadata.assets_dir / kEditorDirName;
    Path scripts_path = metadata.assets_dir / kScriptsDirName;

    CreateFileWatchers(editor_path);
    CreateFileWatchers(scripts_path);
  }

  bool Project::EditorDirectoryChanged() {
    OE_ASSERT(metadata.cs_editor_watcher != nullptr  && metadata.lua_editor_watcher != nullptr, "Project editor watchdog nullptr!");
    return metadata.cs_editor_watcher->DirectoryChanged() || 
           metadata.lua_editor_watcher->DirectoryChanged();
  }

  bool Project::ScriptDirectoryChanged() {
    OE_ASSERT(metadata.cs_scripts_watcher != nullptr  && metadata.lua_scripts_watcher != nullptr, "Project scripts watchdog nullptr!");
    return metadata.cs_scripts_watcher->DirectoryChanged() || 
           metadata.lua_scripts_watcher->DirectoryChanged();
  }

  bool Project::AnyScriptChanged() {
    for (const auto& fw : metadata.filewatchers) {
      if (fw->ChangedSinceLastCheck()) {
        return true;
      }
    }

    return false;
  }

  void Project::CreateFileWatchers(const Path& dirpath) {
    if (!Filesystem::PathExists(dirpath)) {
      return;
    } else if (!Filesystem::IsDirectory(dirpath)) {
      return;
    }

    for (auto entry : std::filesystem::recursive_directory_iterator(dirpath)) {
      if (entry.is_regular_file() && (entry.path().extension() == ".cs" || entry.path().extension() == ".lua")) {
        metadata.filewatchers.push_back(NewScope<FileWatcher>(entry.path().string()));
      }
    }
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
