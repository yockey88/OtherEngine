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

def generate_config_file(name="", path=""):
    if name == "":
      print("Please provide a name for the project (-n)")
      sys.exit(1)

    if path == "":
        path = name

    path = "tests/{}".format(path)

    if not os.path.exists(path):
        os.makedirs(path)
        print("Created directory: {}".format(path))

    print("Generating project: {}".format(name))

    with open("{}/{}.other".format(path, name), "w") as f:
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
