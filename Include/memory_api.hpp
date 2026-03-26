#pragma once
#include <cstdint>

namespace os_simulation_memory {
class IMemoryManager {
 public:
  virtual ~IMemoryManager() = default;
  /**
   * Returns false if page fault
   */
  virtual bool accessAddress(uint64_t virtualAddress) = 0;
  virtual void handlePageFault(uint64_t virtualAddress) = 0;
};
}  // namespace os_simulation_memory
