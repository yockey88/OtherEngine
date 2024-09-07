from tools.core.legacy import do_legacy_cmds as do_legacy_cmds
from tools.core.commands import initialize_parser as initialize_parser
from tools.project.project_builders import full_build as full_build

# singleton context objects
from tools.pipeline.pipeline_env import oe_env as env
from tools.pipeline.pipeline import pipeline as pipeline