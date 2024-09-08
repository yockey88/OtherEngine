import sys
import os

from typing import override
from abc import abstractmethod

from pathlib import Path

from other.core import utilities
from other.core.utilities import Singleton

from other.core.project_builders import gen_projects, full_build, build_project

from other.core.file_generators import generate_engine_files

from other.pipeline.pipeline_env import oe_env
from other.pipeline.pipeline_config import PipelineConfig

class Pipeline(Singleton):
  config: PipelineConfig = None

  def __init__(self):
    self.config = oe_env.get_pipeline_config()

  @classmethod
  def _process_error(self, res, success_msg=None, fail_msg=None):
    if res == 0:
      if success_msg is not None:
        print(success_msg)
    else:
      if fail_msg is not None:
        print(fail_msg)
      sys.exit(1)

  @classmethod
  def _do_build(self, msg: str, func, name=None):
    print(msg)

    res = 0
    if name is None:
        res = func()
    else:
        res = func(name)

    self._process_error(res)

  def get_config(self):
    return self.config
  
  @classmethod
  def _gen_files(self):
    project, directory, filename = oe_env.get_settings().generate_files
    return generate_engine_files(project, directory, filename)
  
  @classmethod
  def _gen_projects(self):
    return gen_projects()

  @classmethod
  def _try_build(self):
    if oe_env.get_settings().build is None:
      return 0
    
    config = oe_env.project_configuration()
    verbose: bool = oe_env.is_verbose()
    
    build_list = oe_env.build_list()
    do_full_build: bool = False

    if len(build_list) == 0:
      do_full_build = True
    
    if do_full_build:
      res = full_build(oe_env.project_configuration(), oe_env.is_verbose(), )
      self._process_error(res, " > full build successful", " !> full build failed!")
      return res
    else:
      res = 0
      for proj in build_list:
        res = build_project(oe_env.project_configuration(), oe_env.is_verbose(), proj)
        if res != 0:
          break
      return res

  @abstractmethod
  def run(self):
    pass