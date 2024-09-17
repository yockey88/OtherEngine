import os

from typing import override

from other.core import utilities
from other.pipeline import env
from other.pipeline.pipeline import Pipeline
from other.native import engine as oe

class YockcraftPipeline(Pipeline):
  @override
  def run(self):

    # self._process_error(self._try_build())
    # self._process_error(self._try_run())

    return 0

if __name__ == "__main__":
  cfg_path = "yockcraft/config/toolchain.cfg"
  env.init(cfg_path, False)
  print(oe.open_scene("./yockcraft/assets/scenes/overworld.yscn"))

  # pipeline = YockcraftPipeline()
  # pipeline.run()