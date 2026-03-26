class WorkloadConfiguration:
    RANDOM_SEED = 42
    """
    int: Fixed seed for the random number generator.

    Rationale: Ensures academic reproducibility. Running the generator multiple
    times will produce the exact same workload files, allowing for isolated
    comparisons between Windows and MGLRU without workload variance.
    """

    MAX_VIRTUAL_PAGES = 2500
    """
    int: The realistic working boundary of pages a process will touch.

    Rationale: While the C++ architecture allows 1,048,576 pages (4GB), an actual
    process doesn't use all of it. 2500 pages equals ~10MB of virtual memory usage,
    which is large enough to overflow the 4MB (1024 frames) of physical RAM.
    """

    LOCALITY_HOT_ZONE_RATIO = 0.20
    """
    float: 20% of 2500 pages = 500 pages (2MB).

    Rationale: If two interactive processes run simultaneously, their combined
    hot zones (1000 pages) will almost entirely fill the 1024 physical frames,
    creating perfect, realistic memory contention.
    """

    LOCALITY_HOT_ZONE_PROBABILITY = 0.80
    """
    float: The probability that a memory access will hit the hot zone.
    Rationale: Completes the 80/20 rule. 80% of accesses hit the 20% hot zone,
    accurately modeling temporal and spatial locality in interactive applications.
    """

    LOOP_SIZE_STANDARD = 600
    """
    int: A standard loop size.

    Rationale: Takes up more than half the physical RAM (1024 frames). Will cause
    heavy page faulting when competing with interactive processes.
    """

    LOOP_SIZE_LARGE = 1200
    """
    int: A massive thrashing loop.

    Rationale: 1200 is strictly greater than the 1024 physical frames. If a process
    runs this loop, it is mathematically guaranteed to trigger Belady's Anomaly and
    thrash a standard LRU algorithm, perfectly testing the MGLRU protection logic.
    """

    LOOP_SIZE_LARGE = 80
    """
    int: A larger cycle size for aggressive background thrashing.

    Rationale: Creates severe memory pressure to test if the Multi-Generational
    LRU (MGLRU) can successfully age and evict these pages without impacting
    the hot zones of high-priority interactive tasks.
    """

    BURST_INTERACTIVE_MEAN = 2.0
    """
    float: The mean for the log-normal distribution of interactive CPU bursts.

    Rationale: Log-normal distributions model CPU bursts realistically. A low mean
    generates mostly very short CPU bursts (typical of UI threads waiting on user input).
    """

    BURST_INTERACTIVE_DEVIATION = 0.5
    """
    float: The standard deviation for interactive CPU bursts.

    Rationale: Keeps interactive bursts tightly clustered around small values.
    """

    BURST_BACKGROUND_MEAN = 5.0
    """
    float: The mean for the log-normal distribution of background CPU bursts.

    Rationale: A higher mean generates longer, uninterrupted CPU execution times,
    typical of heavy computational tasks, file transfers, or database scans.
    """

    BURST_BACKGROUND_DEVIATION = 1.0
    """
    float: The standard deviation for background CPU bursts.

    Rationale: Allows for wider variance in background task execution lengths.
    """

    BURST_MIXED_MEAN = 4.0
    """float: The mean burst time for a mixed realistic OS workload."""

    BURST_MIXED_DEVIATION = 1.0
    """float: The standard deviation for a mixed realistic OS workload."""

    MINIMUM_BURST_TIME_STANDARD = 1
    """int: The absolute minimum CPU burst time (prevents 0-tick processes)."""

    MINIMUM_BURST_TIME_BACKGROUND = 10
    """
    int: The minimum burst time specifically for background processes.

    Rationale: Ensures that background data-scanning tasks are long enough to
    actually flood the physical memory and trigger page replacements.
    """

    PRIORITY_MINIMUM = 1
    """int: The highest priority bound (Linux Nice -20 equivalent)."""

    PRIORITY_MAXIMUM = 10
    """int: The lowest priority bound (Linux Nice 19 equivalent)."""

    PRIORITY_INTERACTIVE_MEAN = 2.0
    """
    float: The mean priority for interactive scenarios.

    Rationale: Centers around Priority 2, ensuring foreground UI apps receive
    the largest time slices in the Completely Fair Scheduler (CFS).
    """

    PRIORITY_INTERACTIVE_DEVIATION = 1.0
    """float: Standard deviation for interactive priority distribution."""

    PRIORITY_BACKGROUND_MEAN = 9.0
    """
    float: The mean priority for background scenarios.

    Rationale: Centers around Priority 9, ensuring background data scans
    do not starve the CPU of other critical processes.
    """

    PRIORITY_BACKGROUND_DEVIATION = 1.0
    """float: Standard deviation for background priority distribution."""

    PRIORITY_MIXED_MEAN = 5.0
    """float: Normal distribution center for a realistically mixed system load."""

    PRIORITY_MIXED_DEVIATION = 2.0
    """float: Standard deviation for a mixed system load."""

    ARRIVAL_RATE_LAMBDA = 0.5
    """
    float: The lambda parameter for the exponential inter-arrival distribution.

    Rationale: Models independent events arriving at a continuous average rate.
    A lambda of 0.5 dictates that, on average, a new process arrives every 2 ticks,
    creating overlapping process executions to test scheduler fairness.
    """

    MINIMUM_INTER_ARRIVAL_TIME = 1
    """int: Prevents multiple processes from having the exact same arrival tick."""

    MIXED_SCENARIO_INTERACTIVE_THRESHOLD = 3
    """
    int: Priorities at or below this are assigned interactive locality memory traces.
    """

    MIXED_SCENARIO_BACKGROUND_THRESHOLD = 8
    """
    int: Priorities at or above this are assigned aggressive scanning/looping traces.
    """

    MIXED_SCENARIO_SCAN_PROBABILITY = 0.5
    """
    float: The 50/50 chance a background task will perform a linear scan vs a thrashing loop.
    """

    DEFAULT_BASE_DIRECTORY = "../workloads"
    """str: The root directory for workload generation output."""

    TRACES_SUBDIRECTORY = "traces"
    """str: The subdirectory name to hold the .ref memory trace files."""

    CSV_HEADER = "ProcessID,Priority(1-10),BurstTime,ArrivalTime\n"
    """str: The exact CSV header expected by the C++ simulation file parser."""