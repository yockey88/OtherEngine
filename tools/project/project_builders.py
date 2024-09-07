import os
import subprocess

import tools.core.project_settings as project_settings
import tools.core.utilities as utilities

from tools.pipeline.pipeline_env import oe_env

def full_build():
    config = oe_env.configuration()

    print("Building {} in [{}] Configuration".format(
        project_settings.EXE_NAME, config
    ))

    verbose: bool = False

    stdout = subprocess.DEVNULL if verbose is False else None
    stderr = subprocess.DEVNULL if verbose is False else None

    ret = 0
    if utilities.is_windows():
        MSBUILD = os.environ["MSBUILD"][8:-1]
        MSBUILD = "C:\\\\" + MSBUILD
        ret = subprocess.call(
            [
                "cmd.exe", "/c", MSBUILD,
                "{}.sln".format(project_settings.PROJECT_NAME),
                "/property:Configuration={}".format(config)
            ],
            stdout=stdout, stdin=stderr
        )
    else:
        raise EnvironmentError("Non-windows platform detected")

    return ret