import project_settings
import subprocess
import sys

ret = 0


def generate():
    if project_settings.IsWindows():
        ret = subprocess.call(
            ["cmd.exe", "/c", "premake\\premake5", "vs2022"]
        )
        if (ret != 0):
            sys.exit(ret)
        ret = subprocess.call(
            [
                "cmd.exe", "/c",
                "C:\\Yock\\gamedev\\Testbed\\premake\\premake5",
                "vs2022",
                "--file=C:\\Yock\\gamedev\\Testbed\\premake5.lua"
            ]
        )
