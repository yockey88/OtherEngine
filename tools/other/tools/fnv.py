import sys

from ..core.utilities import fnv
from ..core.utilities import run_project

if __name__ == "__main__":
  if len(sys.argv) < 2:
    print("Usage: {} <string>".format(sys.argv[0]))
    sys.exit(1)

  sys.exit(fnv(sys.argv[1]))
  