#pragma once

#include <cstdint>

namespace os_simulator {
constexpr uint8_t MIN_PRIORITY = 1;
constexpr uint8_t MAX_PRIORITY = 10;
constexpr uint64_t USER_CPU_HERTZ = 100;
/**
 * 4KB pages
 */
constexpr uint32_t PAGE_SIZE_BYTES = 4096;
/**
 * Total Physical RAM available
 */
constexpr uint32_t PHYSICAL_FRAME_COUNT = 1024;
/**
 * 4GB Virtual Space
 */
constexpr uint64_t VIRTUAL_ADDRESS_SPACE = 1ULL << 32;
/*
 * Simulated disk latency for page faults
 */
constexpr uint32_t BACKING_STORE_READ_LATENCY_MS = 10;
/*
 * Simulated disk latency for page faults
 */
constexpr uint32_t BACKING_STORE_WRITE_LATENCY_MS = 40;
/*
 * Cost to switch from one process to another
 */
constexpr uint64_t CONTEXT_SWITCH_COST_MS = 2;
/*
 * Granular distance relative to the leftmost task
 * in a Red-Black Tree to avoid over-scheduling.
 */
constexpr uint64_t LINUX_MIN_GRANULARITY_MS = 10;
/*
 * The target time period in which all runnable tasks should run at least once.
 * Used by CFS to calculate dynamic time slices.
 */
constexpr uint64_t LINUX_TARGET_LATENCY_MS = 20;
/*
 * Used in calculating the new vruntime of a process
 * for delta time.
 */
constexpr uint64_t LINUX_DELTA_EXEC_NS = 1'000'000;
/*
 * Standard Linux configuration uses 4 generations (0 = oldest, 3 = youngest)
 */
constexpr uint8_t LINUX_MGLRU_MAX_GENERATIONS = 4;
/*
 * Percentage of FRAME_COUNT efore proactive aging begins
 */
constexpr float LINUX_MGLRU_PROACTIVE_AGING_WATERMARK = 0.80;
/*
 * How often aging is run when needed (system under pressure)
 *
 */
constexpr uint64_t LINUX_MGLRU_AGING_SWEEP_INTERVAL_MS = 500;
/*
 * Delay in waking up the backgorund daemon (or a task,)
 */
constexpr uint64_t LINUX_MGLRU_DAEMON_WAKEUP_LATENCY_MS = 50;
}  // namespace os_simulator
