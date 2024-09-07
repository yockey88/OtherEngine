import os
import subprocess
import sys
import platform

from tools.core.project_settings import TOOLS_DIR

PLATFORM = sys.platform
for x in platform.uname():
    if "microsoft" in x.lower():
        PLATFORM = "windows"
        break


def normalize_config_str(config):
    if config == "debug":
        return "Debug"
    if config == "release":
        return "Release"

    return config


def run_project(config, name, arguments):
    if is_windows():
        proc_args = ["cmd.exe", "/c",
                     "{}\\run.bat".format(TOOLS_DIR),
                     config, name]
        proc_args.extend(arguments)
        ret = subprocess.call(proc_args, cwd=os.getcwd())
        return True if ret == 0 else False
    else:
        return False


def is_windows():
    return PLATFORM == "windows"


def is_linux():
    return PLATFORM == "linux"


def is_mac():
    return PLATFORM == "darwin"


def process_arguments(argv):
    ret = {}
    for arg in argv:
        try:
            k = arg[0:arg.index("=")]
            v = arg[arg.index("=")+1:]
        except Exception:
            k = arg
            v = 0
        ret[k] = v
    return ret


def get_arg_value(args, name, default):
    return args[name] if name in args else default


class Singleton:
    def __init__(self, decorated):
        self._decorated = decorated

    def instance(self):
        try:
            return self._instance
        except AttributeError:
            self._instance = self._decorated()
            return self._instance

    def __call__(self):
        raise TypeError('Singletons must be accessed through `instance()`.')

    def __instancecheck__(self, inst):
        return isinstance(inst, self._decorated)
