import sys
import os

from pathlib import Path


template_cpp = \
"""
/**
 * \\file {}/{}.cpp
 **/
#include "{}/{}.hpp"

namespace other {{\n\n\n}} // namespace other
"""

template_hpp = \
"""
/**
 * \\file {}/{}.cpp
 **/
#ifndef OTHER_ENGINE_{}_HPP
#define OTHER_ENGINE_{}_HPP

namespace other {{\n\n\n}} // namespace other

#endif // !OTHER_ENGINE_{}_HPP
"""

# proj_template_cpp = \
# """
# /**
#   * \\file {}/{}.cpp
#   **/
# #include "{}/{}.hpp"

# namespace {} {{\n\n\n}} // namespace {}
# """

# proj_template_hpp = \
# """
# /**
#   * \\file {}/{}.hpp
#   **/
# #ifndef {}_{}_HPP
# #define {}_{}_HPP

# namespace {} {{\n\n\n}} // namespace {}

# #endif // !{}_{}_HPP
# """


def generate_engine_files(project="", directory="", filename=""):
  verbose: bool = True  # oe_env.is_verbose()

  if verbose:
    print("Generating files...")
    print(" > project: {}".format(project))
    print(" > directory: {}".format(directory))
    print(" > filename: {}".format(filename))

  if os.path.exists(directory) is False:
    os.makedirs(directory)

  p = Path("OtherEngine/src") / directory / filename
  if p.exists():
    print(" !> file already exists!")
    return 1

  hpp = p.with_suffix(".hpp")
  cpp = p.with_suffix(".cpp")
  uc_hname = filename.upper()

  if hpp.exists() or cpp.exists():
    print(" !> one of the .cpp/.hpp pair (or both) already exists!")
    return 1

  if verbose:
    print(" > creating files: [{}]\n  - {}\n  - {}".format(uc_hname, cpp, hpp))

    with open(hpp, "w") as f:
      f.write(template_hpp.format(directory, filename, uc_hname, uc_hname, uc_hname))

    with open(cpp, "w") as f:
      f.write(template_cpp.format(directory, filename, directory, filename))

    return 0
  
def generate_build_file(name=""):
  if name == "":
    print("Please provide a name for the project (-n)")
    return 1
  
  with open("{}/{}/premake5.lua".format(Path("."), name), "w") as f:
    f.write("local {} = {{\n".format(name))
    f.write("  name = \"{}\",\n".format(name))
    f.write("  path = \"./{}\",\n".format(name))
    f.write("  kind = \"ConsoleApp\",\n")
    f.write("  language = \"C++\",\n")
    f.write("  cppdialect = \"C++latest\",\n")
    f.write("  architecture = \"x86_64\",\n")
    f.write("\n")
    f.write("  files = function()\n")
    f.write("    files {{\n")
    f.write("      \"./{}/src/**.cpp\",\n".format(name))
    f.write("      \"./{}/src/**.hpp\",\n".format(name))
    f.write("    }}\n")
    f.write("  end,\n")
    f.write("\n")
    f.write("  include_dirs = function()\n")
    f.write("    includedirs {{ \"./{}\" }}\n".format(name))
    f.write("    externalincludedirs {{ \"%{{wks.location}}/DotOther/NetCore\" }}\n")
    f.write("  end,\n")
    f.write("\n")
    f.write("  defines = function()\n")
    f.write("    defines {{ \"OE_MODULE\" }}\n")
    f.write("  end,\n")
    f.write("\n")
    f.write("  windows_configuration = function()\n")
    f.write("    entrypoint \"WinMainCRTStartup\"\n")
    f.write("  end,\n")
    f.write("\n")
    f.write("  components = {{\n")
    f.write("    [\"OtherEngine\"] = \"%{{wks.location}}/OtherEngine/src\"\n")
    f.write("  }}\n")
    f.write("}}\n")
    f.write("\n")
    f.write("AddProject({})\n".format(name))

  return 0

def generate_config_file(name):
  if name == "":
    print("Please provide a name for the project (-n)")
    return 1

  path = "{}/{}.other".format(name, name)
  os.makedirs(name, exist_ok=True)
  print("Generating project: {}".format(name))

  with open(path, "w") as f:
    f.write("[project]\n")
    f.write("name = \"{}\"\n".format(name))
    f.write("author = \"<no-author>\"\n")
    f.write("version = 0.0.1\n")
    f.write("need-primary-scene = false\n")
    f.write("\n")
    f.write("[window]\n")
    f.write("width = 1280\n")
    f.write("height = 720\n")
    f.write("title = \"Other Engine {}\"\n".format(name))
    f.write("\n")
    f.write("[log]\n")
    f.write("console-level = \"trace\"\n")
    f.write("file-level = \"trace\"\n")
    f.write("path = \"./logs/{}.log\"\n".format(name))

# def generate_project_file(name):
#   if name == "":
#     print("Please provide a name for the project")
#     return 1

#   src_dir = os.path.join(name, "src")
#   os.makedirs(src_dir, exist_ok=True)
#   print("Generating project: {}".format(name))

#   with open("{}/{}.cpp".format(src_dir, name), "w") as f:
#     f.write(proj_template_cpp.format(name, name, name, name, name, name))

#   with open("{}/{}.hpp".format(src_dir, name), "w") as f:
#     f.write(proj_template_hpp.format(name, name, name, name, name, name, name, name))

#   return 0