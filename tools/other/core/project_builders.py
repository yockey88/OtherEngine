import os
import subprocess
import sys

from pathlib import Path

from . import project_settings
from . import utilities

def get_platform_test_paths_for_msbuild():
    if utilities.is_windows():
        return [
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\Enterprise\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\Professional\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\BuildTools\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "MSBuild.exe"
        ];
    elif utilities.is_linux():
        return [
            "/mnt/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe",
            "/mnt/c/Program Files (x86)/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe",
            "/mnt/c/Program Files (x86)/Microsoft Visual Studio/2022/Enterprise/MSBuild/Current/Bin/MSBuild.exe",
            "/mnt/c/Program Files (x86)/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin/MSBuild.exe",
            "MSBuild.exe"
        ];
    else:
        raise EnvironmentError("Non-windows platform detected")

def find_ms_build():
    if not (utilities.is_windows() or utilities.is_linux()):
        raise EnvironmentError("Attempting to find MSBuild on non-windows platform")
    
    if utilities.is_linux():
        return ""
    
    try:
        msbuild = os.environ["MSBUILD"]
        if not os.path.exists(msbuild):
            raise FileNotFoundError("MSBuild not found at [{}]".format(msbuild))
        return msbuild
    except KeyError:
        ... ## no-op
    except FileNotFoundError as e:
        print(" > Failed to find MSBuild at [{}] using environment variable".format(msbuild))
        print("   > Ensure that MSBuild is installed and available in PATH and MSBUILD environment variable is set correct;y")
        print("   > Attempting to find MSBuild in common locations")
    
    test_paths = get_platform_test_paths_for_msbuild()
    for path in test_paths:
        if os.path.exists(path):
            return path
        
    if os.path.exists("MSBuild.exe"):
        return "MSBuild.exe"
    
    return None
    
    

def is_project_file(filename, test_name):
    if filename not in test_name:
        return False

    if filename != Path(test_name).stem:
        return False

    if (test_name.endswith(".vcxproj")
            or test_name.endswith(".csproj")
            or test_name.endswith(".sln")):
        return True
    return False


def full_build(config, verbose):
    stdout = subprocess.DEVNULL if verbose is False else None
    stderr = subprocess.DEVNULL if verbose is False else None

    print(" > Performing full rebuild in [{}] Configuration".format(
        config
    ))
    
    
    proc_args = []
    if utilities.is_windows():
        msbuild = find_ms_build()
        if msbuild is None:
            print(" > Failed to find MSBuild")
            return 1
        
        proc_args = [
            "cmd.exe", "/c", msbuild,
            "{}.sln".format(project_settings.PROJECT_NAME),
            "/p:Configuration={}".format(config)
        ]
    elif utilities.is_linux():
        proc_args = ["make", "-j22"]
    else:
        raise EnvironmentError("Unsupported platform detected")
    
    ret = 0
    ret = subprocess.call(proc_args, stdout=stdout, stdin=stderr )
    if ret != 0:
        print(" > Failed to build project")

    return ret


def get_candidates(config, filename):
    candidates: list[Path] = []
    for root, dirs, files in os.walk("."):
        for name in files:
            if is_project_file(filename, name):
                candidates.append(Path(os.path.join(root, name)))
                break
    return candidates


def filter_candidates(candidates):
    for p in candidates:
        if not p.exists():
            candidates.remove(p)
            continue

        if p.suffix == ".sln":
            return p

    if len(candidates) == 0:
        return None

    return candidates[0]


def build_project(config, verbose, filename):
    stdout = subprocess.DEVNULL if verbose is False else None
    stderr = subprocess.DEVNULL if verbose is False else None

    filepath = filter_candidates(get_candidates(config, filename))
    if filepath is None:
        print("No project file found for {}".format(filename))
        print(" > defaulting to full rebuild")
        return full_build()

    print(" > Building {} in [{}] Configuration".format(
        filepath, config
    ))

    print(" > Building {}".format(filename))

    proc_args = []
    if utilities.is_windows():
        msbuild = find_ms_build()
        if msbuild is None:
            print(" > Failed to find MSBuild")
            return 1
        proc_args = [
            "cmd.exe", "/c", msbuild,
            "{}".format(filepath),
            "/p:Configuration={}".format(config)
        ]
    elif utilities.is_linux():
        proc_args = ["make", "config={}".format(config)]
    else:
        raise EnvironmentError("Unsupported platform detected")

    ret = subprocess.call(proc_args, stdout=stdout, stdin=stderr)
    return ret


def gen_projects():
    verbose: bool = True  # oe_env.is_verbose()

    stdout = subprocess.DEVNULL if verbose is False else None
    stderr = subprocess.DEVNULL if verbose is False else None

    print(" > Generating project files...")
    if os.path.exists("./tools/other/native/othernative.pyd"):
        try:
            os.remove("./tools/other/native/othernative.pyd")
        except PermissionError as e:
            print(" > Failed to remove old project file : {}".format(e))
    
    proc_args = []
    if utilities.is_windows():
        if not os.path.exists("premake\\premake5.exe"):
            print(" > Premake5 not found in [premake\\premake5.exe]")
            return 1
        proc_args = ["cmd.exe", "/c", "premake\\premake5.exe", "vs2022"]
    elif utilities.is_linux():
        if not os.path.exists("premake/premake5"):
            print(" > Premake5 not found in [premake/premake5]")
            return 1
        proc_args = ["./premake/premake5", "gmake2"]
    else:
        raise EnvironmentError("Unsupported platform detected")
    
    print("generating project files from {}".format(os.getcwd()))
    return subprocess.call(proc_args, stdout=stdout, stdin=stderr)
