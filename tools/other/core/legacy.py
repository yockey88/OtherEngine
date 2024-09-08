import sys
import os
import subprocess

from . import commands
from . import project_settings

TOOLS_DIR = project_settings.TOOLS_DIR
engine_cmds = commands.engine_cmds

def is_build_cmnd(cmnd):
    build_cmnds = ["buildsln", "run"]
    for x in build_cmnds:
        if x == cmnd:
            return True
    return False


def parse_legacy_args(argv):
    argc = len(argv)
    i = 0

    cmd_list = []
    while i < argc:
        cmds = [argv[i]]
        while True:
            if i < argc - 1 and not is_build_cmnd(argv[i + 1]):
                cmds.append(argv[i + 1])
                i = i + 1
            else:
                break
        cmd_list.append(cmds)
        i = i + 1

    return cmd_list


def run_cmnd(cmnds):
    cmnds[0] = "{}/{}/legacy/{}.py".format(os.getcwd(), TOOLS_DIR, cmnds[0])
    print("[Command] -> {}".format(cmnds))

    ret = 0
    if os.path.exists(cmnds[0]):
        cmnds.insert(0, "python3")
        ret = subprocess.call(cmnds)
    else:
        print("Invalid Commands [{}]".format(cmnds))
        ret = -1

    return ret


def do_legacy_cmds():
    def is_not_legacy_cmd(val):
        # special case cause otherwise we run something weird
        if val == "cli.py":
            return True
        for c in engine_cmds:
            if c.short_flag == val or c.long_flag == val:
                return True
        return False

    nonlegacy_args = list(filter(is_not_legacy_cmd, sys.argv))
    legacy_args = list(set(sys.argv) - set(nonlegacy_args))
    cmd_list = parse_legacy_args(legacy_args)

    for c in cmd_list:
        print("\n---------------------------")
        print("[Executing Command] -> ", c[0])
        if len(c) > 1:
            print("\t[Flags] -> {}\n".format(" , ".join(c[1:])))
        if run_cmnd(c) != 0:
            sys.exit(1)
        print("\n")
    sys.exit(0)
