import tomllib
from argparse import ArgumentParser

from . import commands

def parse_config(path):
  with open(path, "rb") as f:
    return tomllib.load(f)
  return None

def initialize_parser(pipeline_config):
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

  config_table = []
  try:
    configs = pipeline_config["configuration"]
    for c in configs:
      config_table.append(c["name"])
  except:
    pass

  if len(config_table) == 0:
    config_table.append("debug")
    config_table.append("release")

  commands.engine_cmds.append(
    commands.Command(
        "-c", "--config",
        "sets the configuration for the passed in commands",
        action='store_const', choices=config_table, 
        default="debug"
    )
  );

  for c in commands.engine_cmds:
    c.define_self(parser)

  return parser