from __future__ import annotations

import sys
import os

from typing import override

from argparse import Namespace
from pathlib import Path

from ..core import Singleton
from ..core import utilities
from ..core import parsers
from ..native import engine as oe

from dataclasses import dataclass

@dataclass
class PipelineConfig:
  engine_path: str
  project_list = []

  def __init__(self, engine_path: str = ".", project_list = []):
    self.engine_path = engine_path
    self.project_list = project_list
  
  def find_project(self, name: str):
    for proj_name, path in self.project_list:
      if proj_name == name:
        return (proj_name, path)
    return None

class OtherEnginePipelineEnvironment(Singleton):
  settings: Namespace
  project_config: str

  pipeline_config = None

  help_printed: bool = False

  def __init__(self):
    self.settings = None
    self.project_config = "debug"

  def _configure(self):
    ...

  def init(self, path="default", parse_cmds=True):
    p = Path(os.getcwd()) / path
    if not os.path.exists(p):
      raise IOError(
        """project_config file for Other Engine tool pipeline [{}] not found.
        Currently in directory [{}]""".format(
          p, os.getcwd()
        )
      )

    table = parsers.parse_config(p)
    pipeline_config = self._process_config_table(table)

    if pipeline_config is None:
      raise ValueError(
        "NO pipeline configuration found for tool pipeline"
      )
    
    self.pipeline_config = pipeline_config
    cfg = self.pipeline_config.find_project("yockcraft")
    if cfg is not None:
      print("Found project configuration for 'yockcraft' at [{}]".format(cfg[1]))
      oe.Native.set_cfg_path(str(p))

    if parse_cmds:
      parser = parsers.initialize_parser()
      if len(sys.argv) == 1:
        parser.print_help()
        self.help_printed = True
      self.parse_args(parser)
    else:
      self.settings = Namespace()
      self.settings.verbose = False
      self.settings.legacy_cmd = False
      self.settings.build = None
      self.settings.config = None

    if self.settings is None:
      self.settings = Namespace()
      
    if self.settings.verbose:
      print("Pipeline settings loaded")
      print("Project configuration: {}".format(self.project_config))
      print("Verbose mode: {}".format(self.settings.verbose))
      print("Legacy mode: {}".format(self.settings.legacy_cmd))
      print("Build list: {}".format(self.settings.build))
      print("Configuration: {}".format(self.settings.config))
      print("Pipeline configuration loaded\n")

      print("Command line arguments: {}".format(self.settings))

      print("Engine Config :>\n  - path: '{}'".format(pipeline_config.engine_path))
      if pipeline_config.project_list is not None and len(pipeline_config.project_list) > 0:
        for proj, path in pipeline_config.project_list:
          print("Project List :>\n  - [{}] at [{}]".format(
            proj, path
          ))
      else:
        print("No projects found in pipeline configuration")
      

  def get_settings(self):
    return self.settings
  
  def get_pipeline_config(self):
    return self.pipeline_config

  def parse_args(self, parser):
    self.settings = parser.parse_args()

    if self.settings.config is not None:
      self.setting.config = self.settings.config

    self.project_config = utilities.normalize_config_str(
                            self.project_config
                          )
    
  def help_was_printed(self):
    return self.help_printed

  def project_configuration(self):
    return self.project_config

  def is_verbose(self):
    return self.settings.verbose

  def is_legacy(self):
    return self.settings.legacy_cmd

  def build_list(self):
    return self.settings.build if self.settings.build is not None else []

  def _process_config_table(self, table):
    try:
      engine_config = table["engine"]
    except KeyError or TypeError:
      engine_config = None
      
    try:
      if engine_config is None:
        raise KeyError()
      engine_path = engine_config["path"]
    except KeyError:
      engine_path = "."

    try:
      projects = table["project"]

      projs = []
      for proj in projects:
        projs.append((proj["name"], proj["path"]))
    except KeyError or TypeError:
      projs = []

    return PipelineConfig(engine_path, projs)


oe_env = OtherEnginePipelineEnvironment()
