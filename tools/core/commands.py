from argparse import Action
from argparse import ArgumentParser

from tools.core.actions import BuildAction
from tools.core.actions import RunAction


class Command:
    def __init__(self, sflag, lflag, help_msg,
                 action=None, nargs=None, choices=None, required=False,
                 metavar=None, default=None,
                 **kwargs):
        self.long_flag = lflag
        self.name = lflag[2:].replace('-', '_')
        self.short_flag = sflag
        self.help_message = help_msg

        if action is None:
            self.action = 'store_true'
        else:
            self.action = action.__init__()

        self.nargs = nargs
        self.choices = choices

        if choices is not None and nargs is None:
            self.nargs = 1

        self.required = required
        self.metavar = metavar
        self.default = default

    def define_self(self, parser):
        parser.add_argument(
            self.short_flag, self.long_flag,
            help=self.help_message, action=self.action
        )
        for act in parser._actions:
            if act.dest == self.name:

                if self.nargs is not None:
                    act.nargs = self.nargs
                else:
                    act.nargs = '?'

                act.choices = self.choices
                act.dest = self.name
                act.required = self.required
                act.dest = self.name

                if self.metavar is not None:
                    act.metavar = self.metavar

                break

    name: str
    short_flag: str
    long_flag: str
    help_message: str

    action: Action
    nargs = None
    choices = None
    required: bool
    metavar = None
    default = None


engine_cmds: list[Command] = [
    # use legacy commands
    Command(
        "-lc", "--legacy-cmd",
        """runs all commands in 'legacy-command' list using legacy tools.
            This will disable all new tools and features and will only run the
            commands in the 'command' list""",
        nargs='*', metavar="command"
    ),

    # verbosity commands
    Command(
        "-v", "--verbose",
        "output verbose logging (trace level logging)"
    ),

    # build project
    Command(
        "-b", "--build",
        """attempts to build projects with names in 'project' list,
        if no arguments are passed a full rebuild is attempted
        (all libraries, engine components, and projects)""",
        action=BuildAction(), nargs='*', metavar="project"
    ),

    # run project
    Command(
        "-r", "--run",
        "runs the project specified by 'project' if it exists",
        action=RunAction(), metavar="project"
    ),

    # configuration
    Command(
        "-c", "--config",
        "sets the configuration for the passed in commands",
        action='store_const', choices=["debug", "release"], 
        default="debug"
    )
]


def initialize_parser():
    parser = ArgumentParser(
        prog="OtherEngine Tools Pipeline",
        description="""
        The Python Interface for interacting with the Other Engine from the
        command line and managing projects, assets, dependencies and other
        """,
        epilog="""
        Refer to Other Engine documentation for more detailed help
            (if it has been written yet)
        """
    )

    for c in engine_cmds:
        c.define_self(parser)

    return parser
