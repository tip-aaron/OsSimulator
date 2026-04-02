import random
import os
from config import WorkloadConfiguration

def gen_locality_trace(burst_time: int, max_pages: int) -> list[int]:
    trace = []
    hot_zone_size = max(1, int(max_pages * WorkloadConfiguration.LOCALITY_HOT_ZONE_RATIO))
    hot_zone_start = random.randint(0, max_pages - hot_zone_size)
    hot_zone_end = hot_zone_start + hot_zone_size

    for _ in range(burst_time):
        if random.random() < WorkloadConfiguration.LOCALITY_HOT_ZONE_PROBABILITY:
            trace.append(random.randint(hot_zone_start, hot_zone_end))
        else:
            trace.append(random.randint(0, max_pages))
    return trace

def gen_scan_trace(burst_time: int) -> list[int]:
    return list(range(1, burst_time + 1))

def gen_loop_trace(burst_time: int, loop_size: int) -> list[int]:
    trace = []
    for current_access in range(burst_time):
        trace.append((current_access % loop_size) + 1)
    return trace
