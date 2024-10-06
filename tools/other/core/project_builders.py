import os
import subprocess
import sys

from pathlib import Path

from . import project_settings
from . import utilities

MSBUILD = "MSBuild.exe"
if sys.platform == "win32" or sys.platform == "cygwin":
    try:
        MSBUILD = os.environ["MSBUILD"]
    except KeyError:
        MSBUILD = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe"
elif sys.platform == "linux":
    try:
        MSBUILD = os.environ["MSBUILD"][1:-1]
    except KeyError:
        MSBUILD = "/mnt/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe"


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
    
    msbuild = MSBUILD
    if sys.platform == "linux":
        if not os.path.exists(msbuild):
            print(" > Failed to find MSBuild at [{}]".format(msbuild))
            print("   > Ensure that MSBuild is installed and available in PATH and MSBUILD environment variable is set")
            return
        else:
            ### must be a linux path
            msbuild = "C:\\\\" + msbuild[7:].replace("/", "\\")
    elif not os.path.exists(msbuild):
        print(" > Failed to find MSBuild at [{}]".format(msbuild))
        print("   > Ensure that MSBuild is installed and available in PATH and MSBUILD environment variable is set")
        return
    else:
        print(" > Found MSBuild at [{}]".format(msbuild))

    ret = 0
    if utilities.is_windows():
        ret = subprocess.call(
            [
                "cmd.exe", "/c", msbuild,
                "{}.sln".format(project_settings.PROJECT_NAME),
                "/p:Configuration={}".format(config)
            ],
            stdout=stdout, stdin=stderr
        )
    else:
        raise EnvironmentError("Non-windows platform detected")

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

    ret = 0
    if utilities.is_windows():
        ret = subprocess.call(
            [
                "cmd.exe", "/c", MSBUILD,
                "{}".format(filepath),
                "/p:Configuration={}".format(config),
                "/p:BuildProjectReferences=false"
            ],
            stdout=stdout, stdin=stderr
        )
    else:
        raise EnvironmentError("Non-windows platform detected")

    return ret


def gen_projects():
    verbose: bool = True  # oe_env.is_verbose()

    stdout = subprocess.DEVNULL if verbose is False else None
    stderr = subprocess.DEVNULL if verbose is False else None

    print(" > Generating project files...")
    return subprocess.call(
        ["cmd.exe", "/c", "premake\\premake5.exe", "vs2022"],
        stdout=stdout, stdin=stderr
    )
