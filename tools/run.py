import project_settings
import os , subprocess , sys

args = project_settings.ProcessArguments(sys.argv)
config = project_settings.GetArgValue(args , "c" , "debug")
project = project_settings.GetArgValue(args , "p" , project_settings.EXE_NAME)

if (config == "debug"):
    config = "Debug"
    
if (config == "release"):
    config = "Release"

exepath = "{}/bin/{}/{}".format(os.getcwd() , config , project_settings.EXE_NAME)
ret = 0

if project_settings.IsWindows():
    print("Running: {}\\run.bat {} {}".format(project_settings.TOOLS_DIR , config , project))
    ret = subprocess.call(
        [
            "cmd.exe" , "/c" , "{}\\run.bat".format(project_settings.TOOLS_DIR) , 
            config , project ,
        ] , 
        cwd=os.getcwd()
    )
else:
    ret = subprocess.call(["{}".format(exepath)] , cwd=exepath)

sys.exit(ret)
