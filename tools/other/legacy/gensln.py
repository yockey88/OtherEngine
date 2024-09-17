import subprocess
import sys

if __name__ == "__main__":
    ret = 0

    ret = subprocess.call(
        ["cmd.exe", "/c", "premake\\premake5.exe", "vs2022"]
    )
    if (ret != 0):
        sys.exit(ret)
