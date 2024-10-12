from argparse import Action
from argparse import ArgumentParser

from .actions import EditAction
from .actions import BuildAction
from .actions import RunAction
from .actions import RunDotnetAction
from .actions import TestAction
from .actions import GenFileAction


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
                    act.nargs = 0

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

    # edit project
    Command(
        "-e", "--edit",
        "opens the project specified by 'project' in the default editor",
        nargs=1, metavar="project", action=EditAction()
    ),

    # build project
    Command(
        "-b", "--build",
        """attempts to build projects with names in 'project' list,
        if no arguments are passed a full rebuild is attempted
        (all libraries, engine components, and projects)""",
        action=BuildAction(), nargs='*', metavar="project"
    ),

    # testing
    Command(
        "-t", "--test",
        """runs the tests for the test specified by 'test' if it exists,
           otherwise runs the entire test suite if no arguments are passed""",
        action=TestAction(), nargs='*', metavar="test"
    ) ,
    
    ### TODO:
    #       - this command should be loaded from DotOther's pipeline config
    # Command(
    #     "-dot", "--dotother-test",
    #     """...""",
    #     action=DotOtherTestAction(), nargs='*', metavar="dotother-test"
    # ) ,

    # run project
    Command(
        "-r", "--run",
        "runs the project specified by 'project' if it exists followed by the command line arguments",
        action=RunAction(), nargs='*', metavar="project"
    ),

    Command(
        "-rdn", "--run-dotnet",
        "runs the dotnet project specified by 'project' if it exists",
        action=RunDotnetAction(), nargs='*', metavar="project"
    ),

    # configuration
    Command(
        "-c", "--config",
        "sets the configuration for the passed in commands",
        action='store_const', choices=["debug", "release"], 
        default="debug"
    ),

    # generate engine files
    Command(
        "-gf", "--generate-files",
        """generates files named file.cpp/file.hpp placing them in 
        directory 'directory' of project 'project' if 'project' exists and
        is registered with engine. If 'directory' does not exist it is created""",
        action=GenFileAction(), nargs=3, metavar=("project", "directory", "file")
    ),

    # generate projects
    Command(
        "-gp", "--generate-projects",
        """generates project files (.sln/.vcxproj/.csproj) files 
        for all projects in the project cache 
        (see your pipeline .cfg file [[project]] section)"""
    ), 
    
    # fnv hash
    Command(
        "-fnv", "--fnv",
        "runs the fnv hash on the input item",
        action='store_true', nargs=1, metavar="item"
    ),

    # view platform
    Command(
        "-vp", "--view-platform",
        "prints the current platform"
    )
]
