import sys

import utilities
from tools.pipeline_env import oe_env

config = oe_env.configuration()

ret = 0 if utilities.run_project(config, "fnv", []) else 1
sys.exit(ret)
