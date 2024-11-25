import sys
import os

from typing import override
from abc import abstractmethod

from pathlib import Path

from ..core import utilities
from ..core.utilities import Singleton
from ..core.project_builders import gen_projects, full_build, build_project
from ..core.file_generators import generate_engine_files
from ..core.run_tests import run_test
from ..pipeline.pipeline_env import oe_env
from ..pipeline.pipeline_env import PipelineConfig

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
      res = full_build(config, verbose)
      self._process_error(res, " > full build successful", " !> full build failed!")
      return res
    else:
      res = 0
      for proj in build_list:
        res = build_project("Debug" , verbose , proj)
        if res != 0:
          break
      return res
    
  @classmethod
  def _process_linux_path(path: Path):
    c_offset = 0
    cpath = ""

    for part in path.parts:
      cpath += path + "/"

      c_offset += 1
      if part == '/' or part == '.':
        continue

      c_offset += len(part)
      if part == "c":
        break

    return c_offset, cpath

  @classmethod
  def _run_project(self):
    config = oe_env.project_configuration()
    verbose: bool = oe_env.is_verbose()

    project = oe_env.get_settings().run[0]

    proj_data = oe_env.get_project_path(project)
    if proj_data is None:
      print("project {} not found in environment configuration, did you remember to add it to the project list?".format(project))
      return 1
    
    [real_name,proj_path,config_file] = proj_data
    config_file = Path(config_file).absolute()
    project_work_dir = Path(proj_path).absolute()
    args = [
      "--project", "{}".format(config_file),
      "--cwd", "{}".format(project_work_dir)
    ]

    if verbose:
      print(" > [{} build] running {} with arguments {}".format(config, real_name, args))

    if oe_env.get_settings().edit is not None:
      print("  > editing project")
      args.append("--editor")

    if oe_env.get_settings().run_test_harness is not None:
      print("  > running test harness")
      args.append("--testing")

    return utilities.run_project(config, real_name, args)

  @classmethod 
  def _run_dotnet_project(self):
    config = oe_env.project_configuration()
    verbose: bool = oe_env.is_verbose()

    project = oe_env.get_settings().run_dotnet[0]

    if sys.platform == "linux":
      c_offset, cpath = _process_linux_path(path_dir)
      path_dir = Path(cpath)

    print("dotnet {}".format(project))
    res = utilities.run_dotnet_project(config, project, oe_env.get_settings().run_dotnet[1:])

    return res

  @classmethod
  def _try_run(self):
    if oe_env.get_settings().run is None and oe_env.get_settings().edit is None:
      return 0
    
    if oe_env.get_settings().run_test_harness is not None and len(oe_env.get_settings().run_test_harness) > 0:
      if oe_env.get_settings().run is None:
        setattr(oe_env.get_settings(), "run", [])

      oe_env.get_settings().run.extend(oe_env.get_settings().run_test_harness)
    elif oe_env.get_settings().edit is not None and len(oe_env.get_settings().edit) > 0:
      if oe_env.get_settings().run is None:
        setattr(oe_env.get_settings(), "run", [])
      
      oe_env.get_settings().run.extend(oe_env.get_settings().edit)

    if oe_env.get_settings().run is not None:
      return self._run_project()
    else:
      return 1

  @classmethod
  def _try_test(self):
    if oe_env.get_settings().test is None:
      return 0
    
    test_list = oe_env.test_list()
    ignore_list = oe_env.test_ignore_list()
    print("> running tests w/ filters : tests = {} , ignore = {}".format("all" if len(test_list) == 0 else test_list, ignore_list))
    return run_test(test_list , ignore_list if len(test_list) == 0 else [] , [])
    
  @classmethod
  def _try_create_project(self):
    if oe_env.get_settings().create_project is None:
      return 0
    
    project_name = oe_env.get_settings().create_project[0]

    return utilities.create_project(project_name)

  @abstractmethod
  def run(self):
    pass