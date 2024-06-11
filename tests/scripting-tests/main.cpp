/**
 * \file tests/scripting-tests/main.cpp
 **/
#include <filesystem>

#include <ShlObj.h>
#include <minwinbase.h>
#include <processthreadsapi.h>

#include <spdlog/fmt/fmt.h>

#include "core/defines.hpp"

using Path = std::filesystem::path;

int main(int argc , char* argv[]) {
    TCHAR program_files_path_buffer[MAX_PATH];
    SHGetSpecialFolderPath(0 , program_files_path_buffer , CSIDL_PROGRAM_FILES , FALSE);

    Path ms_build = std::filesystem::path(program_files_path_buffer) /
      "Microsoft Visual Studio" / "2022" / "Community" / 
      "MSBuild" / "Current" / "Bin" / "MSBuild.exe";

    std::string project_file_str = "C:/Yock/gamedev/Testbed/CsScript1.csproj";
    std::replace(project_file_str.begin() , project_file_str.end() , '/' , '\\');

    /// replace configuration in the future
    std::string cmd = fmt::format(fmt::runtime("{}") , ms_build.string());

    /// we can't use LaunchProcess because we want to wait on the process to finish

    std::replace(cmd.begin() , cmd.end() , '/' , '\\');
    std::string args = fmt::format(fmt::runtime("\"\"{}\" {}\" /property:Configuration=Debug") , cmd , project_file_str);

    std::wstring wcmd(cmd.begin() , cmd.end());
    wchar_t* wcmdline = const_cast<wchar_t*>(wcmd.c_str());

    other::println("Launching process {}" , args);    

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&si , sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi , sizeof(pi));

    std::system(args.c_str());

    // bool result = CreateProcessW(nullptr , wcmdline , nullptr , nullptr , 
    //                              true , 0 , nullptr , nullptr , &si , &pi);  

    // if (!result) {
    //   other::println("Failed to launch build using command line {}" , cmd);
    //   CloseHandle(pi.hProcess);
    //   CloseHandle(pi.hThread);
    //   return 1;
    // }

    // WaitForSingleObject(pi.hProcess , INFINITE);
    // CloseHandle(pi.hProcess);
    // CloseHandle(pi.hThread);

  return 0;
}
