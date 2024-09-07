import project_settings
import utilities
import sys

arguments = sys.argv[1:]
args = project_settings.ProcessArguments(sys.argv)
config = project_settings.GetArgValue(args, "-c", "debug")

if (config == "debug"):
    config = "Debug"

if (config == "release"):
    config = "Release"

ret = 0 if utilities.run_project(config, "sandbox", arguments) else 1
sys.exit(ret)
