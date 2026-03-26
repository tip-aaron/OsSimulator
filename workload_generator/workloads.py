"""
Generators for producing strictly defined statistical workloads to test
specific operating system scheduling and memory management hypotheses.
"""

import random
import os
from config import WorkloadConfiguration
from memory_traces import gen_locality_trace, gen_scan_trace, gen_loop_trace

def _write_process_and_trace_data(scenario_name: str, process_data: list, trace_data: dict):
    """
    Handles the file I/O operations for saving scenario definitions and memory traces.

    Args:
        scenario_name: The name used to format the output CSV file.
        process_data: A list of formatted strings containing the CPU attributes.
        trace_data: A dictionary mapping process IDs to their respective memory trace lists.
    """

    scenario_directory = os.path.join(WorkloadConfiguration.DEFAULT_BASE_DIRECTORY, scenario_name)
    traces_directory = os.path.join(scenario_directory, WorkloadConfiguration.TRACES_SUBDIRECTORY)

    os.makedirs(traces_directory, exist_ok=True)

    csv_filepath = os.path.join(scenario_directory, "processes.csv")

    with open(csv_filepath, 'w', encoding='utf-8') as out_file:
        out_file.write(WorkloadConfiguration.CSV_HEADER)
        out_file.writelines(process_data)

    for process_id, trace in trace_data.items():
        trace_filepath = os.path.join(traces_directory, f"process_{process_id}.ref")
        with open(trace_filepath, 'w', encoding='utf-8') as trace_file:
            for page in trace:
                trace_file.write(f"{page}\n")

def gen_interactive(num_processes: int):
    """
    Generates a workload consisting entirely of highly interactive, foreground processes.

    Characteristics include extremely short CPU bursts, high priorities, and strong
    locality of reference. Designed to test responsiveness and working set efficiency.

    Args:
        num_processes: The total number of processes to simulate in this scenario.
    """

    random.seed(WorkloadConfiguration.RANDOM_SEED)

    current_arrival_time = 0
    process_data = []
    trace_data = {}

    for process_index in range(num_processes):
        current_arrival_time += max(WorkloadConfiguration.MINIMUM_INTER_ARRIVAL_TIME,
                            int(random.expovariate(WorkloadConfiguration.ARRIVAL_RATE_LAMBDA)))

        process_id = process_index + 1
        burst_time = max(WorkloadConfiguration.MINIMUM_BURST_TIME_STANDARD,
                         int(random.lognormvariate(WorkloadConfiguration.BURST_INTERACTIVE_MEAN,
                                                   WorkloadConfiguration.BURST_INTERACTIVE_DEVIATION)))
        priority = int(round(random.normalvariate(WorkloadConfiguration.PRIORITY_INTERACTIVE_MEAN,
                                                  WorkloadConfiguration.PRIORITY_INTERACTIVE_DEVIATION)))
        priority = max(WorkloadConfiguration.PRIORITY_MINIMUM, min(WorkloadConfiguration.PRIORITY_MAXIMUM, priority))

        process_data.append(f"{process_id},{priority},{burst_time},{current_arrival_time}\n")
        trace_data[process_id] = gen_locality_trace(burst_time, WorkloadConfiguration.MAX_VIRTUAL_PAGES)

    _write_process_and_trace_data("interactive", process_data, trace_data)

def gen_background(num_processes: int):
    """
    Generates a workload consisting entirely of long-running, background processes.

    Characteristics include massive CPU bursts, low priorities, and sequential
    or thrashing memory patterns. Designed to test throughput and anti-thrashing mechanisms.

    Args:
        num_processes: The total number of processes to simulate in this scenario.
    """

    random.seed(WorkloadConfiguration.RANDOM_SEED)

    current_arrival_time = 0
    process_data = []
    trace_data = {}

    for process_index in range(num_processes):
        current_arrival_time += max(WorkloadConfiguration.MINIMUM_INTER_ARRIVAL_TIME,
                            int(random.expovariate(WorkloadConfiguration.ARRIVAL_RATE_LAMBDA)))

        process_id = process_index + 1
        burst_time = max(WorkloadConfiguration.MINIMUM_BURST_TIME_BACKGROUND,
                         int(random.lognormvariate(WorkloadConfiguration.BURST_BACKGROUND_MEAN,
                                                   WorkloadConfiguration.BURST_BACKGROUND_DEVIATION)))
        priority = int(round(random.normalvariate(WorkloadConfiguration.PRIORITY_BACKGROUND_MEAN,
                                                  WorkloadConfiguration.PRIORITY_BACKGROUND_DEVIATION)))
        priority = max(WorkloadConfiguration.PRIORITY_MINIMUM, min(WorkloadConfiguration.PRIORITY_MAXIMUM, priority))
        process_data.append(f"{process_id},{priority},{burst_time},{current_arrival_time}\n")

        if random.random() < WorkloadConfiguration.MIXED_SCENARIO_SCAN_PROBABILITY:
            trace_data[process_id] = gen_scan_trace(burst_time)
        else:
            trace_data[process_id] = gen_loop_trace(burst_time, WorkloadConfiguration.LOOP_SIZE_LARGE)

    _write_process_and_trace_data("background", process_data, trace_data)

def gen_mixed_interactive_and_background(num_processes: int):
    """
    Generates a realistic, unified workload combining interactive and background tasks.

    Designed to test the overall fairness of the CPU scheduler and the ability
    of the memory manager to protect interactive working sets from background interference.

    Args:
        num_processes: The total number of processes to simulate in this scenario.
    """

    random.seed(WorkloadConfiguration.RANDOM_SEED)

    current_arrival_time = 0
    process_data = []
    trace_data = {}

    for process_index in range(num_processes):
        current_arrival_time += max(WorkloadConfiguration.MINIMUM_INTER_ARRIVAL_TIME,
                            int(random.expovariate(WorkloadConfiguration.ARRIVAL_RATE_LAMBDA)))

        process_id = process_index + 1
        burst_time = max(WorkloadConfiguration.MINIMUM_BURST_TIME_BACKGROUND,
                         int(random.lognormvariate(WorkloadConfiguration.BURST_MIXED_MEAN,
                                                   WorkloadConfiguration.BURST_MIXED_DEVIATION)))
        priority = int(round(random.normalvariate(WorkloadConfiguration.PRIORITY_MIXED_MEAN,
                                                  WorkloadConfiguration.PRIORITY_MIXED_DEVIATION)))
        priority = max(WorkloadConfiguration.PRIORITY_MINIMUM, min(WorkloadConfiguration.PRIORITY_MAXIMUM, priority))

        process_data.append(f"{process_id},{priority},{burst_time},{current_arrival_time}\n")

        if priority <= WorkloadConfiguration.MIXED_SCENARIO_INTERACTIVE_THRESHOLD:
            trace_data[process_id] = gen_locality_trace(burst_time, WorkloadConfiguration.MAX_VIRTUAL_PAGES)
        elif priority >= WorkloadConfiguration.MIXED_SCENARIO_BACKGROUND_THRESHOLD:
            if random.random() < WorkloadConfiguration.MIXED_SCENARIO_SCAN_PROBABILITY:
                trace_data[process_id] = gen_scan_trace(burst_time)
            else:
                trace_data[process_id] = gen_loop_trace(burst_time, WorkloadConfiguration.LOOP_SIZE_LARGE)
        else:
            trace_data[process_id] = gen_locality_trace(burst_time, WorkloadConfiguration.MAX_VIRTUAL_PAGES)

    _write_process_and_trace_data("mixed_interactive_and_background", process_data, trace_data)
