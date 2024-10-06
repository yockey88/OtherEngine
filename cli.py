import sys

from typing import override

from tools.other.core import utilities
from tools.other.core.legacy import do_legacy_cmds
from tools.other.pipeline import env
from tools.other.pipeline.pipeline import Pipeline

class OtherCliPipeline(Pipeline):
    @override
    def run(self):
      print("> running pipeline")
      if env.get_settings().fnv:
        return utilities.fnv(env.get_settings().fnv)
      if env.get_settings().generate_files:
        self._process_error(self._gen_files(), " > file generation successful", " !> file generation failed!")
      if env.get_settings().generate_projects or env.get_settings().generate_files:
        self._process_error(self._gen_projects(), " > project generation successful", " !> project generation failed!")

      self._process_error(self._try_build())
      if env.should_test():
        self._process_error(self._try_test() , " > testing successful", " !> testing failed!")
        sys.exit(0)

      self._process_error(self._open_editor())
      self._process_error(self._try_run())
      return 0

if __name__ == "__main__":
    # if cli.py is begin called then we use engine configuration
    env.init("tools/other/other.cfg")
    verbose: bool = env.is_verbose()

    if env.get_settings().view_platform:
        utilities.print_platform_string()

    if env.is_legacy() and not env.help_was_printed():
        if verbose:
            print("> running legacy commands")
        do_legacy_cmds()
    elif not env.help_was_printed():
        pipeline = OtherCliPipeline()
        pipeline.run()