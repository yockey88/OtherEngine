import subprocess
import sys

ret = 0

ret = subprocess.call(
    ["cmd.exe", "/c", "premake\\premake5.exe", "vs2022"]
)
if (ret != 0):
    sys.exit(ret)
