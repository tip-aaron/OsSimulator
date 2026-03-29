#pragma once
#include <cstdint>

namespace os_simulation_architecture {
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
constexpr uint32_t BACKING_STORE_LATENCY_MS = 10;
/*
 * Cost to switch from one process to another
 */
constexpr uint64_t CONTEXT_SWITCH_TICK_COST = 0;
}  // namespace os_simulation_architecture
