## premake5.lua
 ## project -> repo name
 ## Project -> repo name

## project_settings.py
 ## project -> repo name

import os , sys 

if len(sys.argv) < 2:
    print("Usage: python3 create-premake.py <project name>")
    exit(1)

repo_name = sys.argv[1]

premake_path = "./premake5.lua"

if not os.path.exists(premake_path):
    print("premake5.lua not found!")
    exit(1)
else:
    print("premake5.lua found!")
    premake_contents = open(premake_path , "r").read()
    premake_contents = premake_contents.replace("\"project\"" , "\"{}\"".format(repo_name))
    premake_contents = premake_contents.replace("\"Project\"" , "\"{}\"".format(repo_name))

    open(premake_path , "w").write(premake_contents)

proj_settings_path = "./tools/project_settings.py".format(repo_name)

if not os.path.exists(proj_settings_path):
    print("project_settings.py not found!")
    exit(1)
else:
    print("project_settings.py found!")
    project_settings_contents = open(proj_settings_path , "r").read()
    project_settings_contents = project_settings_contents.replace("\"project\"" , "\"{}\"".format(repo_name))

    open(proj_settings_path , "w").write(project_settings_contents)
