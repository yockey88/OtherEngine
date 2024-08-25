import project_settings
import utilities
import sys

arguments = sys.argv[1:]
args = project_settings.ProcessArguments(sys.argv)
config = project_settings.GetArgValue(args, "-c", "debug")
FILTER = project_settings.GetArgValue(args, "-f", "")

if (config == "debug"):
    config = "Debug"

if (config == "release"):
    config = "Release"

real_args = []
if FILTER != "":
    real_args.append("--gtest_filter={}".format(FILTER))

ret = 0 if utilities.run_project(config, "OtherTestEngine", real_args) else 1

sys.exit(ret)
