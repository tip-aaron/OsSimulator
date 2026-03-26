# Workloads

Below are the three workloads generated for this simulation.

## 1. Interactive Workload

**Goal:** Evaluate system responsiveness and "Foreground" task performance.

- **Process Profile: * CPU:** Short, frequent bursts (Log-normal distribution, Low Mean).
  - **Priority:** High (Priority 1-3), mapping to Linux nice values -20 to -11.
  - **Memory:** High Temporal Locality (80/20 Rules).
- **Expected Outcome: * Windows Simulation:** Should excel because it's optimized for interactive foreground "Working Sets".
  - **Linux CFS:** Should provide the lowest **Response Time** as high-priority tasks receive more CPU time.
- **Metric Focus:** Average Response Time and Page Fault Rate under low pressure.

## 2. Background / Data Workload

**Goal:**  Evaluate system throughput and resistance to memory thrashing.

- **Process Profile: * CPU:** Long, sustained bursts (Log-normal distribution, High Mean).
  - **Priority:** Low (Priority 8-10), mapping to Linux nice values 11 to 19.
  - **Memory:** Sequential Scans and Large Cyclic Loops (Thrashing).
- **Expected Outcome: * Linux MGLRU:** Should outperform Windows.
    MGLRU's multi-generational aging is designed to detect "use-once" sequential scans and evict them before they flush out the entire cache.
  - **Throughput:** Should remain high as the scheduler focuses on completion rather than rapid switching.
- **Metric Focus:** Throughput and Page Fault Rate under high pressure or heavy contention.

## 3. Mixed / Real-World Workload

**Goal:** Evaluate scheduler fairness and memory isolation in production-like conditions.

- **Process Profile: * Composition:** A statistical mix of interactive and background processes.
  - **Arrival:** Modeled via Exponential Distribution (Poisson process) to ensure overlapping execution.
- **Expected Outcome: * Fairness:** Linux CFS is expected to maintain "Target Latency," ensuring the background tasks don't starve, while MGLRU prevents "scans" from stealing the memory frames needed by interactive "hot zones."
  - **System Balance:** Provides the most comprehensive view of the trade-offs between the two OS philosophies.
- **Metric Focus:** CPU Utilization, Fairness Index, and Average Waiting Time.
