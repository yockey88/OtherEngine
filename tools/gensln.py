import project_settings
import subprocess , sys

ret = 0;

if project_settings.IsWindows():
    ret = subprocess.call(
        ["cmd.exe" , "/c" , "premake\\premake5" , "vs2022"]
    )
    if (ret != 0):
        sys.exit(ret)

if project_settings.IsLinux():
    ret = subprocess.call(["premake/premake5.linux" , "gmake2"])

if project_settings.IsMac():
    ret = subprocess.call(["premake/premake5" , "gmake2"])
    if ret == 0:
        subprocess.call(["premake/premake5" , "xcode4"])

sys.exit(ret)
