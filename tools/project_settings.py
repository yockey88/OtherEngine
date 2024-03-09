EXE_NAME = "project"
PROJECT_NAME = "TestWorkspace"

TOOLS_DIR = "tools"

VMAJOR = 0
VMINOR = 0
VPATCH = 2

import sys , platform
PLATFORM = sys.platform
for x in platform.uname():
    if "microsoft" in x.lower():
        PLATFORM = "windows"
        break

def IsWindows():
    return PLATFORM == "windows"

def IsLinux():
    return PLATFORM == "linux"

def IsMac():
    return PLATFORM == "darwin"

def ProcessArguments(argv):
    ret = {}
    for arg in argv:
        try:
            k = arg[0:arg.index("=")]
            v = arg[arg.index("=")+1:]
        except:
            k = arg
            v = 0
        ret[k] = v
    return ret

def GetArgValue(args , name , default):
    return args[name] if name in args else default

