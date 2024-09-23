import tomllib
from argparse import ArgumentParser

from . import commands

def parse_config(path):
  with open(path, "rb") as f:
    return tomllib.load(f)
  return None

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

  for c in commands.engine_cmds:
    c.define_self(parser)

  return parser