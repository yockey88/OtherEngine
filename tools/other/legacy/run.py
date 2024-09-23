import sys
import os
import subprocess

import other.core.project_settings as project_settings
import other.core.utilities as utilities

def run_default():
    arguments = sys.argv[1:]
    args = utilities.process_arguments(sys.argv)
    project = utilities.get_arg_value(args, "-n", project_settings.EXE_NAME)
    config = utilities.get_arg_value(args, "-c", "debug")

    config = utilities.normalize_config_str(config)

    exepath = "{}/bin/{}/{}".format(os.getcwd(), config, project_settings.EXE_NAME)
    ret = 0

    if utilities.is_windows():
        proc_args = ["cmd.exe", "/c",
                    "{}\\run.bat".format(project_settings.TOOLS_DIR),
                    config, project]
        proc_args.extend(arguments)
        ret = subprocess.call(proc_args, cwd=os.getcwd())
    else:
        ret = subprocess.call(["{}".format(exepath)], cwd=exepath)

    sys.exit(ret)


if __name__ == "__main__":
    run_default()