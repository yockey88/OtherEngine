import sys

from core import utilities

arguments = sys.argv[1:]
args = utilities.process_arguments(sys.argv)
config = utilities.get_arg_value(args, "-c", "debug")

config = utilities.normalize_config_str(config)

ret = 0 if utilities.run_project(config, "gl_sandbox", arguments) else 1

sys.exit(ret)
