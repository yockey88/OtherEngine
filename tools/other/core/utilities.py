from __future__ import annotations
from abc import ABCMeta, abstractmethod

import os
import subprocess
import sys
import platform

from . import project_settings

TOOLS_DIR = project_settings.TOOLS_DIR
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

def fnv(arg):
    return subprocess.call(
        ["cmd.exe", "/c", ".\\bin\\Debug\\fnv\\fnv.exe", arg]
    )


class Singleton(metaclass=ABCMeta):
    __metaclass__ = ABCMeta
    instance: Singleton = None
    name = None

    def __new__(self, name=None, *args, **kwargs):
        if not self.instance:
            self.instance = super(Singleton, self).__new__(self, *args, **kwargs)
            self.name = name
        return self.instance
