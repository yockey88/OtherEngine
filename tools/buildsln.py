import project_settings
import gensln
import os
import subprocess
import sys

args = project_settings.ProcessArguments(sys.argv)
print(args)
CONFIG = project_settings.GetArgValue(args, "-c", "debug")

if (CONFIG == "debug"):
    CONFIG = "Debug"

if (CONFIG == "release"):
    CONFIG = "Release"

print("Building {} in [{}] Configuration".format(
    project_settings.EXE_NAME, CONFIG)
)
print("\n")

ret = 0

first_build = False
if not os.path.exists("bin/{}/{}".format(CONFIG, project_settings.EXE_NAME)):
    first_build = True

MSBUILD = \
    "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe"


def find_msbuild():
    if project_settings.IsWindows():
        if os.path.exists(MSBUILD):
            return True
        else:
            MSBUILD.replace("2022", "2019")
            if os.path.exists(MSBUILD):
                return True
            else:
                return False
    else:
        return False


gensln.generate()


if project_settings.IsWindows():
    MSBUILD = os.environ["MSBUILD"][8:-1]
    MSBUILD = "C:\\\\" + MSBUILD
    ret = subprocess.call(
        [
            "cmd.exe", "/c", MSBUILD,
            "{}.sln".format(project_settings.PROJECT_NAME),
            "/property:Configuration={}".format(CONFIG)
        ]
    )
    if (ret != 0):
        sys.exit(ret)

if project_settings.IsLinux():
    ret = subprocess.call(["make", "config={}".format(CONFIG)])

if project_settings.IsMac():
    ret = subprocess.call(["make", "config={}".format(CONFIG)])

sys.exit(ret)
