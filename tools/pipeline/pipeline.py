from tools.project.project_builders import full_build
from tools.pipeline.pipeline_env import oe_env
from tools.core.utilities import Singleton

@Singleton
class Pipeline:
    def try_build(self):
        build_list = oe_env.build_list()
        if len(build_list) == 0:
            print("> performing full rebuilding")
            res = full_build()
            if res == 0:
                print(" > rebuild successful")
            else:
                print(" !> rebuild failed!")
            return

        for b in build_list:
            ...


pipeline = Pipeline.instance()
