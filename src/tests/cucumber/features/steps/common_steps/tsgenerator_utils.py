# Methods to run TS generator

import subprocess
from common_steps.tsgenerator_output_handler import tsgenerator_output_handler

def run_tsgenerator(context, all_thermal: bool, all_links: bool):
    command = build_tsgenerator_command(context, all_thermal, all_links)
    print(f"Running command: {command}")
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    out, err = process.communicate()
    context.tsgenerator_thermal_output_path = parse_thermal_output_folder_from_logs(out)
    context.tgoh = tsgenerator_output_handler(context.tsgenerator_thermal_output_path)
    context.return_code = process.returncode


def build_tsgenerator_command(context, all_thermal: bool, all_links: bool):
    command = [context.config.userdata["ts-generator"]]
    if all_thermal:
        command.append("--all-thermal")
    if all_links:
        command.append("--all-links")
    command.append(str(context.study_path))
    return command



def parse_thermal_output_folder_from_logs(logs: bytes) -> str:
    for line in logs.splitlines():
        if b'Thermal output folder : ' in line:
            return line.split(b'Thermal output folder : ')[1].decode('ascii')
    raise LookupError("Could not parse output folder in output logs")