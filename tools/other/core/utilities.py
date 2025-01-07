from __future__ import annotations
from abc import ABCMeta, abstractmethod

import os
import subprocess
import sys
import platform
from pathlib import Path

from . import project_settings
from . import project_builders
from . import file_generators

TOOLS_DIR = project_settings.TOOLS_DIR
PLATFORM = sys.platform

if sys.platform == "win32":
    PLATFORM = "windows"
else:
    for x in platform.uname():
        if "microsoft" in x.lower():
            PLATFORM = "windows"
            break

def print_platform_string():
    print("Platform: {}".format(PLATFORM))

def normalize_config_str(config):
    return config[0].upper() + config[1:]


def run_project(config, name, arguments):
    proc_args = []
    if is_windows():
        proc_args = ["cmd.exe", "/c", "{}\\run.bat".format(TOOLS_DIR), config, name]
    elif is_linux():
        proc_args = ["sh", "{}\\run.sh".format(TOOLS_DIR), config, name]
    else:
        return False
    proc_args.extend(arguments)
    ret = subprocess.call(proc_args, cwd=os.getcwd())
    return True if ret == 0 else False

def run_dotnet_project(config, name, arguments = []):
    if is_windows():
        print(" > running {}".format(name))
        proc_args = ["pwsh" , "-Command", 
                     "& ./bin/{}/net8.0/{}.exe".format(config,name,name)]
        
        if len(arguments) > 0:
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
    if arg is None or len(arg) == 0:
        print(" > no argument provided to fnv")
        return 1
    
    if len(arg) > 1:
        print(" > too many arguments provided to fnv")
        return 1
    
    proc_args = []
    if is_windows():
        proc_args = [ "cmd.exe", "/c", ".\\bin\\Debug\\fnv\\fnv.exe", arg[0] ]
    elif is_linux():
        proc_args = [ "./bin/Debug/fnv/fnv", arg[0] ]
    else:
        return 1
    
    return subprocess.call(proc_args, cwd=os.getcwd())

def create_project(name):
    # create top-level project directory
    os.makedirs(name, exist_ok=True)
    print(" > creating project {}".format(name))
    
    #create core project directory
    proj_dir = os.path.join(name, name)
    os.makedirs(proj_dir, exist_ok=True)
    src_dir = os.path.join(proj_dir, "src")
    assets_dir = os.path.join(proj_dir, "assets")
    scripts_dir = os.path.join(proj_dir, "scripts")
    editor_dir = os.path.join(proj_dir, "editor")
    materials_dir = os.path.join(proj_dir, "materials")
    shaders_dir = os.path.join(proj_dir, "shaders")
    scenes_dir = os.path.join(proj_dir, "scenes")

    os.makedirs(src_dir, exist_ok=True)
    os.makedirs(assets_dir, exist_ok=True)
    os.makedirs(scripts_dir, exist_ok=True)
    os.makedirs(editor_dir, exist_ok=True)
    os.makedirs(materials_dir, exist_ok=True)
    os.makedirs(shaders_dir, exist_ok=True)
    os.makedirs(scenes_dir, exist_ok=True)
    
    file_generators.generate_config_file(name)
    file_generators.generate_build_file(name)
    return 0

class Singleton(metaclass=ABCMeta):
    __metaclass__ = ABCMeta
    instance: Singleton = None
    name = None

    def __new__(self, name=None, *args, **kwargs):
        if not self.instance:
            self.instance = super(Singleton, self).__new__(self, *args, **kwargs)
            self.name = name
        return self.instance
