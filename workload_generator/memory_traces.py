"""
Algorithms for generating specific memory reference strings to test
various page replacement behaviors like locality, scanning, and thrashing.
"""

import random
from config import WorkloadConfiguration

def gen_locality_trace(burst_time: int, max_pages: int) -> list[int]:
    """
    Generates a memory reference trace simulating the 80/20 locality of reference rule.

    Calculates a subset of virtual pages to act as a hot zone. A high probability
    of accesses will fall within this hot zone, while the remainder will be randomly
    distributed across the entire virtual address space.

    Args:
        burst_time: The total number of memory accesses to generate.
        max_pages: The upper bound of the virtual address space.

    Returns:
        A list of integers representing the virtual pages accessed in sequence.
    """

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
    """
    Generates a purely sequential memory reference trace.

    Used to test algorithms against use-once access patterns, effectively
    simulating sequential data processing or background file scanning.

    Args:
        burst_time: The total number of memory accesses to generate.

    Returns:
        A sequential list of integers starting from 1 up to the burst_time.
    """

    return list(range(1, burst_time + 1))

def gen_loop_trace(burst_time: int, loop_size: int) -> list[int]:
    """
    Generates a cyclical memory reference trace.

    Used to simulate array iterations or database operations that can trigger
    Belady's Anomaly or severe thrashing in LRU-based algorithms.

    Args:
        burst_time: The total number of memory accesses to generate.
        loop_size: The number of unique pages to iterate through before repeating.

    Returns:
        A list of integers representing the cyclical page accesses.
    """

    trace = []
    for current_access in range(burst_time):
        trace.append((current_access % loop_size) + 1)

    return trace
