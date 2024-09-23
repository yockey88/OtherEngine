import os
import subprocess
import sys

from other.core import utilities

if __name__ == "__main__":
  arguments = sys.argv[1:]
  args = utilities.process_arguments(sys.argv)

  CMD = "C:\\\\Yock\\gamedev\\Testbed\\bin\\Debug\\testbed.exe"
  cmds = ["cmd.exe", "/c", CMD,
          "--project", "C:\\\\Yock\\gamedev\\Testbed\\testbed.other",
          "--editor",
          "--cwd", "C:\\\\Yock\\gamedev\\Testbed"]

  ret = subprocess.call(cmds, cwd=os.getcwd())
  sys.exit(ret)
