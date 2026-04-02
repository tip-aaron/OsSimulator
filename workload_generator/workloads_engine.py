"""
Generators for producing strictly defined statistical workloads to test
specific operating system scheduling and memory management hypotheses.
"""

import random
import os
from config import WorkloadConfiguration
from memory_traces import gen_locality_trace, gen_scan_trace, gen_loop_trace

PAGE_SIZE = 4096

def _write_process_and_trace_data(scenario_name: str, process_data: list, trace_data: dict, read_probability: float) -> None:
    scenario_directory = os.path.join(WorkloadConfiguration.DEFAULT_BASE_DIRECTORY, scenario_name)
    traces_directory = os.path.join(scenario_directory, WorkloadConfiguration.TRACES_SUBDIRECTORY)

    os.makedirs(traces_directory, exist_ok=True)
    csv_filepath = os.path.join(scenario_directory, "processes.csv")

    with open(csv_filepath, 'w', encoding='utf-8') as out_file:
        out_file.write(WorkloadConfiguration.CSV_HEADER)
        out_file.writelines(process_data)

    for process_id, data in trace_data.items():
        burst_time = data['burst_time']
        trace = data['trace']
        num_accesses = len(trace)

        trace_filepath = os.path.join(traces_directory, f"process_{process_id}.ref")

        with open(trace_filepath, 'w', encoding='utf-8') as trace_file:
            base_gap = 0
            remainder = 0
            if num_accesses > 0:
                total_non_memory_ticks = max(0, burst_time - num_accesses)
                base_gap = total_non_memory_ticks // num_accesses
                remainder = total_non_memory_ticks % num_accesses

            for i, page in enumerate(trace):
                access_type = 'R' if random.random() < read_probability else 'W'
                
                # CRITICAL FIX: Convert the page index to a realistic memory byte address
                # Multiply by 4096 (Page Size) and add a random byte offset within that page
                actual_byte_address = (page * PAGE_SIZE) + random.randint(0, PAGE_SIZE - 1)
                hex_address = hex(actual_byte_address)

                if num_accesses > 0:
                    non_mem_ticks = base_gap + (1 if i < remainder else 0)
                else:
                    non_mem_ticks = 0

                trace_file.write(f"{access_type} {hex_address} {non_mem_ticks}\n")

    print(f"Generated {scenario_name} traces at: {os.path.abspath(traces_directory)}")


def gen_interactive(num_processes: int, is_read_heavy: bool = True) -> None:
    random.seed(WorkloadConfiguration.RANDOM_SEED)
    read_probability = 0.8 if is_read_heavy else 0.2
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
        
        num_accesses = max(1, int(burst_time * WorkloadConfiguration.MEMORY_INTENSITY_RATIO))
        trace = gen_locality_trace(num_accesses, WorkloadConfiguration.MAX_VIRTUAL_PAGES)
        trace_data[process_id] = {'burst_time': burst_time, 'trace': trace}

    _write_process_and_trace_data("interactive", process_data, trace_data, read_probability)

def gen_background(num_processes: int, is_read_heavy: bool = True) -> None:
    random.seed(WorkloadConfiguration.RANDOM_SEED)
    read_probability = 0.8 if is_read_heavy else 0.2
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

        num_accesses = max(1, int(burst_time * WorkloadConfiguration.MEMORY_INTENSITY_RATIO))

        if random.random() < WorkloadConfiguration.MIXED_SCENARIO_SCAN_PROBABILITY:
            trace = gen_scan_trace(num_accesses)
        else:
            trace = gen_loop_trace(num_accesses, WorkloadConfiguration.LOOP_SIZE_LARGE)

        trace_data[process_id] = {'burst_time': burst_time, 'trace': trace}

    _write_process_and_trace_data("background", process_data, trace_data, read_probability)

def gen_mixed_interactive_and_background(num_processes: int, is_read_heavy: bool = True) -> None:
    random.seed(WorkloadConfiguration.RANDOM_SEED)
    read_probability = 0.8 if is_read_heavy else 0.2
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

        num_accesses = max(1, int(burst_time * WorkloadConfiguration.MEMORY_INTENSITY_RATIO))

        if priority <= WorkloadConfiguration.MIXED_SCENARIO_INTERACTIVE_THRESHOLD:
            trace = gen_locality_trace(num_accesses, WorkloadConfiguration.MAX_VIRTUAL_PAGES)
        elif priority >= WorkloadConfiguration.MIXED_SCENARIO_BACKGROUND_THRESHOLD:
            if random.random() < WorkloadConfiguration.MIXED_SCENARIO_SCAN_PROBABILITY:
                trace = gen_scan_trace(num_accesses)
            else:
                trace = gen_loop_trace(num_accesses, WorkloadConfiguration.LOOP_SIZE_LARGE)
        else:
            trace = gen_locality_trace(num_accesses, WorkloadConfiguration.MAX_VIRTUAL_PAGES)

        trace_data[process_id] = {'burst_time': burst_time, 'trace': trace}

    _write_process_and_trace_data("mixed_interactive_and_background", process_data, trace_data, read_probability)