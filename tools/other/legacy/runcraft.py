import sys

from other.core import utilities

if __name__ == "__main__":
  arguments = sys.argv[1:]
  args = utilities.process_arguments(sys.argv)
  config = utilities.get_arg_value(args, "-c", "debug")

  config = utilities.normalize_config_str(config)

  craft_args = ["--project", "./yockcraft.other",
                "--cwd", "./yockcraft",
                "--editor"]
  ret = 0 if utilities.run_project(config, "yockcraft", craft_args) else 1
  sys.exit(ret)
