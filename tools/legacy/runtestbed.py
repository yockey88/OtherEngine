import project_settings
import os
import subprocess
import sys

arguments = sys.argv[1:]
args = project_settings.ProcessArguments(sys.argv)

CMD = "C:\\\\Yock\\gamedev\\Testbed\\bin\\Debug\\testbed.exe"
cmds = ["cmd.exe", "/c", CMD,
        "--project", "C:\\\\Yock\\gamedev\\Testbed\\testbed.other",
        "--editor",
        "--cwd", "C:\\\\Yock\\gamedev\\Testbed"]

ret = subprocess.call(cmds, cwd=os.getcwd())
sys.exit(ret)
