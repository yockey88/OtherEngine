import sys

import core.utilities as utilities

from project.file_generators import generate_config_file

args = utilities.process_arguments(sys.argv)
NAME = utilities.get_arg_value(args, "-n", "")
PATH = utilities.get_arg_value(args, "-p", "")

if __name__ == "__main__":
    generate_config_file(NAME, PATH)
