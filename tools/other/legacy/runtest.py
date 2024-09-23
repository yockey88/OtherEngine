import sys

from other.core import utilities

if __name__ == "__main__":
    arguments = sys.argv[1:]
    args = utilities.process_arguments(sys.argv)
    FILTER = utilities.get_arg_value(args, "-f", "")
    REPEAT = utilities.get_arg_value(args, "-r", "")
    config = utilities.get_arg_value(args, "-c", "debug")

    args = []
    if FILTER != "":
        args.append("--gtest_filter={}".format(FILTER))

    if REPEAT != "":
        args.append("--gtest_repeat={}".format(REPEAT))

    ret = 0 if utilities.run_project(config, "unit_tests", args) else 1

    sys.exit(ret)
