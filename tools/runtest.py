import project_settings
import os
import subprocess
import sys

arguments = sys.argv[1:]
args = project_settings.ProcessArguments(sys.argv)
FILTER = project_settings.GetArgValue(args, "-f", "")
REPEAT = project_settings.GetArgValue(args, "-c", "")

CMD = ".\\bin\\Debug\\unit_tests\\unit_tests.exe"
cmds = ["cmd.exe", "/c", CMD]
if FILTER != "":
    cmds.append("--gtest_filter={}".format(FILTER))

if REPEAT != "":
    cmds.append("--gtest_repeat={}".format(REPEAT))

ret = subprocess.call(cmds, cwd=os.getcwd())

sys.exit(ret)
