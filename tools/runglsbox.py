import project_settings
import os
import subprocess
import sys

arguments = sys.argv[1:]
args = project_settings.ProcessArguments(sys.argv)
config = project_settings.GetArgValue(args, "-c", "debug")

if (config == "debug"):
    config = "Debug"

if (config == "release"):
    config = "Release"

ret = 0

if project_settings.IsWindows():
    proc_args = ["cmd.exe", "/c", "{}\\run.bat".format(project_settings.TOOLS_DIR), config, "gl_sandbox"]
    proc_args.extend(arguments)
    ret = subprocess.call(proc_args, cwd=os.getcwd())
else:
    ret = 1

sys.exit(ret)
