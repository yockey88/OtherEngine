import sys

from tools import other

if __name__ == "__main__":
    # other.env.init()
    parser = other.initialize_parser()
    if len(sys.argv) == 0:
        parser.print_help()
        sys.exit(0)

    other.env.parse_args(parser)

    verbose: bool = other.env.is_verbose()

    if verbose:
        print("Verbose Logging toggled on")
        print("config : {}".format(other.env.configuration()))

    if other.env.is_legacy():
        if verbose:
            print("> running legacy commands")
        sys.exit(other.do_legacy_cmds())

    other.pipeline.try_build()
