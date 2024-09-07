import sys
import os

import tools.core.utilities as utilities
from tools.project.file_generators import generate_engine_file

args = utilities.process_arguments(sys.argv)
DIRECTORY = utilities.get_arg_value(args, "-d", "")
FILE_NAME = utilities.get_arg_value(args, "-n", "")
        
if __name__ == "__main__":
    generate_engine_file(DIRECTORY, FILE_NAME)