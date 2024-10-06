import os
import subprocess
import sys

from pathlib import Path

from . import project_settings
from . import utilities

def run_test(include_filter  = [], exclude_filter = [], other_args = []):
  f = [] 

  include_str = None
  if len(include_filter) > 0:
    include_str = include_filter[0]
    for i in include_filter[1:]:
      include_str += ":{}.*".format(i)
  
  exclude_str = None
  if len(exclude_filter) > 0:
    exclude_str = "-{}".format(exclude_filter[0])
    for e in exclude_filter[1:]:
      exclude_str += ":{}".format(e)

  filter_exists = include_str is not None or exclude_str is not None
  
  fstr = "--gtest_filter=" if filter_exists else None
  fstr += include_str if include_str is not None else ""
  fstr += ":" if include_str is not None and exclude_str is not None else ""
  fstr += exclude_str if exclude_str is not None else ""
  
  proc_args = ["./bin/Debug/unit_tests/unit_tests.exe"]
  proc_args.extend([fstr] if filter_exists else [])

  if len(other_args) > 0:
    proc_args.extend(other_args)

  print ("> running unit tests")
  print("   > {}".format(proc_args))
  return subprocess.call(proc_args, cwd=os.getcwd())