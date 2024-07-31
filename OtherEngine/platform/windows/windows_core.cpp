/**
 * \file platform/windows/windows_core.cpp
 **/
#include "core/platform.hpp"

#include <ShlObj.h>
#include <minwinbase.h>
#include <processthreadsapi.h>
#include <winbase.h>

#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "windows/windows_plugin_loader.hpp"

namespace other {

  Scope<PluginLoader> PlatformLayer::GetPluginLoader(const std::string_view path) {
    return NewScope<WindowsPluginLoader>(path);
  }

  PlatformType PlatformLayer::CurrentPlatform() {
    return PlatformType::WINDOWS;
  }

  bool PlatformLayer::LaunchProject(const Path& path , LaunchType type) {
    OE_ASSERT(Filesystem::FileExists(path) , "File does not exist : {}", path);

    Path project_file = path;
    Path project_folder = path.parent_path();
    bool found = false;

    do {
      project_file = project_file.parent_path();
      if (project_file.empty()) {
        OE_ERROR("Could not find project file in path : {}", path.string());
        return false;
      }

      for (const auto& entry : std::filesystem::directory_iterator(project_file)) {
        if (entry.path().extension() == ".other") {
          project_file = entry.path();
          project_folder = project_file.parent_path();
          found = true;
          break;
        }
      }
    } while (!found);

    if (!std::filesystem::exists(project_file)) {
      OE_ERROR("Could not find project file in path : {}", path.string());
      return false;
    }

    std::string args_str = "";
    args_str.append("--project ");
    args_str.append(std::filesystem::absolute(project_file).string());
    args_str.append(" ");

    if (type == LaunchType::EDITOR) {
      args_str.append("--editor");
      args_str.append(" ");
    }

    args_str.append("--cwd ");
    args_str.append(std::filesystem::absolute(project_folder).string());
    args_str.append(" ");

    auto real_path = std::filesystem::relative(path);
    OE_DEBUG("Launching project : {} with args : {}", real_path , args_str);
    return LaunchProcess(real_path , project_folder , args_str);
  }

  bool PlatformLayer::LaunchProcess(const Path& path_ , const Path& working_dir , 
                                    const std::string& args) {
    /// assert here because these checks should always have passed before calling 
    OE_ASSERT(Filesystem::FileExists(path_) , "File does not exist : {}", path_);
    OE_ASSERT(Filesystem::IsDirectory(working_dir) , "Working directory does not exist : {}", working_dir);

    OE_DEBUG("ARGS : {}", args);

    std::string path = std::filesystem::absolute(path_).string();
    std::string folder = std::filesystem::absolute(working_dir).string();

    std::replace(path.begin() , path.end() , '\\' , '/');
    std::replace(folder.begin() , folder.end() , '\\' , '/');

    /// windows typically wants widestrings
    std::wstring wpath(path.begin(), path.end());
    std::wstring wfolder(folder.begin(), folder.end());
    std::wstring wargs(path.begin() , path.end());
    wargs.append(L" ");
    wargs.append(args.begin(), args.end());

    /// necessary bc CreateProcessW expects non-const lpCommandLine
    wchar_t* wcmdline = const_cast<wchar_t*>(wargs.c_str());

    OE_DEBUG("Launching process in {} : {} {}", folder , path , args);

    PROCESS_INFORMATION pi;
    STARTUPINFO si; 
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    ZeroMemory(&pi, sizeof(pi));
    bool result = CreateProcessW(
        nullptr , wcmdline , nullptr , nullptr , true , 
        CREATE_NEW_CONSOLE , nullptr , nullptr , &si , &pi);

    if (result) {
      OE_DEBUG("Launched process [{}] in [{}]", path , folder);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    } else {
      OE_ERROR("Failed to launch process [{}] in [{}] ({})", path , folder , GetLastError());
    }

    return result;
  }
      
  bool PlatformLayer::BuildProject(const Path& project_file) {
    if (!Filesystem::FileExists(project_file)) {
      OE_ERROR("Failed to build project file : {}" , project_file);
      return false;
    } 

    TCHAR program_files_path_buffer[MAX_PATH];
    SHGetSpecialFolderPath(0 , program_files_path_buffer , CSIDL_PROGRAM_FILES , FALSE);

    Path ms_build = std::filesystem::path(program_files_path_buffer) /
      "Microsoft Visual Studio" / "2022" / "Community" / 
      "Msbuild" / "Current" / "Bin" / "MSBuild.exe";

    std::string project_file_str = project_file.string();
    std::replace(project_file_str.begin() , project_file_str.end() , '/' , '\\');

    /// replace configuration in the future
    std::string cmd = fmt::format(fmt::runtime("\"\"{}\" \"{}\" /p:Configuration=Debug\"") , ms_build.string(), project_file_str);

    return system(cmd.c_str()) == 0;
  }

} // namespace other
