import project_settings
import os
import subprocess
import sys

arguments = sys.argv[1:]
args = project_settings.ProcessArguments(sys.argv)
project = project_settings.GetArgValue(args, "-n", project_settings.EXE_NAME)
config = project_settings.GetArgValue(args, "-c", "debug")
tests = project_settings.GetArgValue(args, "-t", "false")

if (config == "debug"):
    config = "Debug"

if (config == "release"):
    config = "Release"

ret = 0

if project_settings.IsWindows():
    proc_args = ["cmd.exe", "/c", "{}\\run.bat".format(project_settings.TOOLS_DIR), config, "sandbox"]
    proc_args.extend(arguments)
    ret = subprocess.call(proc_args, cwd=os.getcwd())
else:
    ret = 1

sys.exit(ret)
