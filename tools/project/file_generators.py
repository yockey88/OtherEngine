import sys
import os

def generate_engine_file(directory, filename):
    if filename == "":
        print("Must provide a name for the .cpp/.hpp file pair you wish to generate (-n=<pairname>)")
        sys.exit(1)

    if directory == "":
        print("Must provide a subdirectory for the files (-d=<directory>")
        print(" > Note this directory will be placed under OtherEngine/src if it does not already exist there")
        sys.exit(1)

    h_file = filename + ".hpp"
    c_file = filename + ".cpp"

    if directory != "":
        directory = os.path.join("OtherEngine", "src")
        directory = os.path.join(directory, directory)
        os.makedirs(directory, exist_ok=True)

        full_hpath = "OtherEngine/src/{}/{}".format(directory, h_file)
        full_cpath = "OtherEngine/src/{}/{}".format(directory, c_file)

    if os.path.exists(full_hpath):
        print("File {} already exists!".format(full_hpath))
        sys.exit(1)

    if os.path.exists(full_cpath):
        print("File {} already exists!".format(full_cpath))
        sys.exit(1)

    uc_hname = filename.upper()

    with open("{}".format(full_hpath), "w") as f:
        f.write("""/**
     * \\file {}/{}
     **/
    #ifndef OTHER_ENGINE_{}_HPP
    #define OTHER_ENGINE_{}_HPP

    namespace other {{



    }} // namespace other

    #endif // !OTHER_ENGINE_{}_HPP
    """.format(directory, h_file, uc_hname, uc_hname, uc_hname))

    with open("{}".format(full_cpath), "w") as f:
        f.write("""/**
     * \\file {}/{}
     **/
    #include "{}/{}"

    namespace other {{



    }} // namespace other
    """.format(directory, c_file, directory, h_file))
        

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