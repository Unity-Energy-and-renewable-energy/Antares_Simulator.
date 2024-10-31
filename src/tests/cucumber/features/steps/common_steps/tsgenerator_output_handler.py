# TS-generator outputs parsing

import os
import pandas as pd


class tsgenerator_output_handler:
    def __init__(self, generator_output_path):
        self.generator_output_path = generator_output_path
        self.generated_ts = {}

    def get_generate_ts(self, area: str, cluster: str):
        uid = f"{area}.{cluster}"
        if uid not in self.generated_ts:
            self.__parse_generated_ts(area, cluster)
        return self.generated_ts[uid]

    def __parse_generated_ts(self, area: str, cluster: str):
        uid = f"{area}.{cluster}"
        absolute_path = os.path.join(self.generator_output_path, area, f"{cluster}.txt")
        self.generated_ts[uid] = pd.read_csv(absolute_path, header=None, sep='\t')
