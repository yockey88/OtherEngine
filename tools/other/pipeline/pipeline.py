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
from other.pipeline.pipeline_env import PipelineConfig

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
  def _open_editor(self):
    if not oe_env.get_settings().edit is None:
      return 0
    
    config = oe_env.project_configuration()
    verbose: bool = oe_env.is_verbose()

    # name: str = oe_env.get_settings().edit[0]
    # res = utilities.open_project(config, name)
    # self._process_error(res, " > project opened in editor", " !> failed to open project in editor")
    return 1

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
        res = build_project(proj)
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
  def _try_run(self):
    if oe_env.get_settings().run is None:
      return 0
    
    config = oe_env.project_configuration()
    verbose: bool = oe_env.is_verbose()

    project = oe_env.get_settings().run[0]

    path_dir = Path(".")
    candidates: list[Path] = []
    for _, _, paths in os.walk(".", False):
      for path in paths:
        p = Path(path)
        if p.suffix == ".other" and p.stem == project:
          candidates.append(p) 
          path_dir = p.parent.absolute()

    if len(candidates) == 0:
      print(" > no project found for {}".format(project))
      return 1
    
    if len(candidates) > 1:
      print(" > multiple projects found named {}!".format(project))
      return 1
    
    name = candidates[0].stem
    filename = candidates[0].name

    if sys.platform == "linux":
      c_offset, cpath = _process_linux_path(path_dir)
      path_dir = Path(cpath)

    print("{}/{}/{}".format(path_dir, name, filename))
    res = utilities.run_project(config, name, [
      "--project", "{}/{}/{}.other".format(path_dir, name, name),
      "--cwd", "./{}".format(name) 
    ])

  @abstractmethod
  def run(self):
    pass