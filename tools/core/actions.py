import argparse


class BuildAction(argparse.Action):
    def __init__(self, **kwargs):
        super().__init__("-b", "--build", nargs='*', **kwargs)

    def __call__(self, parser, namespace, values, option_string):
        print("here")
        setattr(namespace, self.dest, values)


class RunAction(argparse.Action):
    def __init__(self, **kwargs):
        super().__init__("-r", "--run", nargs=1, **kwargs)

    def __call__(self, parser, namespace, values, option_string):
        setattr(namespace, self.dest, values)
