from argparse import Namespace

from tools.core.utilities import Singleton
import tools.core.utilities as utilities


@Singleton
class OtherEnginePipelineEnvironment:
    settings: Namespace
    config: str

    def __init__(self):
        self.settings = None
        self.config = "debug"

    def parse_args(self, parser):
        self.settings = parser.parse_args()

        if self.settings.config is not None:
            self.config = self.settings.config

        self.config = utilities.normalize_config_str(self.config)

    def configuration(self):
        return self.config

    def is_verbose(self):
        return self.settings.verbose

    def is_legacy(self):
        return self.settings.legacy_cmd

    def build_list(self):
        return self.settings.build if self.settings.build is not None else []


oe_env = OtherEnginePipelineEnvironment.instance()
