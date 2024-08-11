import project_settings
import sys
import os

args = project_settings.ProcessArguments(sys.argv)
print(args)

NAME = project_settings.GetArgValue(args, "-n", "")

if NAME == "":
    print("Please provide a name for the project (-n)")
    sys.exit(1)

PATH = project_settings.GetArgValue(args, "-p", "")

if PATH == "":
    PATH = NAME

PATH = "tests/{}".format(PATH)

if not os.path.exists(PATH):
    os.makedirs(PATH)
    print("Created directory: {}".format(PATH))

print("Generating project: {}".format(NAME))

with open("{}/{}.other".format(PATH, NAME), "w") as f:
    f.write("[project]\n")
    f.write("name = \"{}\"\n".format(NAME))
    f.write("author = \"<no-author>\"\n")
    f.write("version = 0.0.1\n")
    f.write("\n")
    f.write("[window]\n")
    f.write("width = 1280\n")
    f.write("height = 720\n")
    f.write("title = \"Other Engine {}\"\n".format(NAME))
    f.write("\n")
    f.write("[log]\n")
    f.write("console-level = \"trace\"\n")
    f.write("file-level = \"trace\"\n")
    f.write("path = \"./logs/{}.log\"\n".format(NAME))
