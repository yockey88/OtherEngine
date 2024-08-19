import project_settings
import sys
import os

args = project_settings.ProcessArguments(sys.argv)

DIRECTORY = project_settings.GetArgValue(args, "-d", "")
FILE_NAME = project_settings.GetArgValue(args, "-n", "")

if FILE_NAME == "":
    print("Must provide a name for the .cpp/.hpp file pair you wish to generate (-n=<pairname>)")
    sys.exit(1)

if DIRECTORY == "":
    print("Must provide a subdirectory for the files (-d=<directory>")
    print(" > Note this directory will be placed under OtherEngine/src if it does not already exist there")
    sys.exit(1)

h_file = FILE_NAME + ".hpp"
c_file = FILE_NAME + ".cpp"

if DIRECTORY != "":
    directory = os.path.join("OtherEngine", "src")
    directory = os.path.join(directory, DIRECTORY)
    os.makedirs(directory, exist_ok=True)

    full_hpath = "OtherEngine/src/{}/{}".format(DIRECTORY, h_file)
    full_cpath = "OtherEngine/src/{}/{}".format(DIRECTORY, c_file)

if os.path.exists(full_hpath):
    print("File {} already exists!".format(full_hpath))
    sys.exit(1)

if os.path.exists(full_cpath):
    print("File {} already exists!".format(full_cpath))
    sys.exit(1)

uc_hname = FILE_NAME.upper()

with open("{}".format(full_hpath), "w") as f:
    f.write("""/**
 * \\file {}/{}
 **/
#ifndef OTHER_ENGINE_{}_HPP
#define OTHER_ENGINE_{}_HPP

namespace other {{



}} // namespace other

#endif // !OTHER_ENGINE_{}_HPP
""".format(DIRECTORY, h_file, uc_hname, uc_hname, uc_hname))

with open("{}".format(full_cpath), "w") as f:
    f.write("""/**
 * \\file {}/{}
 **/
#include "{}/{}"

namespace other {{



}} // namespace other
""".format(DIRECTORY, c_file, DIRECTORY, h_file))
