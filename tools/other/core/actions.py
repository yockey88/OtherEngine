import argparse

## VERY minimial at the moment, simply to have them here when its time to fill them out

class EditAction(argparse.Action):  
    def __init__(self, **kwargs):
        super().__init__("-e", "--edit", nargs=1, **kwargs)

    def __call__(self, parser, namespace, values, option_string):
        setattr(namespace, self.dest, values)

class BuildAction(argparse.Action):
    def __init__(self, **kwargs):
        super().__init__("-b", "--build", nargs='*', **kwargs)

    def __call__(self, parser, namespace, values, option_string):
        setattr(namespace, self.dest, values)


class RunAction(argparse.Action):
    def __init__(self, **kwargs):
        super().__init__("-r", "--run", nargs=1, **kwargs)

    def __call__(self, parser, namespace, values, option_string):
        setattr(namespace, self.dest, values)

class GenFileAction(argparse.Action):
    def __init__(self, **kwargs):
        super().__init__("-g", "--gen-file", nargs=1, **kwargs)

    def __call__(self, parser, namespace, values, option_string):
        setattr(namespace, self.dest, values)
