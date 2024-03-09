import os, sys
import subprocess

TOOLS_DIR = "tools"

def RunCmnd(cmnds):
    ret = 0
    cmnds[0] =  "{}/{}/{}.py".format(os.getcwd() , TOOLS_DIR , cmnds[0])
    if os.path.exists(cmnds[0]):
        cmnds.insert(0 , "python3")
        ret = subprocess.call(cmnds)
    else:
        print("Invalid Commands [{}]".format(cmnds))
        ret = -1

    return ret

argc = len(sys.argv)
i = 1
while i < argc:
    cmnds = [sys.argv[i]]

    while True:
        if i < argc - 1 and sys.argv[i + 1][0] == '-':
            cmnds.append(sys.argv[i + 1][1:])
            i = i + 1
        else :
            break

    print("\n---------------------------")
    print("[Executing Command] -> " , cmnds[0])
    if len(cmnds) > 1:
        print("\t[Flags] -> {}\n".format(" , ".join(cmnds[1:])))
    if RunCmnd(cmnds) != 0:
        break
    print("\n")
    i = i + 1
