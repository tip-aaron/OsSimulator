#pragma once
#include <cstdint>

namespace OsSimulationArchitecture {
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
}

