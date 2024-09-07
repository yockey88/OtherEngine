import sys

import tools.core.utilities as utilities

arguments = sys.argv[1:]
args = utilities.process_arguments(sys.argv)
config = utilities.get_arg_value(args, "-c", "debug")
FILTER = utilities.get_arg_value(args, "-f", "")

config = utilities.normalize_config_str(config)

real_args = []
if FILTER != "":
    real_args.append("--gtest_filter={}".format(FILTER))

ret = 0 if utilities.run_project(config, "OtherTestEngine", real_args) else 1

sys.exit(ret)
